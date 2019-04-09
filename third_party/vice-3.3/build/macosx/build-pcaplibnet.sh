#!/bin/bash

#
# build-pcaplibnet.sh - build the pcaplibnet on macs for VICE ethernet (TFE/RRNET) support
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
# Usage:     build-pcaplibnet.sh <build-dir> <arch:ppc|i386>
#

# get build tools
SCRIPT_DIR="`dirname \"$0\"`"
. "$SCRIPT_DIR/build-inc.sh"

# parse args
parse_args "$@"

# setup compiler environment
set_compiler_env

echo "===== pcaplibnet build $BUILD_TAG ====="

COMPILE_IN_SOURCE=1
configure_make_install libpcap-1.0.0.tar.gz libpcap-1.0.0 lib/libpcap.a install \
                       "http://www.tcpdump.org/"

PATCH="mv src/libnet_init.c src/libnet_init.c.org ; \
sed -e 's/getuid() && geteuid()/0/' < src/libnet_init.c.org > src/libnet_init.c ; \
mv src/libnet_build_gre.c src/libnet_build_gre.c.org ; \
sed -e 's/^inline//g' < src/libnet_build_gre.c.org > src/libnet_build_gre.c"
configure_make_install libnet-1.1.2.1.tar.gz libnet lib/libnet.a install \
                       "http://ftp.debian.org/debian/pool/main/libn/libnet0/libnet0_1.0.2a.orig.tar.gz"

echo "===== pcaplibnet $BUILD_TAG ====="
echo "(Read 'libpcap-1.0.0/README.macosx' to find out how to install pcap on Mac OS X correctly)"
