#!/bin/sh

#
# nextfix.sh - NextStep binary header fixer for I586
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
# Usage: nextfix.sh <binary>
#                    $1
#

echo fixing $1, please wait...
dd if=$1 of=$1.cut bs=1 skip=9
printf '\xce\xfa\xed\xfe\x07\x00\x00\x00\x03' >$1.head
cat $1.head $1.cut >$1.ok
mv $1.ok $1
chmod +x $1
rm $1.head $1.cut
echo $1 has been fixed.
