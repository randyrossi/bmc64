#!/bin/sh

#
# gcccpu.sh - This script compiles certain "cpu" files with -O0 or -O1
#
# Written by
#  Marco van den Heuvel <blackystardust68@yahoo.com>
#  Marcus Sutton <loggedoubt@gmail.com>
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
# This script gets used in place of gcc, for compilers that crash
# or cannot compile the 'cpu' files with -O3/-O2
#

cpufile=no
cpuopt=no
oldopt=no
filename=no

for i in $*
do
  case "$i" in
    *asm*.c|*mon_*.c)
      cpufile=no
      ;;
    *c64cpusc.c|*c64dtvcpu.c|*z80.c|*cpmcart.c)
      cpufile=yes
      cpuopt=-O1
      ;;
    *scpu64cpu.c)
      cpufile=yes
      cpuopt=-O0
      ;;
  esac

  case "$i" in
    -O2|-O3)
      oldopt=$i
      ;;
  esac

  case "$i" in
    *.c|*.cc)
      filename=$i
      ;;
  esac
done

newcmdline="gcc"

if test x"$filename" != "xno"; then
  if test x"$cpufile" = "xyes"; then
    echo "compiling $filename with $cpuopt"
  elif test x"$oldopt" != "xno"; then
    echo "compiling $filename with $oldopt"
  else
    echo "compiling $filename normally"
  fi
fi

for i in $*
do
  isopt=no
  case "$i" in
    -O2|-O3)
      isopt=yes
      ;;
  esac

  if test x"$cpufile" = "xyes" -a x"$isopt" = "xyes"; then
    newcmdline="$newcmdline $cpuopt"
  else
    newcmdline="$newcmdline $i"
  fi
done

$newcmdline
