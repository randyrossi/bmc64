#!/bin/sh

#
# build-vice-dist.sh
#
# build a binary distribution of VICE for macs
# you can select the architecture (including universal binaries),
# the ui type (x11 or gtk) and the dist type (dir or dmg)
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
# call this script from vice top-level directory!
#

echo "---------- Build Mac Binary Distribution ----------"

# check for vice version in configure.proto
if [ ! -e configure.proto ]; then
  echo "please run this script from inside the VICE source directory!"
  exit 1
fi
# fetch vice version
eval `grep 'VICE_VERSION_[A-Z]*=' configure.proto`
if test x"$VICE_VERSION_BUILD" = "x" -o x"$VICE_VERSION_BUILD" = "x0" ; then
  VICE_VERSION=$VICE_VERSION_MAJOR"."$VICE_VERSION_MINOR
else
  VICE_VERSION=$VICE_VERSION_MAJOR"."$VICE_VERSION_MINOR"."$VICE_VERSION_BUILD
fi

# ----- check args -----
if [ "x$1" = "x" ]; then
  echo "Usage: $0 <arch> <sdk_version> <compiler> <ui-type> <dist-type> <extlib-dir> <build-dir> <debug>"
  echo "   arch        Build architecture       i386 ppc x86_64 ubi i386+ppc i386+x86_64"
  echo "   sdk-ver     Select SDK version       10.4 10.5 10.6"
  echo "   compiler    Select compiler suite    gcc40 gcc42 clang"
  echo "   ui-type     User Interface Type      sdl x11 gtk cocoa"
  echo "   dist-type   Type of Distribution     dmg dir"
  echo "   extlib-dir  External Libraries"
  echo "   build-dir   Where VICE is built"
  echo "   debug       Build debug version      0"
  exit 1
fi
ARCH="$1"
MULTI_ARCH="`echo \"$ARCH\" | sed -e 's/+/ /g'`"
for A in $MULTI_ARCH ; do
  if [ "$A" != "i386" -a "$A" != "ppc" -a "$A" != "x86_64" ]; then
    echo "Wrong architecture given: $A"
    exit 1
  fi
done
SDK_VERSION="$2"
case "$SDK_VERSION" in
  10.[4-9]) ;;
  *)
    echo "Wrong SDK version given: $SDK_VERSION"
    exit 1
    ;;
esac
COMPILER="$3"
case "$COMPILER" in
  gcc40) ;;
  gcc42) ;;
  clang*) ;;
  *)
    echo "Wrong compiler given: $COMPILER"
    exit 1
    ;;
esac
UI_TYPE="$4"
if [ "$UI_TYPE" != "x11" -a "$UI_TYPE" != "gtk" -a "$UI_TYPE" != "cocoa" -a "$UI_TYPE" != "sdl" ]; then
  echo "Wrong UI Type given: use 'x11' or 'gtk', or 'cocoa'!"
  exit 1
fi
DIST_TYPE="$5"
if [ "$DIST_TYPE" != "dir" -a "$DIST_TYPE" != "dmg" ]; then
  echo "Wrong Build Target: use 'dir' or 'dmg'!"
  exit 1
fi
EXTLIB_DIR="`cd \"$6\" && pwd`"
if [ ! -d "$EXTLIB_DIR" ]; then
  echo "ExtLib Directory '$EXTLIB_DIR' does not exist!"
  exit 1
fi
BUILD_DIR="`cd \"$7\" && pwd`"
if [ "$BUILD_DIR" = "" ]; then
  echo "No Build Directory given!"
  exit 1
fi
DEBUG="$8"
if [ "x$DEBUG" = "x" ]; then
  DEBUG=0
fi

echo "+  architecture: $ARCH [$MULTI_ARCH]"
echo "+  sdk version:  $SDK_VERSION"
echo "+  compiler:     $COMPILER"
echo "+  ui type:      $UI_TYPE"
echo "+  dist type:    $DIST_TYPE"
echo "+  ext lib dir:  $EXTLIB_DIR"
echo "+  build dir:    $BUILD_DIR"
echo "+  debug:        $DEBUG"

# ----- determine number of CPUs -----
NUM_CPUS=`hostinfo | grep 'processors are logically available' | awk '{print $1}'`
echo "+  cpu cores:    $NUM_CPUS"
echo

