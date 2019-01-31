#! /bin/bash

#
# src2doxy.sh - fixup some phrases
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

# this little script tries to fixup some phrases found in the source so they
# will (or will not) show up in the doxygen documentation
#
# /* BUG: ... -> /*! \bug ...
# /* FIXME: ... -> /*! \todo FIXME: ...
# /* TODO: ... -> /*! \todo ...
# /* #... */ (remove, commented out preprocessor stuff)
# /* ---... */ (remove)
# #define ... /* ... */ -> ... /*!< ... */ (autobrief)
# /* ... */ -> /*! ... */ (autobrief, convert comments to qt style if they begin 
#                          at start of line and the line ends right after them)
#
cat $1 | \
    sed -s 's/\* BUG:/\*! \\bug /g' |
    sed -s 's/\* FIXME:/\*! \\todo FIXME:/g' |
    sed -s 's/\* TODO:/\*! \\todo /g' |
    sed -s 's:^/\* #.*\*/$::' |
    sed -s 's:^/\*.---.*\*/$::' |
    sed -s 's:\(^#define .* \)\(/\* \)\(.*\*/$\):\1/\*!< \3:' |
    sed -s 's:^/\* \(.*\) \*/$:/\*! \1 \*/:'
