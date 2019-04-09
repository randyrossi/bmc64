#!/bin/sh

#
# make-bindist.sh - make binary distribution for the windows SDL port
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
# Usage: make-bindist.sh <strip> <vice-version> <--enable-arch> <zip|nozip> <x64sc-included> <top-srcdir> <cpu> <SDL-version>
#                         $1      $2             $3              $4          $5               $6           $7    $8
#

STRIP=$1
VICEVERSION=$2
ENABLEARCH=$3
ZIPKIND=$4
X64SC=$5
TOPSRCDIR=$6
CPU=$7
SDLVERSION=$8

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
  if [ ! -e src/$i.exe ]
  then
    echo Error: executable file\(s\) not found, do a \"make\" first
    exit 1
  fi
done

if test x"$CPU" = "xx86_64" -o x"$CPU" = "xamd64"; then
  WINXX="win64"
else
  WINXX="win32"
fi

if test x"$SDLVERSION" = "x2"; then
  echo Generating $WINXX SDL2 port binary distribution.
  SDLNAME="SDL2VICE"
else
  echo Generating $WINXX SDL port binary distribution.
  SDLNAME="SDLVICE"
fi
rm -f -r $SDLNAME-$VICEVERSION-$WINXX
mkdir $SDLNAME-$VICEVERSION-$WINXX
for i in $EXECUTABLES
do
  $STRIP src/$i.exe
  cp src/$i.exe $SDLNAME-$VICEVERSION-$WINXX
done
cp -a $TOPSRCDIR/data/C128 $TOPSRCDIR/data/C64 $SDLNAME-$VICEVERSION-$WINXX
cp -a $TOPSRCDIR/data/C64DTV $TOPSRCDIR/data/CBM-II $SDLNAME-$VICEVERSION-$WINXX
cp -a $TOPSRCDIR/data/DRIVES $TOPSRCDIR/data/PET $SDLNAME-$VICEVERSION-$WINXX
cp -a $TOPSRCDIR/data/PLUS4 $TOPSRCDIR/data/PRINTER $SDLNAME-$VICEVERSION-$WINXX
cp -a $TOPSRCDIR/data/SCPU64 $TOPSRCDIR/data/VIC20 $SDLNAME-$VICEVERSION-$WINXX
cp -a $TOPSRCDIR/doc/html $SDLNAME-$VICEVERSION-$WINXX
rm $SDLNAME-$VICEVERSION-$WINXX/html/checklinks.sh
cp $TOPSRCDIR/FEEDBACK $TOPSRCDIR/README $SDLNAME-$VICEVERSION-$WINXX
cp $TOPSRCDIR/COPYING $TOPSRCDIR/NEWS $SDLNAME-$VICEVERSION-$WINXX
cp $TOPSRCDIR/doc/readmes/Readme-SDL.txt $SDLNAME-$VICEVERSION-$WINXX
rm `find $SDLNAME-$VICEVERSION-$WINXX -name "Makefile*"`
rm `find $SDLNAME-$VICEVERSION-$WINXX -name "amiga_*.vkm"`
rm `find $SDLNAME-$VICEVERSION-$WINXX -name "osx*.vkm"`
rm `find $SDLNAME-$VICEVERSION-$WINXX -name "beos_*.vkm"`
rm `find $SDLNAME-$VICEVERSION-$WINXX -name "x11_*.vkm"`
rm $SDLNAME-$VICEVERSION-$WINXX/html/texi2html
mkdir $SDLNAME-$VICEVERSION-$WINXX/doc
cp $TOPSRCDIR/doc/vice.chm $SDLNAME-$VICEVERSION-$WINXX/doc
cp $TOPSRCDIR/doc/vice.hlp $SDLNAME-$VICEVERSION-$WINXX/doc
cp $TOPSRCDIR/doc/vice.pdf $SDLNAME-$VICEVERSION-$WINXX/doc
if test x"$ZIPKIND" = "xzip"; then
  if test x"$ZIP" = "x"; then
    zip -r -9 -q $SDLNAME-$VICEVERSION-$WINXX.zip $SDLNAME-$VICEVERSION-$WINXX
  else
    $ZIP $SDLNAME-$VICEVERSION-$WINXX.zip $SDLNAME-$VICEVERSION-$WINXX
  fi
  rm -f -r $SDLNAME-$VICEVERSION-$WINXX
  if test x"$SDLVERSION" = "x2"; then
    echo $WINXX SDL2 port binary distribution archive generated as $SDLNAME-$VICEVERSION-$WINXX.zip
  else
    echo $WINXX SDL port binary distribution archive generated as $SDLNAME-$VICEVERSION-$WINXX.zip
  fi
else
  if test x"$SDLVERSION" = "x2"; then
    echo $WINXX SDL2 port binary distribution directory generated as $SDLNAME-$VICEVERSION-$WINXX
  else
    echo $WINXX SDL port binary distribution directory generated as $SDLNAME-$VICEVERSION-$WINXX
  fi
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
