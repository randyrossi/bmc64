#!/bin/sh

#
# make-bindist.sh - make binary distribution for the AmigaOS ports
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
# Usage: make-bindist.sh <strip> <vice-version> <host-cpu> <host-system> <enable_arch> <zip|nozip> <x64sc-included> <top-srcdir> <exe-ext>
#                         $1      $2             $3         $4            $5            $6          $7               $8           $9
#

STRIP=$1
VICEVERSION=$2
HOSTCPU=$3
HOSTSYSTEM=$4
ENABLEARCH=$5
ZIPKIND=$6
X64SC=$7
TOPSCRDIR=$8
EXEEXT=$9

if test x"$X64SC" = "xyes"; then
  SCFILE="x64sc"
else
  SCFILE=""
fi

EMULATORS="x64 x64dtv $SCFILE xscpu64 x128 xcbm2 xcbm5x0 xpet xplus4 xvic"
CONSOLE_TOOLS="c1541 cartconv petcat"
EXECUTABLES="$EMULATORS $CONSOLE_TOOLS vsid"

for i in $EXECUTABLES
do
  if [ ! -e src/$i$EXEEXT ]
  then
    echo Error: executable file\(s\) not found, do a \"make all\" first
    exit 1
  fi
done

if test x"$HOSTSYSTEM" = "xaros"; then
  AMIGAFLAVOR=$HOSTCPU-AROS-$VICEVERSION
  echo Generating AROS port binary distribution.
else
  if test x"$HOSTSYSTEM" = "xmorphos"; then
    AMIGAFLAVOR=MorphOS-$VICEVERSION
    echo Generating MorphOS port binary distribution.
  else
    if test x"$HOSTSYSTEM" = "xwarpos"; then
      AMIGAFLAVOR=WarpOS-$VICEVERSION
      echo Generating WarpOS port binary distribution.
    else
      if test x"$HOSTSYSTEM" = "xpowerup"; then
        AMIGAFLAVOR=PowerUP-$VICEVERSION
        echo Generating PowerUP port binary distribution.
      else
        if test x"$HOSTCPU" = "xm68k"; then
          AMIGAFLAVOR=AmigaOS3-$VICEVERSION
          echo Generating AmigaOS3 port binary distribution.
        else
          AMIGAFLAVOR=AmigaOS4-$VICEVERSION
          echo Generating AmigaOS4 port binary distribution.
        fi
      fi
    fi
  fi
fi

rm -f -r VICE-$AMIGAFLAVOR VICE-$AMIGAFLAVOR.info
mkdir VICE-$AMIGAFLAVOR
if test x"$HOSTSYSTEM" != "xaros"; then
  for i in $EXECUTABLES
  do
    $STRIP src/$i$EXEEXT
  done
  for i in $EMULATORS
  do
    $STRIP src/arch/amigaos/$i"ns"$EXEEXT
  done
else
  for i in $EXECUTABLES
  do
    $STRIP --strip-unneeded --remove-section .comment src/$i$EXEEXT
  done
  for i in $EMULATORS
  do
    $STRIP --strip-unneeded --remove-section .comment src/arch/amigaos/$i"ns"$EXEEXT
  done
fi
if test x"$HOSTSYSTEM" = "xmorphos"; then
  for i in $EXECUTABLES
  do
    cp src/$i$EXEEXT VICE-$AMIGAFLAVOR/$i
  done
  for i in $EMULATORS
  do
    cp src/arch/amigaos/$i"ns"$EXEEXT VICE-$AMIGAFLAVOR/$i\ \(no\ sound\)
  done
else
  for i in $EXECUTABLES
  do
    cp src/$i$EXEEXT VICE-$AMIGAFLAVOR/$i.exe
  done
  for i in $EMULATORS
  do
    cp src/arch/amigaos/$i"ns"$EXEEXT VICE-$AMIGAFLAVOR/$i\ \(no\ sound\).exe
  done
fi
cp -a $TOPSCRDIR/data/C128 $TOPSCRDIR/data/C64 $TOPSCRDIR/data/C64DTV $TOPSCRDIR/data/CBM-II $TOPSCRDIR/data/DRIVES VICE-$AMIGAFLAVOR
cp -a $TOPSCRDIR/data/PET $TOPSCRDIR/data/PLUS4 $TOPSCRDIR/data/PRINTER $TOPSCRDIR/data/VIC20 VICE-$AMIGAFLAVOR
cp -a $TOPSCRDIR/data/SCPU64 $TOPSCRDIR/data/fonts VICE-$AMIGAFLAVOR
cp -a $TOPSCRDIR/doc/html VICE-$AMIGAFLAVOR
rm VICE-$AMIGAFLAVOR/html/checklinks.sh
cp $TOPSCRDIR/FEEDBACK $TOPSCRDIR/README VICE-$AMIGAFLAVOR
cp $TOPSCRDIR/COPYING $TOPSCRDIR/NEWS VICE-$AMIGAFLAVOR
cp $TOPSCRDIR/doc/readmes/Readme-Amiga.txt VICE-$AMIGAFLAVOR
mkdir VICE-$AMIGAFLAVOR/doc
cp $TOPSCRDIR/doc/vice.guide VICE-$AMIGAFLAVOR/doc
cp $TOPSCRDIR/doc/vice.pdf VICE-$AMIGAFLAVOR/doc
if test x"$HOSTSYSTEM" = "xwarpos"; then
  for i in $EXECUTABLES
  do
    elf2exe VICE-$AMIGAFLAVOR/$i.exe VICE-$AMIGAFLAVOR/$i.new
    mv -f VICE-$AMIGAFLAVOR/$i.new VICE-$AMIGAFLAVOR/$i.exe
  done
