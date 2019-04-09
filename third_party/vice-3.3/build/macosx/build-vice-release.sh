#!/bin/bash

#
# build-vice-release.sh - build snapshot binaries for x11 gtk and cocoa from the SVN
#
# Written by
#  Christian Vogelgsang <chris@vogelgsang.org>
#
# This file is part of VICE, the Versatile Commodore Emulator.
# See README for copyright notice.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#  02111-1307  USA.
#
# Usage:     build-allext.sh <build-dir> [force build]
#

echo "--- build VICE release from SVN repository ---"

# default parameter
SNAPSHOT=0
LINK_SRC=0
ONLY_BIN=0
DEL_TARGET=0
EXTLIB="extlib"
BUILD_DIR=""
DEBUG=0
DIR_DIST=0

DEFAULT_UI="cocoa"
DEFAULT_ARCH="i386+ppc"
DEFAULT_SDK_VERSION="10.4"
DEFAULT_COMPILER="gcc40"
ALL_JOBS="sdl x11 gtk cocoa cocoa-10.5 cocoa-i386+x86_64-10.6-gcc42 cocoa-i386+x86_64-10.6-clang_gcc cocoa-i386+x86_64-10.6-clang"
JOBS="cocoa-10.5 cocoa-i386+x86_64-10.6-gcc42 cocoa-i386+x86_64-10.6-clang"

OWN_TAG=""

usage() {
  cat <<EOF

  Usage:
    $0 [options] <vice-svn-repository>
    
    Build a VICE Release or Snapshot for a set of uis, archs, sdks, and compilers

  Options:                                                                 Default:
    -s                       do snapshot build                             [release build]
    -l                       link repository directory and use it directly [fresh svn export]
    -b                       only binaries and no source package           [with source package]
    -f                       force delete target directory if it exists    [abort if exists]
    -e <path to extlib>      set path of external library directory        [extlib]
    -o <target dir>          set target directory                          [BUILD-snapshot/release]
    -d                       build debug version                           [release]
    -i                       create distribution in directory              [create DMG]
    
    -u <uis>                 set default ui:       sdl x11 gtk cocoa       [$DEFAULT_UI]
    -a <arch>                set default arch:     i386 ppc x86_64 i386+ppc i386+x86_64 [$DEFAULT_ARCH]
    -k <sdk_version>         set default sdk:      10.4 10.5 10.6 10.7     [$DEFAULT_SDK_VERSION]
    -c <compiler>            set default compiler: gcc40 gcc42 clang       [$DEFAULT_COMPILER]
    -j <jobs>                set build jobs
                             [$JOBS]
    -J                       build all jobs
                             [$ALL_JOBS]
    
    -t <tag>                 overwrite tag for this build 
    -D                       quick debug preset: -slbfdi -j cocoa-i386-10.4
EOF
  exit 2
}

# parse arguments
while getopts "slbfe:o:diu:a:k:c:j:DJt:" i ; do
  case "$i" in
    s) SNAPSHOT=1;;
    l) LINK_SRC=1;;
    b) ONLY_BIN=1;;
    f) DEL_TARGET=1;;
    e) EXTLIB="$OPTARG";;
    o) BUILD_DIR="$OPTARG";;
    d) DEBUG=1;;
    i) DIR_DIST=1;;
    
    u) DEFAULT_UI="$OPTARG";;
    a) DEFAULT_ARCH="$OPTARG";;
    k) DEFAULT_SDK_VERSION="$OPTARG";;
    c) DEFAULT_COMPILER="$OPTARG";;
    j) JOBS="$OPTARG";;
    J) JOBS="$ALL_JOBS";;
    t) OWN_TAG="$OPTARG";;

    D) DEBUG=1
       SNAPSHOT=1
       LINK_SRC=1
       ONLY_BIN=1
       DEL_TARGET=1
       DIR_DIST=1
       JOBS="cocoa-i386-10.4-gcc40"
       ;;
    ?) usage;;
  esac
done
shift $(($OPTIND-1))

