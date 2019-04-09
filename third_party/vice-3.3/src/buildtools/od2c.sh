#!/bin/sh

#
# od2c.sh - convert 'od -Ax -t x1 -w8 <file>' output
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

start="yes"

while read data
do
  charlength=${#data}
  datalength=`expr $charlength - 6`
  tl=`expr $datalength / 3`
  if test x"$start" != "xyes"; then
    if [ $tl -ne 0 ]; then
        tdata="$tdata,"
        echo "    $tdata"
    fi
  else
    start=no
  fi
  if [ $tl -ne 0 ]; then
    if [ $tl -ge 1 ]; then
      tdata="0x${data:7:2}"
    fi
    if [ $tl -ge 2 ]; then
      tdata="$tdata, 0x${data:10:2}"
    fi
    if [ $tl -ge 3 ]; then
      tdata="$tdata, 0x${data:13:2}"
    fi
    if [ $tl -ge 4 ]; then
      tdata="$tdata, 0x${data:16:2}"
    fi
    if [ $tl -ge 5 ]; then
      tdata="$tdata, 0x${data:19:2}"
    fi
    if [ $tl -ge 6 ]; then
      tdata="$tdata, 0x${data:22:2}"
    fi
    if [ $tl -ge 7 ]; then
      tdata="$tdata, 0x${data:25:2}"
    fi
    if [ $tl -ge 8 ]; then
      tdata="$tdata, 0x${data:28:2}"
    fi
  fi
done
echo "    $tdata"