fi
if test x"$HOSTSYSTEM" = "xmorphos"; then
  cp $TOPSCRDIR/src/arch/amigaos/info-files/morphos/VICE.info VICE-$AMIGAFLAVOR.info
  cp $TOPSCRDIR/src/arch/amigaos/info-files/morphos/x*.info VICE-$AMIGAFLAVOR
  if test x"$X64SC" != "xyes"; then
    rm -f VICE-$AMIGAFLAVOR/x64sc.info
  else
    cp VICE-$AMIGAFLAVOR/x64sc.info VICE-$AMIGAFLAVOR/x64sc\ \(no\ sound\).info
  fi
  cp VICE-$AMIGAFLAVOR/xscpu64.info VICE-$AMIGAFLAVOR/xscpu64\ \(no\ sound\).info
  cp VICE-$AMIGAFLAVOR/x128.info VICE-$AMIGAFLAVOR/x128\ \(no\ sound\).info
  cp VICE-$AMIGAFLAVOR/xvic.info VICE-$AMIGAFLAVOR/xvic\ \(no\ sound\).info
  cp VICE-$AMIGAFLAVOR/xpet.info VICE-$AMIGAFLAVOR/xpet\ \(no\ sound\).info
  cp VICE-$AMIGAFLAVOR/xplus4.info VICE-$AMIGAFLAVOR/xplus4\ \(no\ sound\).info
  cp VICE-$AMIGAFLAVOR/xcbm2.info VICE-$AMIGAFLAVOR/xcbm2\ \(no\ sound\).info
  cp VICE-$AMIGAFLAVOR/xcbm5x0.info VICE-$AMIGAFLAVOR/xcbm5x0\ \(no\ sound\).info
else
  cp $TOPSCRDIR/src/arch/amigaos/info-files/VICE.info VICE-$AMIGAFLAVOR.info
  cp $TOPSCRDIR/src/arch/amigaos/info-files/*.exe.info VICE-$AMIGAFLAVOR
  cp VICE-$AMIGAFLAVOR/x64.exe.info VICE-$AMIGAFLAVOR/x64\ \(no\ sound\).exe.info
  if test x"$X64SC" != "xyes"; then
    rm -f VICE-$AMIGAFLAVOR/x64sc.exe.info
  else
    cp VICE-$AMIGAFLAVOR/x64sc.exe.info VICE-$AMIGAFLAVOR/x64sc\ \(no\ sound\).exe.info
  fi
  cp VICE-$AMIGAFLAVOR/xscpu64.exe.info VICE-$AMIGAFLAVOR/xscpu64\ \(no\ sound\).exe.info
  cp VICE-$AMIGAFLAVOR/x128.exe.info VICE-$AMIGAFLAVOR/x128\ \(no\ sound\).exe.info
  cp VICE-$AMIGAFLAVOR/xvic.exe.info VICE-$AMIGAFLAVOR/xvic\ \(no\ sound\).exe.info
  cp VICE-$AMIGAFLAVOR/xpet.exe.info VICE-$AMIGAFLAVOR/xpet\ \(no\ sound\).exe.info
  cp VICE-$AMIGAFLAVOR/xplus4.exe.info VICE-$AMIGAFLAVOR/xplus4\ \(no\ sound\).exe.info
  cp VICE-$AMIGAFLAVOR/xcbm2.exe.info VICE-$AMIGAFLAVOR/xcbm2\ \(no\ sound\).exe.info
  cp VICE-$AMIGAFLAVOR/xcbm5x0.exe.info VICE-$AMIGAFLAVOR/xcbm5x0\ \(no\ sound\).exe.info
fi
rm `find VICE-$AMIGAFLAVOR -name "Makefile*"`
rm `find VICE-$AMIGAFLAVOR -name "dos_*.vkm"`
rm `find VICE-$AMIGAFLAVOR -name "osx*.vkm"`
rm `find VICE-$AMIGAFLAVOR -name "beos_*.vkm"`
rm `find VICE-$AMIGAFLAVOR -name "win_*.v*"`
rm `find VICE-$AMIGAFLAVOR -name "x11_*.vkm"`
rm `find VICE-$AMIGAFLAVOR -name "sdl*.vkm"`
rm `find VICE-$AMIGAFLAVOR -name "*.vsc"`
rm VICE-$AMIGAFLAVOR/html/texi2html
if test x"$ZIPKIND" = "xzip"; then
  tar cf VICE-$AMIGAFLAVOR.tar VICE-$AMIGAFLAVOR VICE-$AMIGAFLAVOR.info
  gzip VICE-$AMIGAFLAVOR.tar
  rm -f -r VICE-$AMIGAFLAVOR VICE-$AMIGAFLAVOR.info
  echo AMIGA port binary distribution archive generated as VICE-$AMIGAFLAVOR.tar.gz
else
  echo AMIGA port binary distribution directory generated as VICE-$AMIGAFLAVOR
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
