#!/bin/bash

#
# build-inc.sh - include file for other build-* scripts
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

parse_args () {
  # get arguments
  BASE_DIR="$1"
  ARCH="$2"
  SDK_VERSION="$3"
  COMPILER="$4"
  export FORCE_BUILD="$5"
  if [ "x$ARCH" = "x" ]; then
    echo "Usage: $0 <build-dir> <arch:ppc|i386|x86_64> [sdk:10.4|10.5|10.6] [gcc40|gcc42|clang] [force build!=0]"
    exit 1
  fi
  if [ ! -d "$BASE_DIR" ]; then
    echo "Missing base dir!"
    exit 1
  fi
  # normalize base dir
  BASE_DIR="`cd \"$BASE_DIR\" && pwd`"

  # check arch
  if [ "$ARCH" = "ppc" ]; then
    INSTALL_DIR="$BASE_DIR/ppc"
  elif [ "$ARCH" = "i386" ]; then
    INSTALL_DIR="$BASE_DIR/i386"
  elif [ "$ARCH" = "x86_64" ]; then
    INSTALL_DIR="$BASE_DIR/x86_64"
  else
    echo "Unknown ARCH: $ARCH (ppc|i386|x86_64)"
    exit 1
  fi

  # autoselect SDK
  if [ "x$SDK_VERSION" = "x" ]; then
    if [ "$ARCH" = "x86_64" ]; then
      SDK_VERSION="10.6"
    else
      SDK_VERSION="10.4"
    fi
  fi
  
  # check SDK
  if [ "$SDK_VERSION" = "10.4" ]; then
    SDK=/Developer/SDKs/MacOSX10.4u.sdk
  elif [ "$SDK_VERSION" = "10.5" ]; then
    SDK=/Developer/SDKs/MacOSX10.5.sdk
  elif [ "$SDK_VERSION" = "10.6" ]; then  
    SDK=/Developer/SDKs/MacOSX10.6.sdk
  else
    echo "Unknown SDK_VERSION: $SDK_VERSION"
    exit 1
  fi

  # autoselect compiler
  if [ "x$COMPILER" = "x" ]; then
    if [ "$SDK_VERSION" = "10.6" ]; then
      COMPILER="gcc42"
    else
      COMPILER="gcc40"
    fi
  fi

  # check compiler
  if [ "$COMPILER" = "gcc40" ]; then
    GCC="/usr/bin/gcc-4.0"
    GXX="/usr/bin/g++-4.0"
  elif [ "$COMPILER" = "gcc42" ]; then
    GCC="/usr/bin/gcc-4.2"
    GXX="/usr/bin/g++-4.2"
  elif [ "$COMPILER" = "clang" ]; then
    GCC="/Developer/usr/bin/clang"
    GXX="/Developer/usr/bin/llvm-g++-4.2"
  else
    echo "Unknown COMPILER: $COMPILER"
    exit 1
  fi

  # extend install dir
  INSTALL_DIR="$INSTALL_DIR-$SDK_VERSION-$COMPILER"

  # setup base dir
  if [ ! -d "$INSTALL_DIR" ]; then
    echo "  creating install dir $INSTALL_DIR"
    mkdir -p "$INSTALL_DIR"
    mkdir -p "$INSTALL_DIR/bin"
    mkdir -p "$INSTALL_DIR/include"
    mkdir -p "$INSTALL_DIR/lib"
    mkdir -p "$INSTALL_DIR/man"
  fi
  
  # number of cpus
  if [ "x$MULTI_CPU" != "x" ]; then
    NUM_CPUS=`hostinfo | grep 'processors are logically available' | awk '{print $1}'`
  else
    NUM_CPUS=1
  fi
  export NUM_CPUS
  
  # build tag
  export BUILD_TAG="[$ARCH-$SDK_VERSION-$COMPILER,force=$FORCE_BUILD,cpus=$NUM_CPUS]"
}

