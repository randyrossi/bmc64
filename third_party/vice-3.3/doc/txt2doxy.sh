#! /bin/bash

#
# txt2doxy.sh - fixup ascii text files
#
# Written by
#  groepaz <groepaz@gmx.net>
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

# this script is used to fix up ascii text files for inclusion in the doxygen
# documentation. it replaces some characters by their proper html equivalent,
# inserts line breaks and tries to link urls.
#
# note: this only covers very few things at the moment and should probably get
#       enhanced =)
#
echo "<tt>"
cat $1 | \
    sed -s 's/&/\&amp;/g' |
    sed -s 's/>/\&gt;/g' |
    sed -s 's/</\&lt;/g' |
    sed -s 's:\(http\://[^ )]*\):\<a href=\"\1\"\>\1\</a\>:' |
    sed -s 's/$/<br>/g' |
    sed -s 's/\t/        /g' |
    sed -s 's/  /\&nbsp;\&nbsp;/g'
echo "</tt>"
