#!/bin/sh

# This script will download and setup a mingw32 cross-compiler and
# it will configure, compile and package VICE.

# This is my script for building a complete cross-compiler toolchain.
# It is based partly on Ray Kelm's script, which in turn was built on
# Mo Dejong's script for doing the same, but with some added fixes.
# The intent with this script is to build a cross-compiled version
# of the current MinGW environment.
#
# Updated by Sam Lantinga <slouken@libsdl.org>

# what flavor are we building?

TARGET=i386-mingw32msvc

# you probably don't need to change anything from here down

TOPDIR=`pwd`
SRCDIR="$TOPDIR/source"

# These are the files from the MinGW 3.1 release

MINGW_URL=http://www.viceteam.org/online/mingw32
GCC_VERSION=3.4.2-20040916-1
GCC=gcc-$GCC_VERSION
GCC_ARCHIVES="gcc-core-$GCC_VERSION-src.tar.gz gcc-g++-$GCC_VERSION-src.tar.gz"
GCC_PATCH=""
BINUTILS=binutils-2.16.91-20060119-1
BINUTILS_ARCHIVE=$BINUTILS-src.tar.gz
MINGW=mingw-runtime-3.3
MINGW_ARCHIVE=$MINGW.tar.gz
W32API=w32api-2.5
W32API_ARCHIVE=$W32API.tar.gz

# These are the files from the SDL website

SDL_URL=http://www.viceteam.org/online/mingw32
OPENGL_ARCHIVE=opengl-devel.tar.gz
DIRECTX_ARCHIVE=directx-devel.tar.gz

# where does it go?

LOCAL_PREFIX="$TOPDIR/mingw32-$GCC_VERSION"
GLOBAL_PREFIX="/usr/local/bin/mingw32-$GCC_VERSION"

#
# download a file from a given url, only if it is not present
#

download_file()
{
	cd "$SRCDIR"
	if test ! -f $1 ; then
		echo "Downloading $1"
		wget "$2/$1"
		if test ! -f $1 ; then
			echo "Could not download $1"
			exit 1
		fi
	else
		echo "Found $1 in the srcdir $SRCDIR"
	fi
  	cd "$TOPDIR"
}

download_files()
{
	mkdir -p "$SRCDIR"
	
	# Make sure wget is installed
	if test "x`which wget`" = "x" ; then
		echo "You need to install wget."
		exit 1
	fi
	for file in $GCC_ARCHIVES; do
		download_file "$file" "$MINGW_URL"
	done
	download_file "$BINUTILS_ARCHIVE" "$MINGW_URL"
	download_file "$MINGW_ARCHIVE" "$MINGW_URL"
	download_file "$W32API_ARCHIVE" "$MINGW_URL"
	download_file "$OPENGL_ARCHIVE" "$SDL_URL"
	download_file "$DIRECTX_ARCHIVE" "$SDL_URL"
}

install_libs()
{
	echo "Installing cross libs and includes"
	mkdir -p "$PREFIX/$TARGET"
	cd "$PREFIX/$TARGET"
	gzip -dc "$SRCDIR/$MINGW_ARCHIVE" | tar xf -
	gzip -dc "$SRCDIR/$W32API_ARCHIVE" | tar xf -
	gzip -dc "$SRCDIR/$OPENGL_ARCHIVE" | tar xf -
	gzip -dc "$SRCDIR/$DIRECTX_ARCHIVE" | tar xf -
	cd "$TOPDIR"
}

extract_binutils()
{
	cd "$SRCDIR"
	rm -rf "$BINUTILS"
	echo "Extracting binutils"
	gzip -dc "$SRCDIR/$BINUTILS_ARCHIVE" | tar xf -
	cd "$TOPDIR"
}

configure_binutils()
{
	cd "$TOPDIR"
	rm -rf "binutils-$TARGET"
	mkdir "binutils-$TARGET"
	cd "binutils-$TARGET"
	echo "Configuring binutils"
	"$SRCDIR/$BINUTILS/configure" --prefix="$PREFIX" --target=$TARGET
	cd "$TOPDIR"
}

build_binutils()
{
	cd "$TOPDIR/binutils-$TARGET"
	echo "Building binutils"
	make
	if test $? -ne 0; then
		echo "make failed"
		exit 1
	fi
	cd "$TOPDIR"
}

install_binutils()
{
	cd "$TOPDIR/binutils-$TARGET"
	echo "Installing binutils"
	make install
	if test $? -ne 0; then
		echo "install failed"
		exit 1
	fi
	cd "$TOPDIR"
}

rmdir_binutils()
{
  echo "Removing temporary binutils source and build directories."
  rm -rf "$SRCDIR/$BINUTILS"
  rm -rf "$TOPDIR/binutils-$TARGET"
}

extract_gcc()
{
	cd "$SRCDIR"
	rm -rf "$GCC"
	echo "Extracting gcc"
	for file in $GCC_ARCHIVES; do
		gzip -dc "$SRCDIR/$file" | tar xf -
	done
	cd "$TOPDIR"
}

patch_gcc()
{
	if [ "$GCC_PATCH" != "" ]; then
		echo "Patching gcc"
		cd "$SRCDIR/$GCC"
		patch -p1 < "$SRCDIR/$GCC_PATCH"
		cd "$TOPDIR"
	fi
}

