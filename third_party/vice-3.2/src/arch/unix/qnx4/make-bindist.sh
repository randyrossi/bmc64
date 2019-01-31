#!/bin/sh

#
# make-bindist.sh - make binary distribution for the QNX 4.x port
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
# Usage: make-bindist.sh <vice-version> <prefix> <--enable-arch> <zip|nozip> <x64sc-include> <topsrcdir>
#                         $1             $2       $3              $4          $5              $6
#

VICEVERSION=$1
PREFIX=$2
ENABLEARCH=$3
ZIPKIND=$4
X64SC=$5
TOPSRCDIR=$6

if test x"$PREFIX" != "x/usr/local"; then
  echo Error: installation path is not /usr/local
  exit 1
fi

if test x"$X64SC" = "xyes"; then
  SCFILE="x64sc"
else
  SCFILE=""
fi

EMULATORS="x64 xscpu64 x64dtv $SCFILE x128 xcbm2 xcbm5x0 xpet xplus4 xvic vsid"
CONSOLE_TOOLS="c1541 cartconv petcat"
EXECUTABLES="$EMULATORS $CONSOLE_TOOLS"

for i in $EXECUTABLES
do
  if [ ! -e /usr/local/bin/$i ]
  then
    echo Error: \"make install\" needs to be done first
    exit 1
  fi
done

if test x"$ZIPKIND" = "xnozip"; then
  echo Error: This port only supports \"make bindistzip\"
  exit 1
fi

echo Generating QNX 4 port binary distribution.
for i in $EXECUTABLES
do
  wstrip /usr/local/bin/$i
done
cp $TOPSRCDIR/src/arch/unix/qnx4/install_msg ./
TARFILES=""
for i in $EXECUTABLES
do
TARFILES="$TARFILES /usr/local/bin/$i"
done
tar cf vice-$VICEVERSION-qnx4.tar install_msg /usr/local/bin/vsid $TARFILES /usr/local/lib/vice /usr/local/man/man1/c1541.1 /usr/local/man/man1/petcat.1 /usr/local/man/man1/vice.1 /usr/local/info/vice.info*
freeze vice-$VICEVERSION-qnx4.tar
rm -f install_msg
echo QNX 4 port binary distribution directory generated as vice-$VICEVERSION-qnx4.tar.F
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
