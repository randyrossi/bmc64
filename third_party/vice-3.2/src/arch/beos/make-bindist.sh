#!/bin/sh

#
# make-bindist.sh - make binary distribution for the BeOS ports
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
# Usage: make-bindist.sh <strip> <vice-version> <cpu> <--enable-arch> <zip|nozip> <c64sc-included> <scpu64sc-included> <top-srcdir>
#                         $1      $2             $3    $4              $5          $6               $7                  $8
#

STRIP=$1
VICEVERSION=$2
CPU=$3
ENABLEARCH=$4
ZIPKIND=$5
X64SCINCLUDED=$6
XSCPU64INCLUDED=$7
TOPSRCDIR=$8

if test x"$X64SCINCLUDED" = "xyes"; then
  EXTRAFILES="x64sc"
else
  EXTRAFILES=""
fi

if test x"$XSCPU64INCLUDED" = "xyes"; then
  EXTRAFILES="$EXTRAFILES xscpu64"
  EXTRADATADIRS="SCPU64"
else
  EXTRADATADIRS=""
fi

EMULATORS="x64 x64dtv $EXTRAFILES x128 xcbm2 xcbm5x0 xpet xplus4 xvic vsid"
CONSOLE_TOOLS="c1541 cartconv petcat"
EXECUTABLES="$EMULATORS $CONSOLE_TOOLS"

for i in $EXECUTABLES
do
  if [ ! -e src/$i ]
  then
    echo Error: executable file\(s\) not found, do a \"make\" first
    exit 1
  fi
done

COPYDATADIRS="C128 C64 C64DTV CBM-II DRIVES PET PLUS4 PRINTER $EXTRADATADIRS VIC20 fonts"

echo Generating BEOS port binary distribution.
if test x"$CPU" = "xpowerpc" -o x"$CPU" = "xppc"; then
  BEOSCPU=powerpc
else
  if test x"$CPU" = "xx86_64" -o x"$CPU" = "xamd64"; then
    BEOSCPU=Haiku64
  else
    BEOSCPU=x86
  fi
fi

rm -f -r BeVICE-$VICEVERSION.$BEOSCPU
mkdir BeVICE-$VICEVERSION.$BEOSCPU
for i in $EXECUTABLES
do
  $STRIP src/$i
  cp src/$i BeVICE-$VICEVERSION.$BEOSCPU
done

for i in $COPYDATADIRS
do
  cp -a $TOPSRCDIR/data/$i BeVICE-$VICEVERSION.$BEOSCPU
done

cp -a $TOPSRCDIR/doc/html BeVICE-$VICEVERSION.$BEOSCPU
cp $TOPSRCDIR/doc/readmes/Readme-BeOS.txt BeVICE-$VICEVERSION.$BEOSCPU
cp $TOPSRCDIR/FEEDBACK $TOPSRCDIR/README BeVICE-$VICEVERSION.$BEOSCPU
cp $TOPSRCDIR/COPYING $TOPSRCDIR/NEWS BeVICE-$VICEVERSION.$BEOSCPU
rm `find BeVICE-$VICEVERSION.$BEOSCPU -name "Makefile*"`
rm `find BeVICE-$VICEVERSION.$BEOSCPU -name "*.vkm" -and ! -name "beos*.vkm"`
rm `find BeVICE-$VICEVERSION.$BEOSCPU -name "*.vsc"`
rm `find BeVICE-$VICEVERSION.$BEOSCPU -name "win_*.v*"`
rm BeVICE-$VICEVERSION.$BEOSCPU/html/texi2html
rm BeVICE-$VICEVERSION.$BEOSCPU/html/checklinks.sh

# just in case ...
rm -f -r `find BeVICE-$VICEVERSION.$BEOSCPU -name ".svn"`

mkdir BeVICE-$VICEVERSION.$BEOSCPU/doc
cp $TOPSRCDIR/doc/vice.pdf BeVICE-$VICEVERSION.$BEOSCPU/doc

if test x"$ZIPKIND" = "xzip"; then
  if test x"$ZIP" = "x"; then
    zip -r -9 -q BeVICE-$VICEVERSION.$BEOSCPU.zip BeVICE-$VICEVERSION.$BEOSCPU
  else
    $ZIP BeVICE-$VICEVERSION.$BEOSCPU.zip BeVICE-$VICEVERSION.$BEOSCPU
  fi
  rm -f -r BeVICE-$VICEVERSION.$BEOSCPU
  echo BEOS port binary distribution archive generated as BeVICE-$VICEVERSION.$BEOSCPU.zip
else
  echo BEOS port binary distribution directory generated as BeVICE-$VICEVERSION.$BEOSCPU
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