configure_gcc()
{
	cd "$TOPDIR"
	rm -rf "gcc-$TARGET"
	mkdir "gcc-$TARGET"
	cd "gcc-$TARGET"
	echo "Configuring gcc"
	"$SRCDIR/$GCC/configure" -v \
		--prefix="$PREFIX" --target=$TARGET \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld \
		--without-newlib --disable-multilib
	cd "$TOPDIR"
}

build_gcc()
{
	cd "$TOPDIR/gcc-$TARGET"
	echo "Building gcc"
	make
	if test $? -ne 0; then
		echo "make failed"
		exit 1
	fi
	cd "$TOPDIR"
}

install_gcc()
{
	cd "$TOPDIR/gcc-$TARGET"
	echo "Installing gcc"
	make LANGUAGES="c c++" install
	if test $? -ne 0; then
		echo "install failed"
		exit 1
	fi
	cd "$TOPDIR"
}

rmdir_gcc()
{
  echo "Removing temporary gcc source and build directories."
  rm -rf "$SRCDIR/$GCC"
  rm -rf "$TOPDIR/gcc-$TARGET"
}

final_tweaks()
{
	echo "Finalizing installation"

	# remove gcc build headers
	rm -rf "$PREFIX/$TARGET/sys-include"

        # Add extra binary links
	if [ ! -f "$PREFIX/$TARGET/bin/objdump" ]; then
		ln "$PREFIX/bin/$TARGET-objdump" "$PREFIX/$TARGET/bin/objdump"
	fi

	# make cc and c++ symlinks to gcc and g++
	if [ ! -f "$PREFIX/$TARGET/bin/g++" ]; then
		ln "$PREFIX/bin/$TARGET-g++" "$PREFIX/$TARGET/bin/g++"
	fi
	if [ ! -f "$PREFIX/$TARGET/bin/cc" ]; then
		ln -s "gcc" "$PREFIX/$TARGET/bin/cc"
	fi
	if [ ! -f "$PREFIX/$TARGET/bin/c++" ]; then
		ln -s "g++" "$PREFIX/$TARGET/bin/c++"
	fi

	# strip all the binaries
	ls "$PREFIX"/bin/* "$PREFIX/$TARGET"/bin/* | egrep -v '.dll$' |
	while read file; do
		strip "$file"
	done

	echo "Installation complete!"
}

SELECTION=0

setup_compiler()
{
  while [ $SELECTION -eq 0 ]
  do
    echo
    echo "1: Install local compiler environment."
    echo "2: Install global compiler environment."
    read SELECTION
    if [ "$SELECTION" -ne 1 -a "$SELECTION" -ne 2 ]
    then
      SELECTION=0
    fi
  done

  if [ "$SELECTION" -eq 1 ]
  then
    PREFIX="$LOCAL_PREFIX"
  fi

  if [ "$SELECTION" -eq 2 ]
  then
    PREFIX="$GLOBAL_PREFIX"
  fi

  PATH="$PREFIX/bin:$PATH"

  download_files

  install_libs

  extract_binutils
  configure_binutils
  build_binutils
  install_binutils

  extract_gcc
  patch_gcc
  configure_gcc
  build_gcc
  install_gcc

  final_tweaks

  rmdir_binutils
  rmdir_gcc
}

check_compiler()
{
  echo
  echo "Checking for local mingw32 compiler environment."
  if [ -f "$LOCAL_PREFIX/bin/$TARGET-gcc" ]; then
    echo "Local mingw32 compiler environment found."
    PREFIX="$LOCAL_PREFIX"
    PATH="$PREFIX/bin:$PATH"
  else
    echo "No local mingw32 compiler environment found."
    echo "Checking for global mingw32 compiler environment."
    if [ -f "$GLOBAL_PREFIX/bin/$TARGET-gcc" ]; then
      echo "Global mingw32 compiler environment found."
      PREFIX="$GLOBAL_PREFIX"
      PATH="$PREFIX/bin:$PATH"
    else
      echo "No global mingw32 compiler environment found."
      setup_compiler
    fi        
  fi
}

create_vice()
{
  TGR_CFLAGS="-Wall -mtune=pentium -O5 -fomit-frame-pointer"
  TGR_CXXFLAGS="$TGR_CFLAGS -fno-exceptions"

  TRG_CXX="$PREFIX/bin/$TARGET-c++"
  TRG_CC="$PREFIX/bin/$TARGET-gcc"
  TRG_AR="$PREFIX/bin/$TARGET-ar"
  TRG_RANLIB="$PREFIX/bin/$TARGET-ranlib"
  TRG_STRIP="$PREFIX/bin/$TARGET-strip"
  TRG_WINDRES="$PREFIX/bin/$TARGET-windres"

  cd ../..

  echo "Configure VICE..."
  CFLAGS="$TGR_CFLAGS" CXXFLAGS="$TGR_CXXFLAGS" CXX="$TRG_CXX" CC="$TRG_CC" AR="$TRG_AR" RANLIB="$TRG_RANLIB" STRIP="$TRG_STRIP" WINDRES="$TRG_WINDRES" ./configure --enable-ffmpeg --host=i386-mingw32 --enable-native-tools $*

  echo "Make VICE..."
  make

  echo "Package VICE..."
  make bindistzip

  cd "$TOPDIR"
}

check_compiler
create_vice
