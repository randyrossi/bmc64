#!/bin/sh

#
# make-bindist.sh - make binary distribution for the AmigaOS SDL port
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
# Usage: make-bindist.sh <strip> <vice-version> <host-cpu> <host-system> <--enable-arch> <zip|nozip> <x64sc-included> <top-srcdir> <SDL-version> <exe-ext>
#                         $1      $2             $3         $4            $5              $6          $7               $8           $9           $10
#

STRIP=$1
VICEVERSION=$2
HOSTCPU=$3
HOSTSYSTEM=$4
ENABLEARCH=$5
ZIPKIND=$6
X64SC=$7
TOPSCRDIR=$8
SDLVERSION=$9

shift

EXEEXT=$9

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
  if [ ! -e src/$i$EXEEXT ]
  then
    echo Error: executable file\(s\) not found, do a \"make all\" first
    exit 1
  fi
done

if test x"$SDLVERSION" = "x2"; then
   SDLV="SDL2.x"
   SDLVNAME="SDL2VICE"
else
   SDLV="SDL1.x"
   SDLVNAME="SDLVICE"
fi

if test x"$HOSTSYSTEM" = "xaros"; then
  AMIGAFLAVOR=$HOSTCPU-AROS-$VICEVERSION
  echo Generating $HOSTCPU AROS $SDLV port binary distribution.
else
  if test x"$HOSTSYSTEM" = "xmorphos"; then
    AMIGAFLAVOR=MorphOS-$VICEVERSION
    echo Generating MorphOS $SDLV port binary distribution.
  else
    if test x"$HOSTSYSTEM" = "xwarpos"; then
      AMIGAFLAVOR=WarpOS-$VICEVERSION
      echo Generating WarpOS $SDLV port binary distribution.
    else
      if test x"$HOSTSYSTEM" = "xpowerup"; then
        AMIGAFLAVOR=PowerUP-$VICEVERSION
        echo Generating PowerUP $SDLV port binary distribution.
      else
        if test x"$HOSTCPU" = "xm68k"; then
          AMIGAFLAVOR=AmigaOS3-$VICEVERSION
          echo Generating AmigaOS3 $SDLV port binary distribution.
        else
          AMIGAFLAVOR=AmigaOS4-$VICEVERSION
          echo Generating AmigaOS4 $SDLV port binary distribution.
        fi
      fi
    fi
  fi
fi

rm -f -r $SDLVNAME-$AMIGAFLAVOR $SDLVNAME-$AMIGAFLAVOR.info
mkdir $SDLVNAME-$AMIGAFLAVOR
if test x"$HOSTSYSTEM" != "xaros"; then
  for i in $EXECUTABLES
  do
    $STRIP src/$i$EXEEXT
  done
else
  for i in $EXECUTABLES
  do
    $STRIP --strip-unneeded --remove-section .comment src/$i$EXEEXT
  done
fi
if test x"$HOSTSYSTEM" = "xmorphos"; then
  for i in $EXECUTABLES
  do
    cp src/$i$EXEEXT $SDLVNAME-$AMIGAFLAVOR/$i
  done
else
  for i in $EXECUTABLES
  do
    cp src/$i$EXEEXT $SDLVNAME-$AMIGAFLAVOR/$i.exe
  done
fi
cp -a $TOPSCRDIR/data/C128 $TOPSCRDIR/data/C64 $SDLVNAME-$AMIGAFLAVOR
cp -a $TOPSCRDIR/data/C64DTV $TOPSCRDIR/data/CBM-II $SDLVNAME-$AMIGAFLAVOR
cp -a $TOPSCRDIR/data/DRIVES $TOPSCRDIR/data/PET $SDLVNAME-$AMIGAFLAVOR
cp -a $TOPSCRDIR/data/PLUS4 $TOPSCRDIR/data/PRINTER $SDLVNAME-$AMIGAFLAVOR
cp -a $TOPSCRDIR/data/SCPU64 $SDLVNAME-$AMIGAFLAVOR
cp -a $TOPSCRDIR/doc/html $SDLVNAME-$AMIGAFLAVOR
rm $SDLVNAME-$AMIGAFLAVOR/html/checklinks.sh
cp $TOPSCRDIR/FEEDBACK $TOPSCRDIR/README $SDLVNAME-$AMIGAFLAVOR
cp $TOPSCRDIR/doc/readmes/Readme-SDL.txt $SDLVNAME-$AMIGAFLAVOR
cp $TOPSCRDIR/COPYING $TOPSCRDIR/NEWS $SDLVNAME-$AMIGAFLAVOR
if test x"$HOSTSYSTEM" = "xwarpos"; then
  for i in $EXECUTABLES
  do
    elf2exe $SDLVNAME-$AMIGAFLAVOR/$i.exe $SDLVNAME-$AMIGAFLAVOR/$i.new
    mv -f $SDLVNAME-$AMIGAFLAVOR/$i.new $SDLVNAME-$AMIGAFLAVOR/$i.exe
  done
fi
if test x"$HOSTSYSTEM" = "xmorphos"; then
  cp $TOPSCRDIR/src/arch/amigaos/info-files/morphos/VICE.info $SDLVNAME-$AMIGAFLAVOR.info
  cp $TOPSCRDIR/src/arch/amigaos/info-files/morphos/x*.info $SDLVNAME-$AMIGAFLAVOR
  if test x"$X64SC" != "xyes"; then
    rm -f $SDLVNAME-$AMIGAFLAVOR/x64sc.info
  fi
else
  cp $TOPSCRDIR/src/arch/amigaos/info-files/VICE.info $SDLVNAME-$AMIGAFLAVOR.info
  cp $TOPSCRDIR/src/arch/amigaos/info-files/*.exe.info $SDLVNAME-$AMIGAFLAVOR
  if test x"$X64SC" != "xyes"; then
    rm -f $SDLVNAME-$AMIGAFLAVOR/x64sc.exe.info
  fi
fi
rm `find $SDLVNAME-$AMIGAFLAVOR -name "Makefile*"`
rm `find $SDLVNAME-$AMIGAFLAVOR -name "osx*.vkm"`
rm `find $SDLVNAME-$AMIGAFLAVOR -name "beos_*.vkm"`
rm `find $SDLVNAME-$AMIGAFLAVOR -name "x11_*.vkm"`
rm `find $SDLVNAME-$AMIGAFLAVOR -name "gtk3*.vkm"`
rm $SDLVNAME-$AMIGAFLAVOR/html/texi2html
mkdir $SDLVNAME-$AMIGAFLAVOR/doc
cp $TOPSCRDIR/doc/vice.guide $SDLVNAME-$AMIGAFLAVOR/doc
cp $TOPSCRDIR/doc/vice.pdf $SDLVNAME-$AMIGAFLAVOR/doc
if test x"$ZIPKIND" = "xzip"; then
  tar cf $SDLVNAME-$AMIGAFLAVOR.tar $SDLVNAME-$AMIGAFLAVOR $SDLVNAME-$AMIGAFLAVOR.info
  gzip $SDLVNAME-$AMIGAFLAVOR.tar
  rm -f -r $SDLVNAME-$AMIGAFLAVOR $SDLVNAME-$AMIGAFLAVOR.info
  echo AMIGA $SDLV port binary distribution archive generated as $SDLVNAME-$AMIGAFLAVOR.tar.gz
else
  echo AMIGA $SDLV port binary destribution directory generated as $SDLVNAME-$AMIGAFLAVOR
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
