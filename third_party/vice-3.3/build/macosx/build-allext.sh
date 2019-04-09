#!/bin/bash

#
# build-allext.sh - build the allext on ppc+i386 macs for VICE
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

# get build tools
SCRIPT_DIR="`dirname \"$0\"`"

run () {
  echo "--- $@ ---"
  $SHELL "$@"
  if [ $? != 0 ]; then
    echo "*** FAILED: $@"
    exit 1
  fi
}

# hidutils
run "$SCRIPT_DIR/build-hidutils.sh" "$1" ppc 10.4 gcc40 $2
run "$SCRIPT_DIR/build-hidutils.sh" "$1" i386 10.4 gcc40 $2
run "$SCRIPT_DIR/build-hidutils.sh" "$1" ppc 10.5 gcc40 $2
run "$SCRIPT_DIR/build-hidutils.sh" "$1" i386 10.5 gcc40 $2
run "$SCRIPT_DIR/build-hidutils.sh" "$1" i386 10.6 gcc42 $2
run "$SCRIPT_DIR/build-hidutils.sh" "$1" x86_64 10.6 gcc42 $2
run "$SCRIPT_DIR/build-hidutils.sh" "$1" i386 10.6 clang $2
run "$SCRIPT_DIR/build-hidutils.sh" "$1" x86_64 10.6 clang $2

# pcaplibnet
run "$SCRIPT_DIR/build-pcaplibnet.sh" "$1" ppc 10.4 gcc40 $2
run "$SCRIPT_DIR/build-pcaplibnet.sh" "$1" i386 10.4 gcc40 $2
run "$SCRIPT_DIR/build-pcaplibnet.sh" "$1" ppc 10.5 gcc40 $2
run "$SCRIPT_DIR/build-pcaplibnet.sh" "$1" i386 10.5 gcc40 $2
run "$SCRIPT_DIR/build-pcaplibnet.sh" "$1" i386 10.6 gcc42 $2
run "$SCRIPT_DIR/build-pcaplibnet.sh" "$1" x86_64 10.6 gcc42 $2
run "$SCRIPT_DIR/build-pcaplibnet.sh" "$1" i386 10.6 clang $2
run "$SCRIPT_DIR/build-pcaplibnet.sh" "$1" x86_64 10.6 clang $2

# ffmpeg
run "$SCRIPT_DIR/build-ffmpeglame.sh" "$1" ppc 10.4 gcc40 $2
run "$SCRIPT_DIR/build-ffmpeglame.sh" "$1" i386 10.4 gcc40 $2
run "$SCRIPT_DIR/build-ffmpeglame.sh" "$1" ppc 10.5 gcc40 $2
run "$SCRIPT_DIR/build-ffmpeglame.sh" "$1" i386 10.5 gcc40 $2
run "$SCRIPT_DIR/build-ffmpeglame.sh" "$1" i386 10.6 gcc42 $2
run "$SCRIPT_DIR/build-ffmpeglame.sh" "$1" x86_64 10.6 gcc42 $2
run "$SCRIPT_DIR/build-ffmpeglame.sh" "$1" i386 10.6 clang $2
run "$SCRIPT_DIR/build-ffmpeglame.sh" "$1" x86_64 10.6 clang $2

# gtk
run "$SCRIPT_DIR/build-gtk.sh" "$1" ppc 10.4 gcc40 $2
run "$SCRIPT_DIR/build-gtk.sh" "$1" i386 10.4 gcc40 $2

# SDLmain
run "$SCRIPT_DIR/build-sdlmain.sh" "$1" ppc 10.4 gcc40 $2
run "$SCRIPT_DIR/build-sdlmain.sh" "$1" i386 10.4 gcc40 $2

echo "========== build-allext: ready =========="