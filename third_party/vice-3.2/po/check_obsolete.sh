#!/bin/sh

#
# check_obsolete.sh - script for checking for unused/obsolete translation elements.
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

echo "checking for obsolete translation items, this can take a while..."

files=""

# build a list of .c files
cfiles=`find ../src -name "*.c"`

# build a list of .h files
hfiles=`find ../src -name "*.h"`

# get rid of all the crap
for i in $cfiles
do
  case $i in
    *.po.c)
      ;;
    *unix*)
      ;;
    *res*.rc*)
      ;;
    *intl_text.c)
      ;;
    *translate_text.c)
      ;;
    *)
      files="$files $i"
      ;;
  esac
done

# get rid of more of the crap
for i in $hfiles
do
  case $i in
    *unix*)
      ;;
    *intl*.h)
      ;;
    *translate*.h)
      ;;
    *)
      files="$files $i"
  esac
done

# generate a file with possible translation ids
rm -f tmp_g.txt
for i in $files
do
  grep >>tmp_g.txt -e IDGS_* -e IDCLS_* -e IDMS_* -e IDMES_* -e IDS_* $i
done

# search through the file for every id and report missing ones
while read data
do
  case ${data%%_*} in
    ID*)
      result=`grep $data tmp_g.txt`
      if test x"$result" = "x"; then
        echo "$data is not used"
      fi
      ;;
  esac
done

rm -f tmp_g.txt
