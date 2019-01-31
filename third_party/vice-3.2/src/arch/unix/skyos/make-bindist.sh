#!/bin/sh

#
# make-bindist.sh - make binary distribution for the SkyOS port
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

if test x"$PREFIX" != "x/boot/programs/VICE"; then
  echo Error: installation path is not /boot/programs/VICE
  exit 1
fi

if test x"$X64SC" = "xyes"; then
  SCFILE="x64sc"
else
  SCFILE=""
fi

EMULATORS="x64 x64dtv xscpu64 $SCFILE x128 xcbm2 xcbm5x0 xpet xplus4 xvic vsid"
CONSOLE_TOOLS="c1541 cartconv petcat"
EXECUTABLES="$EMULATORS $CONSOLE_TOOLS"
LANGUAGES="da de fr hu it nl pl sv tr"

for i in $EXECUTABLES
do
  if [ ! -e /boot/programs/VICE/bin/$i ]
  then
    echo Error: \"make install\" needs to be done first
    exit 1
  fi
done

echo Generating SkyOS port binary distribution.
rm -f -r VICE-$VERSION
mkdir -p VICE-$VERSION/programs/VICE
for i in $LANGUAGES
do
  mkdir -p VICE-$VERSION/programs/VICE/lib/locale/$i/LC_MESSAGES
  mv /boot/programs/VICE/lib/locale/$i/LC_MESSAGES/vice.* VICE-$VERSION/programs/VICE/lib/locale/$i/LC_MESSAGES
done
mkdir VICE-$VERSION/programs/VICE/bin
mv /boot/programs/VICE/bin/vsid VICE-$VERSION/programs/VICE/bin
for i in $EXECUTABLES
do
  mv /boot/programs/VICE/bin/$i VICE-$VERSION/programs/VICE/bin/$i.app
  $STRIP VICE-$VERSION/programs/VICE/bin/$i.app
