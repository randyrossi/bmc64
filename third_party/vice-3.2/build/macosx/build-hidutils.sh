#!/bin/bash

#
# build-hidutils.sh - build the hidutils on macs for joystick support in VICE
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
# Usage:     build-hidutils.sh <build-dir> <arch:ppc|i386>
#

# get build tools
SCRIPT_DIR="`dirname \"$0\"`"
. "$SCRIPT_DIR/build-inc.sh"

# parse args
parse_args "$@"

# create target dirs
make_dirs lib include

# setup compiler environment
set_compiler_env

echo "===== hidutils build $BUILD_TAG ====="

# check if lib is already available
HIDUTIL_LIB="libHIDUtilities.a"
HIDUTIL_HDR="HID_Utilities_External.h"
if [ -e "$INSTALL_DIR/lib/$HIDUTIL_LIB" -a -e "$INSTALL_DIR/include/$HIDUTIL_HDR" -a "x$FORCE_BUILD" = "x" ]; then
  echo "  hidutil library already for $ARCH installed. ($HIDUTIL_LIB, $HIDUTIL_HDR exists)"
  exit 0
fi

# unzip source
DIR="HID Utilities Source"
if [ ! -d "$DIR" ]; then
  # check for file
  SRC="HID_Utilities_Source.zip"
  if [ ! -e "$SRC" ]; then
    echo "FATAL: '$SRC' missing!"
    echo "       please download from http://developer.apple.com/samplecode/HID_Utilities_Source/index.html"
    exit 1
  fi

  # unzip source
  unzip "$SRC"
  if [ ! -d "$DIR" ]; then
    echo "FATAL: '$DIR' does not exits! unzip failed???"
    exit 1
  fi
  
  # patch source
  sed -i bak -e 's,^extern long HIDCalibrateValue,//,g' -e 's,^extern long HIDScale,//,g' "$DIR/HID_Utilities_External.h"
  sed -i bak -e 's,#define kVerboseErrors,//,' "$DIR/HID_Error_Handler.c"
  
fi

# compile files
A_FILE="$INSTALL_DIR/lib/$HIDUTIL_LIB"
HIDUTIL_SRC_FILES="HID_Config_Utilities HID_Error_Handler HID_Name_Lookup \
  HID_Queue_Utilities HID_Transaction_Utilities HID_Utilities"
for src in $HIDUTIL_SRC_FILES ; do
  C_FILE="$DIR/$src.c"
  O_FILE="$DIR/$src.o"
  if [ ! -e "$C_FILE" ]; then
    echo "FATAL: missing source file: $C_FILE"
    exit 1
  fi
  echo "compiling $src ($ARCH)"
  $CC $CFLAGS $CPPFLAGS -O3 -W -c "$C_FILE" -o "$O_FILE"
  if [ ! -e "$O_FILE" ]; then
    echo "FATAL: compile failed in file: $C_FILE"
    exit 1
  fi
  # build archive
  ar cr "$A_FILE" "$O_FILE"
done

# check archive
if [ ! -e "$A_FILE" ]; then
  echo "FATAL: archive missing: $A_FILE"
  exit 1
fi
echo "preparing $A_FILE"
ranlib "$A_FILE"

# install header
if [ ! -e "$INSTALL_DIR/include/$HIDUTIL_HDR" ]; then
  echo "installing header $HIDUTIL_HDR"
  cp "$DIR/$HIDUTIL_HDR" "$INSTALL_DIR/include/"
fi

# clean up source
rm -rf "$DIR"

echo "===== hidutils ready $BUILD_TAG ====="