# mode settings
if [ $SNAPSHOT = 1 ]; then
  echo -n "mode:           snapshot"
else
  echo -n "mode:           RELASE"
fi
if [ $DEBUG = 1 ]; then
  echo " (DEBUG)"
else
  echo
fi

# check repository directory
SVN_REPO="$1"
if [ "x$SVN_REPO" = "x" ]; then
  echo "ERROR: no repository given!"
  usage
fi
if [ ! -d "$SVN_REPO" ]; then
  echo "ERROR: SVN repository '$SVN_REPO' not found!"
  exit 1
fi
if [ ! -s "$SVN_REPO/.svn" ]; then
  echo "ERROR: No SVN repository '$SVN_REPO'!"
  exit 1
fi
if [ ! -x "$SVN_REPO/autogen.sh" ]; then
  echo "ERROR: SVN repository '$SVN_REPO' contains no VICE!"
  exit 1
fi
SVN_REPO="`(cd \"$SVN_REPO\" && pwd -P)`"
echo "SVN repository: $SVN_REPO"

# get revision and branch of build
SVN_INFO=`(cd "$SVN_REPO" && svn info)`
REVISION=`echo "$SVN_INFO" | grep Revision | awk '{ print $2 }'`
SVN_URL=`echo "$SVN_INFO" | grep URL | awk '{ print $2 }'`
SVN_BRANCH=`echo "$SVN_URL" | sed -e 's,.*/vice-emu/,,' -e 's,/vice$,,' -e 's,branches/,,'`
SVN_BRANCH=`basename "$SVN_BRANCH" | sed -e 's,-,_,g' -e 's,\.,,g'`
echo "SVN revision:   $REVISION"
echo "SVN branch:     $SVN_BRANCH"

# check extlib directory
if [ ! -d "$EXTLIB" ]; then
  echo "ERROR: external libs '$EXTLIB' not found!"
  exit 1
fi
EXTLIB="`(cd \"$EXTLIB\" && pwd)`"
echo "external libs:  $EXTLIB"

# check target dir
if [ "x$BUILD_DIR" = x ]; then
  if [ $SNAPSHOT = 1 ]; then
    BUILD_DIR=BUILD-snapshot
  else
    BUILD_DIR=BUILD-release
  fi
fi
if [ -d "$BUILD_DIR" ]; then
  if [ $DEL_TARGET = 0 ]; then
    echo "ERROR: $BUILD_DIR already exists!"
    exit 1
  else
    rm -rf "$BUILD_DIR"
  fi
fi
BUILD_DIR="`pwd`/$BUILD_DIR"
echo "build dir:      $BUILD_DIR"

# create build src dir
mkdir -p "$BUILD_DIR"
if [ ! -d "$BUILD_DIR" ]; then
  echo "ERROR: can't creat dir!"
  exit 1
fi

SRC_DIR="$BUILD_DIR/src"
if [ $LINK_SRC = 1 ]; then
  # link existing source
  echo "linking src:    $SRC_DIR"
  ln -sf "$SVN_REPO" "$SRC_DIR"
else
  # export fresh source
  echo "exporting src:  $SRC_DIR"
  svn export -q "$SVN_REPO" "$SRC_DIR"
  if [ $? != 0 ]; then
    echo "ERROR: export faild!"
    exit 1
  fi
fi

# patch BUILD version if doing snapshot
if [ $SNAPSHOT = 1 -o "$OWN_TAG" != "" ]; then
  # tag
  DATE=`date '+%Y%m%d'`
  if [ "$OWN_TAG" != "" ]; then
      TAG="-$OWN_TAG"
  else
      TAG="-r${REVISION}_${DATE}_$SVN_BRANCH"
  fi

  # patch VICE_VERSION_BUILD
  echo "patching configure.proto: $TAG"
  PATCH_VVB="$SRC_DIR/configure.proto"
  perl -pi -e "s/(VICE_VERSION_BUILD=\d+).*\$/\$1$TAG/" $PATCH_VVB
  if [ $? != 0 ]; then
    echo "ERROR: patching..."
    exit 1
  fi
