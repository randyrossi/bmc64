#!/bin/sh

#
# ppc-build.sh - generate the powerpc BeOS version of VICE on intel BeOS
#
# Written by
#  Marco van den Heuvel <blackystardust68@yahoo.com>
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

# see if we are in the top of the tree
if [ ! -f configure.proto ]; then
  cd ../..
  if [ ! -f configure.proto ]; then
    echo "please run this script from the base of the VICE directory"
    echo "or from the appropriate build directory"
    exit 1
  fi
fi

curdir=`pwd`

CC="/bin/sh $curdir/src/arch/beos/ppc-cc.sh" CXX="/bin/sh $curdir/src/arch/beos/ppc-cc.sh" RANLIB=true STRIP=true AR="/bin/sh $curdir/src/arch/beos/ppc-ar.sh" CFLAGS="-inclpaths nosys" LDFLAGS="-L/boot/develop/lib/ppc /boot/develop/lib/ppc/glue-noinit.a /boot/develop/lib/ppc/init_term_dyn.o /boot/develop/lib/ppc/start_dyn.o -lbe -lroot" ./configure --host=ppc-beos --enable-native-tools
make
make bindist