# ----- determine build options -----
echo "----- Determine Build Options -----"

# check if a library is available for the selected architectures
check_lib () {
  local libName="$1"
  echo -n "checking for library '$libName' ... "
  # check for arch lib
  for A in $MULTI_ARCH ; do
    local A_PATH="$EXTLIB_DIR/$A-$SDK_VERSION-$COMPILER/lib/$libName"
    if [ ! -f "$A_PATH" ]; then
      echo " $A build missing ($A_PATH)"
      false
      return
    fi
  done
  echo " available for $MULTI_ARCH"
  true
}

# default configure flags
CONFIGURE_FLAGS="--disable-nls"

# check for gtk+ libs
if [ "$UI_TYPE" = "gtk" ]; then
  check_lib "libgtk-x11-2.0.dylib"
  if [ "$?" != "0" ]; then
    echo "gtk requested, but no gtk+ libs found!"
    exit 1
  fi
  CONFIGURE_FLAGS="--enable-gnomeui $CONFIGURE_FLAGS"
  CONFIGURE_OPTS="Gtk+"
elif [ "$UI_TYPE" = "cocoa" ]; then
  CONFIGURE_FLAGS="--with-cocoa $CONFIGURE_FLAGS"
  CONFIGURE_OPTS="Cocoa"
elif [ "$UI_TYPE" = "sdl" ]; then
  CONFIGURE_FLAGS="--enable-sdlui $CONFIGURE_FLAGS"
  CONFIGURE_OPTS="SDL"  
fi

# check for hidutil
check_lib "libHIDUtilities.a"
if [ "$?" = "0" ]; then
  echo "+++ With Joystick Support +++"
  CONFIGURE_OPTS="Joystick $CONFIGURE_OPTS"
fi

# check for libpcap and libnet
if [ "$UI_TYPE" != "sdl" ]; then
  check_lib "libpcap.a"
  if [ "$?" = "0" ]; then
    check_lib "libnet.a"
    if [ "$?" = "0" ]; then
      CONFIGURE_FLAGS="--enable-ethernet $CONFIGURE_FLAGS"
      CONFIGURE_OPTS="Ethernet $CONFIGURE_OPTS"
      echo "+++ With Ethernet Support +++"
    fi
  fi
fi

# check for ffmpeg and lame
check_lib "libavcodec.dylib"
if [ "$?" = "0" ]; then
  check_lib "libmp3lame.dylib"
  if [ "$?" = "0" ]; then
    CONFIGURE_FLAGS="--enable-ffmpeg $CONFIGURE_FLAGS"
    CONFIGURE_OPTS="FFMPEG $CONFIGURE_OPTS"
    echo "+++ With FFMPEG + Lame Support +++"
  fi
fi

# clang workarounds for now 
if [ "$COMPILER" = "clang" ]; then
  CONFIGURE_FLAGS="$CONFIGURE_FLAGS"
fi

# ----- setup build dir -----
BUILD_DIR="$BUILD_DIR/$UI_TYPE-$SDK_VERSION-$COMPILER"
if [ ! -d "$BUILD_DIR" ]; then
  mkdir -p "$BUILD_DIR"
fi
if [ ! -d "$BUILD_DIR" ]; then
  echo "ERROR: Creating build directory '$BUILD_DIR' failed!"
  exit 1
fi
# create build subdirs (including 'ub' dirs)
for A in $ARCH $MULTI_ARCH ; do
  A_DIR="$BUILD_DIR/$A"
  if [ ! -d "$A_DIR" ]; then
    mkdir -p "$A_DIR";
  fi
done

# ----- Copy Gtk Config Files -----
if [ "$UI_TYPE" = "gtk" ]; then
  echo "----- Copy GTK Config Files -----"
  ANY_ARCH=${MULTI_ARCH%% *}
  # copy fontconfig 
  MYFONTCONFIG="$BUILD_DIR/$ARCH/etc/fonts"
  if [ ! -d "$MYFONTCONFIG" ]; then
    FONTCONFIG="$EXTLIB_DIR/$ANY_ARCH-$SDK_VERSION-$COMPILER/etc/fonts"
    if [ ! -d "$FONTCONFIG" ]; then
      echo "FontConfig dir not found in '$FONTCONFIG'"
      exit 1
    fi
    mkdir -p "$MYFONTCONFIG"
    (cd "$FONTCONFIG" && tar cf - *) | (cd "$MYFONTCONFIG" && tar xvf -)
  fi
