#!/bin/bash

#
# vice-launcher.sh - runs vice from the nearby application bundle
#
# Written by
#  Christian Vogelgsang <chris@vogelgsang.org>
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
# Usage: make-bindist.sh <strip> <vice-version> <host-cpu> <host-system> <enable_arch> <zip|nozip> <x64sc-included> <top-srcdir> <exe-ext>
#                         $1      $2             $3         $4            $5            $6          $7               $8           $9
#

REALFILE="$0"

# find dir of real file (not the linked ones)
DIR="`dirname \"$REALFILE\"`"
while [ -h "$REALFILE" ]; do
  REALFILE="`readlink \"$REALFILE\"`"
  CURDIR="`dirname \"$REALFILE\"`"
  if [ "${CURDIR:0:1}" = "/" ]; then
    DIR="$CURDIR"
  else
    DIR="$DIR/$CURDIR"
  fi
done

NAME="`basename \"$0\"`"

# find bundle
BUNDLE="$DIR/../VICE.app"
if [ ! -d "$BUNDLE" ]; then
  BUNDLE="$DIR/../$NAME.app"
  if [ ! -d "$BUNDLE" ]; then
    echo "Error: associated bundle '$BUNDLE' not found!"
    exit 1
  fi
fi
BASENAME="`basename \"$BUNDLE\" .app`"

# find launcher
if [ -e "$BUNDLE/Contents/Resources/script" ]; then
  LAUNCHER="$BUNDLE/Contents/Resources/script"
elif [ -e "$BUNDLE/Contents/MacOS/$BASENAME" ]; then
  LAUNCHER="$BUNDLE/Contents/MacOS/$BASENAME"
else
  echo "Error: no launcher script found in '$BUNDLE'!"
  exit 1
fi

# run launcher
export PROGRAM="$NAME"
exec "$LAUNCHER" "$@"
