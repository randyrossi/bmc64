#!/bin/bash

#
# build-sdlmain.sh - build the SDLMain library from the devel-lite source of the SDL framework
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
# Usage:     build-sdlmain.sh <build-dir> <arch:ppc|i386>
#
# call this function twice: for ppc and i386
#

# includes of SDL
SDL_INCLUDES="/Library/Frameworks/SDL.framework/Headers"

# get build tools
SCRIPT_DIR="`dirname \"$0\"`"
. "$SCRIPT_DIR/build-inc.sh"

# parse args
parse_args "$@"

# create target dirs
make_dirs lib include

# setup compiler environment
set_compiler_env

echo "===== SDLMain build $BUILD_TAG ====="

# check if lib is already available
SDLMAIN_LIB="libSDLmain.a"
SDLMAIN_HDR="SDLmain.h"
if [ -e "$INSTALL_DIR/lib/$SDLMAIN_LIB" -a -e "$INSTALL_DIR/include/$SDLMAIN_HDR" ]; then
  echo "  SDLMain library already for $ARCH installed. ($SDLMAIN_LIB, $SDLMAIN_HDR exists)"
  exit 0
fi

# check for source files
M_FILE="SDLMain.m"
H_FILE="SDLMain.h"
if [ ! -e "$M_FILE" -o ! -e "$H_FILE" ]; then
  echo "FATAL: Sources missing: $M_FILE $H_FILE"
  exit 1
fi

# compile files
O_FILE="SDLMain.o"
echo "compiling $M_FILE ($ARCH)"
$CC $CFLAGS $CPPFLAGS -I$SDL_INCLUDES -O3 -W -c "$M_FILE" -o "$O_FILE"
if [ ! -e "$O_FILE" ]; then
  echo "FATAL: compile failed in file: $C_FILE"
  exit 1
fi

# build archive
A_FILE="$INSTALL_DIR/lib/$SDLMAIN_LIB"
ar cr "$A_FILE" "$O_FILE"
if [ ! -e "$A_FILE" ]; then
  echo "FATAL: archive missing: $A_FILE"
  exit 1
fi
echo "preparing $A_FILE"
ranlib "$A_FILE"

# install header
if [ ! -e "$INSTALL_DIR/include/$SDLMAIN_HDR" ]; then
  echo "installing header $SDLMAIN_HDR"
  cp "$H_FILE" "$INSTALL_DIR/include/$SDLMAIN_HDR"
fi

# clean up source
rm -f "$O_FILE"

echo "===== SDLMain ready $BUILD_TAG ====="