fi

# ----- Compile VICE -----
export PATH=/usr/X11R6/bin:$PATH
if [ $DEBUG = 1 ]; then
  COMMON_CFLAGS="-O3 -g"
else
  COMMON_CFLAGS="-O3"
fi

# HACK: extra flags for GL compile
if [ "$UI_TYPE" != "cocoa" ]; then
  LDFLAGS_EXTRA="-dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
fi

build_vice () {
  local BUILD_ARCH="$1"
  local BUILD_SDK="$2"
  local BUILD_SDK_VERSION="$3"
  local BUILD_COMPILER="$4"
  local BUILD_CC="$5"
  local BUILD_CXX="$6"
  local VICE_SRC="`pwd`"
  local BUILD_ARCH2="$BUILD_ARCH"
  if [ "$BUILD_ARCH" = "ppc" ]; then
    BUILD_ARCH2="powerpc"
  fi
  local BUILD_TAG="$BUILD_ARCH-$BUILD_SDK_VERSION-$BUILD_COMPILER"
  local HOST_TAG="`uname -p`-$BUILD_SDK_VERSION-$BUILD_COMPILER"

  echo
  echo "+----- Bulding VICE [$BUILD_TAG] -----"
  echo "+  host:         $HOST_TAG"
  echo "+  sdk path:     $BUILD_SDK"
  echo "+  c compiler:   $BUILD_CC ($BUILD_CC_VERSION)"
  echo "+  c++ compiler: $BUILD_CXX ($BUILD_CXX_VERSION)"

  # already here?
  if [ -f "$BUILD_DIR/$BUILD_ARCH/src/x64" ]; then
    echo "  hmm... VICE already built for $BUILD_ARCH?! skipping..."
    return
  fi

  if test x"$BUILD_SDK" != "x"; then
    ISYSROOT="-isysroot $BUILD_SDK"
  else
    ISYSROOT=""
  fi

  # main VICE compile call
  pushd "$VICE_SRC"
  cd "$BUILD_DIR/$BUILD_ARCH"
  set -x
  env \
    PATH="$EXTLIB_DIR/$HOST_TAG/bin:$PATH" \
    CPPFLAGS="-I$EXTLIB_DIR/$BUILD_TAG/include" \
    CFLAGS="$COMMON_CFLAGS" \
    OBJCFLAGS="$COMMON_CFLAGS" \
    LDFLAGS="-L$EXTLIB_DIR/$BUILD_TAG/lib $LDFLAGS_EXTRA" \
    CC="$BUILD_CC -arch $BUILD_ARCH $ISYSROOT -mmacosx-version-min=$BUILD_SDK_VERSION" \
    CXX="$BUILD_CXX -arch $BUILD_ARCH $ISYSROOT -mmacosx-version-min=$BUILD_SDK_VERSION" \
    LD="$BUILD_CC -arch $BUILD_ARCH $ISYSROOT -mmacosx-version-min=$BUILD_SDK_VERSION" \
    $VICE_SRC/configure --host=$BUILD_ARCH2-apple-darwin $CONFIGURE_FLAGS \
      --x-includes=$BUILD_SDK/usr/X11R6/include --x-libraries=$BUILD_SDK/usr/X11R6/lib
  set +x
  make -j $NUM_CPUS 2>&1 | tee build.log 
  echo "--- Warnings ---" 
  fgrep warning: build.log
  popd

  # check if all went well
  if [ ! -f "$BUILD_DIR/$BUILD_ARCH/src/x64" ]; then
    echo "FATAL: no x64 in $BUILD_DIR/$BUILD_ARCH/src found!"
    exit 1
  fi
}

