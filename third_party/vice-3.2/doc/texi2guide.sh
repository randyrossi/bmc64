#!/bin/sh

#
# texi2guide.sh - This script generates an amigaguide from vice.texi
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
# Usage: texi2guide.sh <makeguide> <guide-file> <texi-file> <host-cpu> <host-os>
#                       $1          $2           $3          $4         $5
#

MAKEGUIDE=$1
GUIDEFILE=$2
TEXIFILE=$3
HOSTCPU=$4
HOSTOS=$5

AMIGAFLAVOR=""

if test x"$HOSTOS" = "xaros"; then
  AMIGAFLAVOR=-Dplatformaros
fi

if test x"$HOSTOS" = "xmorphos"; then
  AMIGAFLAVOR=-Dplatformmorphos
fi

if test x"$HOSTOS" = "xwarpos"; then
  AMIGAFLAVOR=-Dplatformamigaos3
fi

if test x"$HOSTOS" = "xpowerup"; then
  AMIGAFLAVOR=-Dplatformamigaos3
fi

if test x"$HOSTOS" = "xamigaos" -a x"$HOSTCPU" = "xm68k"; then
  AMIGAFLAVOR=-Dplatformamigaos3
fi

if test x"$HOSTOS" = "xamigaos" -a x"$AMIGAFLAVOR" != "x-Dplatformamigaos3"; then
  AMIGAFLAVOR=-Dplatformamigaos4
fi

if test x"$AMIGAFLAVOR" = "x"; then
  AMIGAFLAVOR="-Dplatformamigaall"
fi

echo generating amigaguide
$MAKEGUIDE $AMIGAFLAVOR --error-limit 10000 --amiga -o $GUIDEFILE $TEXIFILE
echo done
