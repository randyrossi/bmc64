#!/bin/sh

#
# install.sh - install Syllable distribution in the correct location
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

current_dir=`pwd`

echo "Installing VICE as an Applaction collection."
if test x"$current_dir" != "x/usr/VICE"; then
  rm -f -r /usr/VICE
  mkdir /usr/VICE
  mkdir /usr/VICE/bin
  mkdir /usr/VICE/lib
  cp -r bin/* /usr/VICE/bin
  cp -r lib/* /usr/VICE/lib
fi
rm -f -r /Applications/VICE
mkdir /Applications/VICE
cp x*.sh /Applications/VICE
cp uninstall.sh /Applications/VICE
if [ -e /usr/VICE/bin/x64sc ]
then
  rm -f /Applications/VICE/x64sh.sh
fi
echo "VICE has been installed and can be started from the desktop Applications item."
echo "The installation directory can now be deleted if needed."