fi

# configure snapshot
echo "--- configuring source tree ---"
(cd "$SRC_DIR" && ./autogen.sh)
if [ $? != 0 ]; then
  echo "ERROR: configuring VICE..."
  exit 1
fi

# build dist jobs
if [ "x$JOBS" != "x" ]; then
  for JOB in $JOBS ; do
    
    # determine job parameters
    UI=$DEFAULT_UI
    ARCH=$DEFAULT_ARCH
    SDK_VERSION=$DEFAULT_SDK_VERSION
    COMPILER=$DEFAULT_COMPILER
    
    # parse job description
    JOB="`echo $JOB | sed -e 's/-/ /g'`"
    for PARAM in $DEFAULT_UI $DEFAULT_ARCH $DEFAULT_SDK_VERSION $DEFAULT_COMPILER $JOB ; do
      case "$PARAM" in
        sdl)    UI=sdl;;
        x11)    UI=x11;;
        gtk)    UI=gtk;;
        cocoa)  UI=cocoa;;
      
        i386)   ARCH=i386;;
        ppc)    ARCH=ppc;;
        x86_64) ARCH=x86_64;;
        *+*)    ARCH="$PARAM";;
        
        10.[4-9])   SDK_VERSION="$PARAM";;

        gcc40)  COMPILER=gcc40;;
        gcc42)  COMPILER=gcc42;;
        clang*) COMPILER="$PARAM";;
        
        ?) echo "Unknown Job Parameter!"; exit 1;;
      esac
    done
        
    # set output log
    TAG="$UI-$ARCH-$SDK_VERSION-$COMPILER"
    LOG="$BUILD_DIR/build-$TAG.log"
    
    # set dist type
    if [ $DIR_DIST = 1 ]; then
      DIST_TYPE="dir"
    else
      DIST_TYPE="dmg"
    fi
    
    # do build
    echo "--- building binaries [$UI-$ARCH-$SDK_VERSION-$COMPILER] ---"
    (cd "$SRC_DIR" && $BASH build/macosx/build-vice-dist.sh \
     "$ARCH" "$SDK_VERSION" "$COMPILER" "$UI" "$DIST_TYPE" "$EXTLIB" "$BUILD_DIR" "$DEBUG") \
     2>&1 | tee "$LOG" | grep ^+

    # check generated files
    if [ $DIR_DIST = 1 ]; then
      FILES="$(ls -d $BUILD_DIR/$UI-$SDK_VERSION-$COMPILER/$ARCH/vice-macosx-* 2>/dev/null)"
    else
      FILES="$(ls $BUILD_DIR/$UI-$SDK_VERSION-$COMPILER/$ARCH/vice-macosx-*.dmg 2>/dev/null)"
    fi
    echo "generated output: $FILES"
    if [ "x$FILES" = "x" ]; then
      echo "no file found!"
      exit 1
    fi

    # show warnings
    echo " -warnings begin-"
    fgrep warning: "$LOG" | sort | uniq
    echo " -warnings end-"

    # size and move files
    du -sh "$FILES"
    mv "$FILES" "$BUILD_DIR"
  done
fi
  
# packing source
if [ $ONLY_BIN = 0 ]; then
  echo "--- source ---"
  echo "packing source"
  DEST_DIR="$BUILD_DIR/tarball"
  mkdir "$DEST_DIR"
  if [ ! -d "$DEST_DIR" ]; then
    echo "ERROR: creating dir"
    exit 1
  fi
  LOG="$BUILD_DIR/build-tarball.log"
  (cd "$DEST_DIR" && $SRC_DIR/configure && make dist) >"$LOG" 2>&1
  # move generated files to top level
  FILES="$(ls $DEST_DIR/*.tar.gz 2>/dev/null)"
  if [ "x$FILES" = "x" ]; then
    echo "FAILED!"
    tail -10 $LOG
    exit 1
  fi
  echo "generated: $FILES"
  du -sh "$FILES"
  mv "$FILES" "$BUILD_DIR"
fi

echo "--- ready ---"
exit 0
