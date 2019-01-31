#!/bin/bash

#
# build-ffmpeglame.sh - build the ffmpeg and mp3lame on macs for VICE
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
# Usage:     build-ffmpeglame.sh <build-dir> <arch:ppc|i386>
#

# get build tools
SCRIPT_DIR="`dirname \"$0\"`"
. "$SCRIPT_DIR/build-inc.sh"

# parse args
parse_args "$@"

# setup compiler environment
set_compiler_env

echo "===== ffmpeg+lame build $BUILD_TAG ====="

configure_make_install lame-3.97.tar.gz lame-3.97 lib/libmp3lame.dylib install \
                       "http://lame.sourceforge.net/index.php" \
                       "--disable-static"
                       
PATCH="touch SDL.h SDL_thread.h ; \
cd ../ffmpeg-svn/ && cp configure c.tmp && sed -e 's/cc=\"cc\"//' < c.tmp > configure"
configure_make_install "" ffmpeg-svn lib/libavcodec.dylib "install-libs install-headers" \
                       "svn checkout svn://svn.mplayerhq.hu/ffmpeg/trunk ffmpeg-svn" \
                       "--disable-ffmpeg --disable-ffserver --disable-ffplay \
                        --disable-static --enable-shared --enable-libmp3lame \
                        --cc=\"$GCC\" \
                        \"--extra-cflags=$CPPFLAGS $COMPILE_TAG\" \"--extra-ldflags=$LDFLAGS $COMPILE_TAG\" \
                        --arch=$ARCH --disable-altivec --disable-mmx" 

echo "===== ffmpeg+lame ready $BUILD_TAG ====="