copy_dylib_rec () {
  local FILE="$1"
  local GOTLIB="$2"
  
  # get external libs
  echo 1>&2 "  checking binary '$FILE' for dylibs"
  local EXTLIB="`otool -L \"$FILE\" | grep dylib | grep -v /usr | grep -v /System | grep -v "$FILE" | cut -f1 -d ' '`"
  for lib in $EXTLIB ; do
    echo "$GOTLIB" | grep -q "$lib"
    if [ $? = 1 ]; then
      #echo 1>&2 "     lib...$lib"
      GOTLIB="$lib $GOTLIB"
      GOTLIB="`copy_dylib_rec \"$lib\" \"$GOTLIB\"`"
    fi
  done
  echo "$GOTLIB"
}

# copy the required dynamic libs
copy_dylib () {
  local BUILD_ARCH="$1"

  # get external libs
  echo "-- copy dylibs for $BUILD_ARCH --"
  EXTLIB="`copy_dylib_rec \"$BUILD_DIR/$BUILD_ARCH/src/x64\"`"

  # make sure lib dir exists
  local LIBDIR="$BUILD_DIR/$BUILD_ARCH/lib"
  if [ ! -d "$LIBDIR" ]; then
    mkdir -p "$LIBDIR"
  fi
  
  # copy lib if not already there
  for lib in $EXTLIB ; do
    LIBNAME="`basename \"$lib\"`"
    if [ ! -e "$LIBDIR/$LIBNAME" ]; then
      echo "  copying required dylib '$LIBNAME'"
      cp "$lib" "$LIBDIR/$LIBNAME"
    else
      echo "  required dylib '$LIBNAME' already here"
    fi
  done
}

fix_ref () {
  local exe="$1"
  local newpath="$2"
  local base="`basename \"$exe\"`"
  local LIBS=`otool -L "$exe" | grep .dylib | grep -v /usr | grep -v /System | awk '{ print $1 }'`
  echo -n "  fixing lib ref in '$1'"
  for lib in $LIBS ; do
    baselib="`basename \"$lib\"`"
    newlib="$newpath/$baselib"
    install_name_tool -change "$lib" "$newlib" "$exe"
    echo -n "."
  done
  cutbase="`basename \"$base\" .dylib`"
  if [ "$cutbase" != "$base" ]; then
    # adapt install name in lib
    libname="@executable_path/$base"
    install_name_tool -id "$libname" "$exe"
    echo -n "I"
  fi
  echo
}

# determine compiler name
case "$COMPILER" in
gcc40) 
  CC_NAME=gcc-4.0
  CXX_NAME=g++-4.0
  ;;
gcc42)
  CC_NAME=gcc-4.2
  CXX_NAME=g++-4.2
  ;;
clang_gcc)
  CC_NAME=clang
  CXX_NAME=llvm-g++
  ;;
clang)
  CC_NAME=clang
  CXX_NAME=clang++
  ;;
*)
  CC_NAME="$COMPILER"
  CXX_NAME="$COMPILER"
  ;;
esac

# get SDK name
case "$SDK_VERSION" in
10.4) SDK_NAME=MacOSX10.4u.sdk;;
*) SDK_NAME=MacOSX${SDK_VERSION}.sdk;;
esac

case "$SDK_VERSION" in
10.8) SDK_SEARCH="no";;
*) SDK_SEARCH="yes";;
esac

# search compiler and SDK
if test x"$SDK_SEARCH" != "xyes"; then
  BUILD_CC="$CC_NAME"
  BUILD_CXX="$CXX_NAME"
  SDK_PATH=""
else
  FOUND=0
  if [ "$DEV_BASES" = "" ]; then
    DEV_BASES="/Developer /Developer3"
  fi
  for BASE in $DEV_BASES ; do
    if [ $FOUND -eq 0 ]; then
      TRY_CC="$BASE/usr/bin/$CC_NAME"
      TRY_CXX="$BASE/usr/bin/$CXX_NAME"
      TRY_SDK="$BASE/SDKs/$SDK_NAME"
      echo "TRY_CC=$TRY_CC, TRY_CXX=$TRY_CXX, TRY_SDK=$TRY_SDK"
      if [ -x "$TRY_CC" -a -x "$TRY_CXX" -a -d "$TRY_SDK" ]; then
        BUILD_CC="$TRY_CC"
        BUILD_CXX="$TRY_CXX"
        SDK_PATH="$TRY_SDK"
        FOUND=1
      fi
    fi
  done
  if [ $FOUND -eq 0 ]; then
    echo "ERROR: C compiler, C++ compiler, or SDK not found!"
    exit 1
  fi