done
mv /boot/programs/VICE/lib/vice VICE-$VERSION/programs/VICE/lib
rm `find VICE-$VERSION -name "sdl*.vkm"`
mkdir -p VICE-$VERSION/programs/VICE/share/man/man1
mv /boot/programs/VICE/share/man/man1/* VICE-$VERSION/programs/VICE/share/man/man1/
mkdir -p VICE-$VERSION/programs/VICE/info
mv /boot/programs/VICE/share/info/vice.info* VICE-$VERSION/programs/VICE/share/info
cp -r $TOPSRCDIR/src/arch/unix/skyos/icons VICE-$VERSION/programs/VICE/
rm -f -r /boot/programs/VICE
if test x"$ZIPKIND" = "xzip"; then
  THEDATE=`date +%d-%b-%Y`
  cat >VICE-$VERSION/install.sif <<_END
[GENERAL]
Magic=SkyOS Installation file

[DESCRIPTION]
Product=VICE $VERSION
Detail=VICE, the Versatile Commodore Emulator 
Copyright=The VICE Team
Version=$VERSION
Date=$THEDATE
License=GPL
Category=Emulators

[PACKAGE]
DefaultPath=

[PANELMENU]
/menu="Emulators/VICE"	/name="vsid"	/link="/boot/programs/VICE/bin/vsid.app"	/icon="/boot/programs/VICE/icons/vsid.ico"
/menu="Emulators/VICE"	/name="x64"	/link="/boot/programs/VICE/bin/x64.app"		/icon="/boot/programs/VICE/icons/x64.ico"   
/menu="Emulators/VICE"	/name="xscpu64"	/link="/boot/programs/VICE/bin/xscpu64.app"		/icon="/boot/programs/VICE/icons/xscpu64.ico"   
/menu="Emulators/VICE"	/name="x64dtv"	/link="/boot/programs/VICE/bin/x64dtv.app"		/icon="/boot/programs/VICE/icons/x64dtv.ico"   
_END

if test x"$X64SC" = "xyes"; then
  cat >>VICE-$VERSION/install.sif <<_END
/menu="Emulators/VICE"	/name="x64sc"	/link="/boot/programs/VICE/bin/x64sc.app"		/icon="/boot/programs/VICE/icons/x64sc.ico"   
_END
fi

cat >>VICE-$VERSION/install.sif <<_END
/menu="Emulators/VICE"	/name="x128"	/link="/boot/programs/VICE/bin/x128.app"	/icon="/boot/programs/VICE/icons/x128.ico"   
/menu="Emulators/VICE"	/name="xcbm2"	/link="/boot/programs/VICE/bin/xcbm2.app"	/icon="/boot/programs/VICE/icons/xcbm2.ico"   
/menu="Emulators/VICE"	/name="xcbm5x0"	/link="/boot/programs/VICE/bin/xcbm5x0.app"	/icon="/boot/programs/VICE/icons/xcbm5x0.ico"
/menu="Emulators/VICE"	/name="xpet"	/link="/boot/programs/VICE/bin/xpet.app"	/icon="/boot/programs/VICE/icons/xpet.ico"   
/menu="Emulators/VICE"	/name="xplus4"	/link="/boot/programs/VICE/bin/xplus4.app"	/icon="/boot/programs/VICE/icons/xplus4.ico"   
/menu="Emulators/VICE"	/name="xvic"	/link="/boot/programs/VICE/bin/xvic.app"	/icon="/boot/programs/VICE/icons/xvic.ico"   

[FILEICONS]
/file="\$INSTALL_ROOT/programs/VICE/bin/vsid.app"	/icon="/boot/programs/VICE/icons/vsid.ico"
/file="\$INSTALL_ROOT/programs/VICE/bin/x64.app"	/icon="/boot/programs/VICE/icons/x64.ico"
/file="\$INSTALL_ROOT/programs/VICE/bin/xscpu64.app"	/icon="/boot/programs/VICE/icons/xscpu64.ico"
/file="\$INSTALL_ROOT/programs/VICE/bin/x64dtv.app"	/icon="/boot/programs/VICE/icons/x64dtv.ico"
_END

if test x"$X64SC" = "xyes"; then
  cat >>VICE-$VERSION/install.sif <<_END
/file="\$INSTALL_ROOT/programs/VICE/bin/x64sc.app"	/icon="/boot/programs/VICE/icons/x64sc.ico"
_END
fi

cat >>VICE-$VERSION/install.sif <<_END
/file="\$INSTALL_ROOT/programs/VICE/bin/x128.app"	/icon="/boot/programs/VICE/icons/x128.ico"
/file="\$INSTALL_ROOT/programs/VICE/bin/xcbm2.app"	/icon="/boot/programs/VICE/icons/xcbm2.ico"
/file="\$INSTALL_ROOT/programs/VICE/bin/xcbm5x0.app"	/icon="/boot/programs/VICE/icons/xcbm5x0.ico"
/file="\$INSTALL_ROOT/programs/VICE/bin/xpet.app"	/icon="/boot/programs/VICE/icons/xpet.ico"
/file="\$INSTALL_ROOT/programs/VICE/bin/xplus4.app"	/icon="/boot/programs/VICE/icons/xplus4.ico"
/file="\$INSTALL_ROOT/programs/VICE/bin/xvic.app"	/icon="/boot/programs/VICE/icons/xvic.ico"

[SS_DESCRIPTION]
VICE is a program that runs on a Unix, MS-DOS, Win32, OS/2, BeOS, QNX, SkyOS, Amiga or Mac OS X machine and executes programs intended for the old 8-bit computers. The current version emulates the C64, the C64DTV, the C128, the VIC20, practically all PET models, the PLUS4 and the CBM-II (aka C610).
[/SS_DESCRIPTION]
[END]
_END

  cd VICE-$VERSION
  tar cf ../VICE-$VERSION.tar *
  cd ..
  gzip VICE-$VERSION.tar
  mv VICE-$VERSION.tar.gz VICE-$VERSION.pkg
  rm -f -r VICE-$VERSION
  echo SkyOS port binary package generated as VICE-$VERSION.pkg
else
  echo SkyOS port binary distribution directory generated as VICE-$VERSION
fi
if test x"$ENABLEARCH" = "xyes"; then
  echo Warning: binaries are optimized for your system and might not run on a different system, use --enable-arch=no to avoid this
fi
