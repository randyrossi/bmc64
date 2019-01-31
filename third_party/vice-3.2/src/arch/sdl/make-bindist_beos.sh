#!/bin/sh

#
# make-bindist.sh - make binary distribution for the BeOS SDL port
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
# Usage: make-bindist.sh <strip> <vice-version> <cpu> <system> <--enable-arch> <zip|nozip> <xscpu64-included> <top-srcdir> <SDL-version>
#                         $1      $2             $3    $4       $5              $6          $7                 $8           $9
#

STRIP=$1
VICEVERSION=$2
CPU=$3
SYSTEM=$4
ENABLEARCH=$5
ZIPKIND=$6
XSCPU64INCLUDED=$7
TOPSRCDIR=$8
SDLVERSION=$9

EXTRAFILES="x64sc"

if test x"$XSCPU64INCLUDED" = "xyes"; then
  EXTRAFILES="$EXTRAFILES xscpu64"
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

if test x"$CPU" = "xpowerpc" -o x"$CPU" = "xppc"; then
  BEOSCPU=powerpc
else
  if test x"$CPU" = "xx86_64" -o x"$CPU" = "xamd64"; then
    BEOSCPU=x86_64
  else
    BEOSCPU=x86
  fi
fi

if test x"$SYSTEM" = "xhaiku"; then
  if test x"$SDLVERSION" = "x2"; then
    echo Generating Haiku SDL2 port binary distribution.
    SDLNAME="SDL2VICE-$BEOSCPU-haiku-$VICEVERSION"
  else
    echo Generating Haiku SDL port binary distribution.
    SDLNAME="SDLVICE-$BEOSCPU-haiku-$VICEVERSION"
  fi
else
  if test x"$SDLVERSION" = "x2"; then
    echo Generating BEOS SDL2 port binary distribution.
    SDLNAME="SDL2VICE-$BEOSCPU-beos-$VICEVERSION"
  else
    echo Generating BEOS SDL port binary distribution.
    SDLNAME="SDLVICE-$BEOSCPU-beos-$VICEVERSION"
  fi
fi

rm -f -r $SDLNAME
mkdir $SDLNAME
for i in $EXECUTABLES
do
  $STRIP src/$i
  cp src/$i $SDLNAME
done
cp -a $TOPSRCDIR/data/C128 $SDLNAME
cp -a $TOPSRCDIR/data/C64 $SDLNAME
if test x"$SCPU64INCLUDED" = "xyes"; then
  cp -a $TOPSRCDIR/data/SCPU64 $SDLNAME
fi
cp -a $TOPSRCDIR/data/C64DTV $SDLNAME
cp -a $TOPSRCDIR/data/CBM-II $SDLNAME
cp -a $TOPSRCDIR/data/DRIVES $SDLNAME
cp -a $TOPSRCDIR/data/PET $SDLNAME
cp -a $TOPSRCDIR/data/PLUS4 $SDLNAME
cp -a $TOPSRCDIR/data/PRINTER $SDLNAME
cp -a $TOPSRCDIR/data/VIC20 $SDLNAME

cp -a $TOPSRCDIR/doc/html $SDLNAME
rm $SDLNAME/html/checklinks.sh
if test x"$SDLVERSION" = "x2"; then
  cp $TOPSRCDIR/doc/readmes/Readme-SDL2.txt $SDLNAME
else
  cp $TOPSRCDIR/doc/readmes/Readme-SDL.txt $SDLNAME
fi
cp $TOPSRCDIR/FEEDBACK $TOPSRCDIR/README $SDLNAME
cp $TOPSRCDIR/COPYING $TOPSRCDIR/NEWS $SDLNAME
rm `find $SDLNAME -name "Makefile*"`
rm `find $SDLNAME -name "*.vkm" -and ! -name "sdl*.vkm"`
rm `find $SDLNAME -name "*.vsc"`
rm `find $SDLNAME -name "win_*.v*"`
rm $SDLNAME/html/texi2html

# just in case ...
rm -f -r `find $SDLNAME -name ".svn"`

mkdir $SDLNAME/doc
cp $TOPSRCDIR/doc/vice.pdf $SDLNAME/doc

if test x"$ZIPKIND" = "xzip"; then
  if test x"$ZIP" = "x"; then
    zip -r -9 -q $SDLNAME.zip $SDLNAME
  else
    $ZIP $SDLNAME.zip $SDLNAME
  fi
  rm -f -r $SDLNAME
  if test x"$SDLVERSION" = "x2"; then
    echo BEOS SDL2 port binary distribution archive generated as $SDLNAME.zip
  else
    echo BEOS SDL port binary distribution archive generated as $SDLNAME.zip
  fi
else
  if test x"$SDLVERSION" = "x2"; then
    echo BEOS SDL2 port binary distribution directory generated as $SDLNAME
  else
    echo BEOS SDL port binary distribution directory generated as $SDLNAME
  fi
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
