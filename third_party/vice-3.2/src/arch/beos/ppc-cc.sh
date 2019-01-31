#!/bin/sh

#
# ppc-cc.sh - PPC BeOS compiler wrapper
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
# This script makes sure a 'valid' output file is generated for when the configure script checks the compiler
#

foundo=no

for i in $*
do
  if test x"$i" = "x-o" -o x"$i" = "x-c"; then
    foundo=yes
  fi
done

if test x"$foundo" = "xno"; then
  mwccppc $* -o a.out
else
  mwccppc $*
fi