# configure/compile/install a autoconf'ed distribution
#
# SRC         source archive
# DIR         source directory
# CHECK_FILE  if this file exists then assume build is already done
# INSTALL     rule to install
# URL         where to download archive if its missing
# CONFIG_OPT  extra switches for configure
#
configure_make_install () {
  SRC="$1"
  DIR="$2"
  CHECK_FILE="$3"
  INSTALL="$4"
  URL="$5"
  CONFIG_OPT="$6"

  echo "----- $DIR $BUILD_TAG -----"
  
  # check if lib is available
  if [ -e "$INSTALL_DIR/$CHECK_FILE" -a "x$FORCE_BUILD" = "x" ]; then
    echo "  Already installed. ($CHECK_FILE available)"
  else
    
    # check for source archive
    if [ "x$SRC" = "x" ]; then
      if [ ! -d "$DIR" ]; then
        echo "FATAL: source dir '$DIR' is missing!"
        echo "       please setup first (from e.g. $URL)"
        exit 1
      fi
    else
      if [ ! -e "$SRC" ]; then
        echo "FATAL: source archive '$SRC' missing in curent directory!"
        echo "       please download first (from e.g. $URL)"
        exit 1
      fi
    fi
    
    # check if source is already unpacked
    if [ ! -d "$DIR" ]; then
      echo "  Unpacking source for $DIR"
      echo "$SRC" | grep .bz2 > /dev/null
      if [ $? != 0 ]; then 
        tar xfz "$SRC"
      else
        tar xfj "$SRC"
      fi
    fi
    if [ ! -d "$DIR" ]; then
      echo "FATAL: source not unpacked to $DIR"
      exit 1
    fi
    
    # prepare BUILD
    if [ "$COMPILE_IN_SOURCE" != "" ]; then
      BUILD_DIR="$DIR"
    else
      BUILD_DIR="BUILD"
      if [ -d "$BUILD_DIR" ]; then
        if [ "x$FORCE_BUILD" != "x" ]; then
          rm -rf "$BUILD_DIR"
        else
          echo "FATAL: build directory '$BUILD_DIR' already here!"
          exit 1
        fi
      fi
      mkdir BUILD
    fi
    
    # patch source?
    if [ "$PATCH" != "" ]; then
      (cd "$DIR" && eval "$PATCH")
    fi
    
    # build
    echo "  configure options: $CONFIG_OPT"
    (cd "$BUILD_DIR" && eval "../$DIR/configure --prefix=\"$INSTALL_DIR\" $CONFIG_OPT $EXTRA_OPT")
    (cd "$BUILD_DIR" && make -j$NUM_CPUS)
    if [ "$?" != "0" ]; then
      echo "FATAL: make failed!"
      exit 1
    fi
    echo "make $INSTALL"
    (cd "$BUILD_DIR" && make $INSTALL)
    
    # check for lib
    if [ ! -e "$INSTALL_DIR/$CHECK_FILE" ]; then
      echo "FATAL: $CHECK_FILE not found!"
      exit 1
    fi
    
    # clean up
    if [ "$COMPILE_IN_SOURCE" = "" ]; then
      rm -rf BUILD
    fi
    if [ "x$SRC" != "x" ]; then
      rm -rf "$DIR"
    fi
    
    echo "----- ready with $2 $BUILD_TAG -----" 
  fi
}

# create directories in install dir
make_dirs () {
  for DIR in "$@" ; do
    if [ ! -d "$INSTALL_DIR/$DIR" ]; then
      echo "  creating directory $DIR"
      mkdir -p "$INSTALL_DIR/$DIR"
    fi
  done
}

set_compiler_env () {    
  # set common flags
  export CPPFLAGS="-I$INSTALL_DIR/include"
  export LDFLAGS="-L$INSTALL_DIR/lib"
  export PATH="$INSTALL_DIR/bin:$PATH"

  # use X11 from SDK
  if [ "$USE_X11" = "1" ]; then 
    EXTRA_OPT="--x-includes=$SDK/usr/X11R6/include --x-libraries=$SDK/usr/X11R6/lib"
  fi
  
  export COMPILE_TAG="-arch $ARCH -isysroot $SDK -mmacosx-version-min=$SDK_VERSION"
  export CC="$GCC $COMPILE_TAG"
  export CXX="$GXX $COMPILE_TAG"
  export LD="$GCC $COMPILE_TAG"
}