fi

# get versions of compiler
BUILD_CC_VERSION=$($BUILD_CC --version | head -1)
BUILD_CXX_VERSION=$($BUILD_CXX --version | head -1)

# build vice
for A in $MULTI_ARCH ; do
  build_vice "$A" "$SDK_PATH" "$SDK_VERSION" "$COMPILER" "$BUILD_CC" "$BUILD_CXX"
  copy_dylib "$A"
done

APPS="c1541 cartconv petcat x128 x64 x64sc x64dtv xcbm2 xcbm5x0 xpet xplus4 xvic"

# ----- Fixin Lib Refs -----
echo "----- Fixing Library References -----"
if [ "$UI_TYPE" = "x11" -o "$UI_TYPE" = "gtk" ]; then
  FIX_LIB_PATH="@executable_path/../lib"
else
  FIX_LIB_PATH="@executable_path"
fi
# in all apps
for app in $APPS ; do
  for A in $MULTI_ARCH ; do
    fix_ref "$BUILD_DIR/$A/src/$app" "$FIX_LIB_PATH"
  done
done
# and all libs
for lib in $EXTLIB ; do
  LIBNAME="`basename \"$lib\"`"
  for A in $MULTI_ARCH ; do
    fix_ref "$BUILD_DIR/$A/lib/$LIBNAME" "$FIX_LIB_PATH"
  done
done

# ----- Create Universal Binary -----
if [ "$ARCH" != "$MULTI_ARCH" ]; then
  echo "----- Combining Binaries into Universal Binaries -----"
  mkdir -p "$BUILD_DIR/$ARCH/src"  
  if [ -f "$BUILD_DIR/$ARCH/src/x64" ]; then
    echo "  hmm... Already combined?! skipping..."
  else
    for app in $APPS ; do
      MULTI_APP=""
      for A in $MULTI_ARCH ; do
        MULTI_APP="$BUILD_DIR/$A/src/$app $MULTI_APP"
      done
      echo "  combining '$app'"
      lipo -create -output "$BUILD_DIR/$ARCH/src/$app" $MULTI_APP
    done
    if [ ! -f "$BUILD_DIR/$ARCH/src/x64" ]; then
      echo "FATAL: no universal x64 found!"
      exit 1
    fi
  fi
  
  # combine dynamic libs
  mkdir -p "$BUILD_DIR/$ARCH/lib"
  for lib in $EXTLIB ; do
    LIBNAME="`basename \"$lib\"`"
    if [ ! -e "$BUILD_DIR/$ARCH/lib/$LIBNAME" ]; then
      MULTI_LIB=""
      for A in $MULTI_ARCH ; do
        MULTI_LIB="$BUILD_DIR/$A/lib/$LIBNAME $MULTI_LIB"
      done
      echo "  combining dylib '$LIBNAME'"
      lipo -create -output "$BUILD_DIR/$ARCH/lib/$LIBNAME" $MULTI_LIB
    else
      echo "  already combined dylib '$LIBNAME'"
    fi
  done
fi

# ----- Bundle Applications and Distribution -----
echo "----- Distribution Creation -----"
VICE_SRC="`pwd`"
ZIP="zip"
if [ "$DIST_TYPE" = "dir" ]; then
  ZIP="nozip"
fi
if [ $DEBUG = 1 ]; then
  STRIP="nostrip"
else
  STRIP="strip"
fi
# make-bindist.sh <top_srcdir> <strip> <vice-version> <--enable-arch> <zip|nozip> <x64sc-included> <ui_type> [bin_format]
#                 $1           $2      $3             $4              $5          $6               $7        $8
(cd "$BUILD_DIR/$ARCH" && \
$SHELL $VICE_SRC/src/arch/unix/macosx/make-bindist.sh $VICE_SRC $STRIP $VICE_VERSION no $ZIP yes $UI_TYPE "$ARCH-$SDK_VERSION-$COMPILER")

echo "----- Ready: architecture: [$ARCH-$SDK_VERSION-$COMPILER,$UI_TYPE,$DIST_TYPE] -----"
echo "VICE was configured with: $CONFIGURE_OPTS"
exit 0
