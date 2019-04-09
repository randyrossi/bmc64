#!/bin/sh

#
# texi2chm.sh - This script uses t2h.pl and the windows chm compiler to generate vice.chm
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
# Usage: texi2chm.sh <hhc> <chm-file> <texi-file> <fixdox.sh> <t2h.pl>
#                     $1    $2         $3          $4          $5
#

HHC=$1
CHMFILE=$2
TEXIFILE=$3
FIXDOXSH=$4
T2HPL=$5

rm -f -r chmtmp
mkdir chmtmp
cp $FIXDOXSH chmtmp
cp $T2HPL chmtmp
cp $TEXIFILE chmtmp
cd chmtmp
./fixdox.sh chm <vice.texi >vicefix.texi
rm -f vice.texi
mv vicefix.texi vice.texi
./t2h.pl vice.texi
$HHC vice.hhp
cd ..
mv chmtmp/vice.chm $CHMFILE
rm -f -r chmtmp
