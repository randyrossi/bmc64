#!/bin/sh

#
# make-bindist.sh - make binary distribution for the SkyOS SDL port
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
# Usage: make-bindist.sh <strip> <vice-version> <prefix> <--enable-arch> <zip|nozip> <x64sc-included> <topsrcdir>
#                         $1      $2             $3       $4              $5          $6               $7
#

STRIP=$1
VERSION=$2
PREFIX=$3
ENABLEARCH=$4
ZIPKIND=$5
X64SC=$6
TOPSRCDIR=$7

if test x"$PREFIX" != "x/boot/programs/SDLVICE"; then
  echo Error: installation path is not /boot/programs/SDLVICE
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
  if [ ! -e /boot/programs/SDLVICE/bin/$i ]
  then
    echo Error: \"make install\" needs to be done first
    exit 1
  fi
done

echo Generating SkyOS SDL port binary distribution.
rm -f -r SDLVICE-$VERSION
mkdir -p SDLVICE-$VERSION/programs/SDLVICE
mkdir SDLVICE-$VERSION/programs/SDLVICE/bin
for i in $EXECUTABLES
do
  mv /boot/programs/SDLVICE/bin/$i SDLVICE-$VERSION/programs/SDLVICE/bin/$i.app
  $STRIP SDLVICE-$VERSION/programs/SDLVICE/bin/$i.app
done
mv /boot/programs/SDLVICE/lib/vice SDLVICE-$VERSION/programs/SDLVICE/lib
mkdir -p SDLVICE-$VERSION/programs/SDLVICE/share/man/man1
mv /boot/programs/SDLVICE/share/man/man1/* SDLVICE-$VERSION/programs/SDLVICE/share/man/man1/
mkdir -p SDLVICE-$VERSION/programs/SDLVICE/info
mv /boot/programs/SDLVICE/share/info/vice.info* SDLVICE-$VERSION/programs/SDLVICE/share/info
cp -r $TOPSRCDIR/src/arch/unix/skyos/icons SDLVICE-$VERSION/programs/SDLVICE/
rm -f -r /boot/programs/SDLVICE
if test x"$ZIPKIND" = "xzip"; then
  THEDATE=`date +%d-%b-%Y`
  cat >SDLVICE-$VERSION/install.sif <<_END
[GENERAL]
Magic=SkyOS Installation file

[DESCRIPTION]
Product=SDLVICE $VERSION
Detail=VICE, the Versatile Commodore Emulator 
Copyright=The VICE Team
Version=$VERSION
Date=$THEDATE
License=GPL
Category=Emulators

[PACKAGE]
DefaultPath=

[PANELMENU]
/menu="Emulators/SDLVICE"	/name="vsid"	/link="/boot/programs/SDLVICE/bin/vsid.app"	/icon="/boot/programs/SDLVICE/icons/vsid.ico"
/menu="Emulators/SDLVICE"	/name="x64"	/link="/boot/programs/SDLVICE/bin/x64.app"		/icon="/boot/programs/SDLVICE/icons/x64.ico"   
/menu="Emulators/SDLVICE"	/name="xscpu64"	/link="/boot/programs/SDLVICE/bin/xscpu64.app"		/icon="/boot/programs/SDLVICE/icons/xscpu64.ico"   
/menu="Emulators/SDLVICE"	/name="x64dtv"	/link="/boot/programs/SDLVICE/bin/x64dtv.app"		/icon="/boot/programs/SDLVICE/icons/x64dtv.ico"   
_end

if test x"$X64SC" = "xyes"; then
  cat >>SDLVICE-$VERSION/install.sif <<_END
/menu="Emulators/SDLVICE"	/name="x64sc"	/link="/boot/programs/SDLVICE/bin/x64sc.app"		/icon="/boot/programs/SDLVICE/icons/x64sc.ico"   
_END
fi

cat >>SDLVICE-$VERSION/install.sif <<_END
/menu="Emulators/SDLVICE"	/name="x128"	/link="/boot/programs/SDLVICE/bin/x128.app"	/icon="/boot/programs/SDLVICE/icons/x128.ico"   
/menu="Emulators/SDLVICE"	/name="xcbm2"	/link="/boot/programs/SDLVICE/bin/xcbm2.app"	/icon="/boot/programs/SDLVICE/icons/xcbm2.ico"   
/menu="Emulators/SDLVICE"	/name="xcbm5x0"	/link="/boot/programs/SDLVICE/bin/xcbm5x0.app"	/icon="/boot/programs/SDLVICE/icons/xcbm5x0.ico"
/menu="Emulators/SDLVICE"	/name="xpet"	/link="/boot/programs/SDLVICE/bin/xpet.app"	/icon="/boot/programs/SDLVICE/icons/xpet.ico"   
/menu="Emulators/SDLVICE"	/name="xplus4"	/link="/boot/programs/SDLVICE/bin/xplus4.app"	/icon="/boot/programs/SDLVICE/icons/xplus4.ico"   
/menu="Emulators/SDLVICE"	/name="xvic"	/link="/boot/programs/SDLVICE/bin/xvic.app"	/icon="/boot/programs/SDLVICE/icons/xvic.ico"   

[FILEICONS]
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/vsid.app"	/icon="/boot/programs/SDLVICE/icons/vsid.ico"
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/x64.app"	/icon="/boot/programs/SDLVICE/icons/x64.ico"
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/xscpu64.app"	/icon="/boot/programs/SDLVICE/icons/xscpu64.ico"
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/x64dtv.app"	/icon="/boot/programs/SDLVICE/icons/x64dtv.ico"
_END

if test x"$X64SC" = "xyes"; then
  cat >>SDLVICE-$VERSION/install.sif <<_END
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/x64sc.app"	/icon="/boot/programs/SDLVICE/icons/x64sc.ico"
_END
fi

cat >>SDLVICE-$VERSION/install.sif <<_END
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/x128.app"	/icon="/boot/programs/SDLVICE/icons/x128.ico"
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/xcbm2.app"	/icon="/boot/programs/SDLVICE/icons/xcbm2.ico"
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/xcbm5x0.app"	/icon="/boot/programs/SDLVICE/icons/xcbm5x0.ico"
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/xpet.app"	/icon="/boot/programs/SDLVICE/icons/xpet.ico"
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/xplus4.app"	/icon="/boot/programs/SDLVICE/icons/xplus4.ico"
/file="\$INSTALL_ROOT/programs/SDLVICE/bin/xvic.app"	/icon="/boot/programs/SDLVICE/icons/xvic.ico"

[SS_DESCRIPTION]
VICE is a program that runs on a Unix, MS-DOS, Win32, OS/2, BeOS, QNX, 
SkyOS, Amiga or Mac OS X machine and executes programs intended for the 
old 8-bit computers. The current version emulates the C64, the C128, the 
VIC20, practically all PET models, the PLUS4 and the CBM-II (aka C610).
[/SS_DESCRIPTION]

[END]
_END

  cd SDLVICE-$VERSION
  tar cf ../SDLVICE-$VERSION.tar *
  cd ..
  gzip SDLVICE-$VERSION.tar
  mv SDLVICE-$VERSION.tar.gz SDLVICE-$VERSION.pkg
  rm -f -r SDLVICE-$VERSION
  echo SkyOS SDL port binary package generated as SDLVICE-$VERSION.pkg
else
  echo SkyOS SDL port binary distribution directory generated as SDLVICE-$VERSION
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
