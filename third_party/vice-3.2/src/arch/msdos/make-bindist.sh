#!/bin/sh

#
# make-bindist.sh - make binary distribution for the DOS port
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
# Usage: make-bindist.sh <strip> <vice-ver-major> <vice-ver-minor> <--enable-arch> <zip|nozip> <x64sc-included> <top-srcdir>
#                         $1      $2               $3               $4              $5          $6               $7
#

STRIP=$1
VICEVERMAJOR=$2
VICEVERMINOR=$3
ENABLEARCH=$4
ZIPKIND=$5
X64SC=$6
TOPSRCDIR=$7

if test x"$X64SC" = "xyes"; then
  SCFILE="x64sc"
else
  SCFILE=""
fi

EMULATORS="x64 x64dtv xscpu64 $SCFILE x128 xcbm2 xcbm5x0 xpet xplus4 xvic vsid"
CONSOLE_TOOLS="c1541 cartconv petcat"
EXECUTABLES="$EMULATORS $CONSOLE_TOOLS"

for i in $EXECUTABLES
do
  if [ ! -e src/$i.exe ]
  then
    echo Error: executable file\(s\) not found, do a \"make\" first
    exit 1
  fi
done

echo Generating MSDOS port binary distribution.
rm -f -r dosvice
mkdir dosvice
for i in $EXECUTABLES
do
  $STRIP src/$i.exe
  cp src/$i.exe dosvice
done
cp -a $TOPSRCDIR/data/C128 $TOPSRCDIR/data/C64 $TOPSRCDIR/data/C64DTV $TOPSRCDIR/data/CBM-II $TOPSRCDIR/data/DRIVES dosvice
cp -a $TOPSRCDIR/data/PET $TOPSRCDIR/data/PLUS4 $TOPSRCDIR/data/PRINTER $TOPSRCDIR/data/VIC20 $TOPSRCDIR/data/SCPU64 dosvice
cp -a $TOPSRCDIR/data/fonts dosvice
cp -a $TOPSRCDIR/doc/html dosvice
cp $TOPSRCDIR/FEEDBACK $TOPSRCDIR/README dosvice
cp $TOPSRCDIR/COPYING $TOPSRCDIR/NEWS dosvice
cp $TOPSRCDIR/doc/readmes/Readme-DOS.txt dosvice/Readme.dos
rm `find dosvice -name "Makefile*"`
rm `find dosvice -name "amiga_*.vkm"`
rm `find dosvice -name "osx*.vkm"`
rm `find dosvice -name "beos_*.vkm"`
rm `find dosvice -name "win_*.v*"`
rm `find dosvice -name "x11_*.vkm"`
rm `find dosvice -name "sdl*.vkm"`
rm `find dosvice -name "*.vsc"`
rm dosvice/html/texi2html
rm dosvice/html/checklinks.sh
mkdir dosvice/doc
cp $TOPSRCDIR/doc/vice.txt dosvice/doc
if test x"$ZIPKIND" = "xzip"; then
  if test x"$ZIP" = "x"; then
    zip -r -9 -q vice$VICEVERMAJOR$VICEVERMINOR.zip dosvice
  else
    $ZIP vice$VICEVERMAJOR$VICEVERMINOR.zip dosvice
  fi
  rm -f -r dosvice
  echo MSDOS port binary distribution archive generated as vice$VICEVERMAJOR$VICEVERMINOR.zip
else
  echo MSDOS port binary distribution directory generated as dosvice
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
