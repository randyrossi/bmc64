#!/bin/sh

#
# build-sdl-package.sh - build a minix 3.x SDL based binary package
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
# this script needs to be run from the top of the vice tree
#

echo "Generating Minix-3.x SDL binary archive"

# see if we are in the top of the tree
if [ ! -f configure.proto ]; then
  cd ../..
  if [ ! -f configure.proto ]; then
    echo "please run this script from the base of the VICE directory"
    echo "or from the appropriate build directory"
    exit 1
  fi
fi

curdir=`pwd`
curdirbase=`basename $curdir`
echo >SDL.build "SDL"

cd ..

binpackage $curdirbase .

files=""

bzip2 -d $curdirbase.tar.bz2
for i in `tar -tf $curdirbase.tar`
do
  checkfile="${i:0:10}"
  if test x"$checkfile" != "x/usr/local"; then
    if test x"$i" != "x.minixpackage"; then
      files="$files --delete $i"
    fi
  fi
done
tar $files -f $curdirbase.tar
bzip2 $curdirbase.tar

mv $curdirbase.tar.bz2 SDL-$curdirbase.tar.bz2

echo SDL Minix-3.x package generated as ../SDL-$curdirbase.tar.bz2
