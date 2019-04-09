#!/bin/sh

#
# convert_owcc_libs.sh - convert openwatcom *.lib files to lib*.a files
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

for i in $*
do
    case $i in
        -lm) # special case, libm functions are in libc, so using any lib will do.
            cp $WATCOM/lib386/nt/version.lib ./libm.a
            cp $WATCOM/lib386/nt/version.lib ./m.a
            ;;
        -lkernel32)
            cp $WATCOM/lib386/nt/kernel32.lib ./libkernel32.a
            cp $WATCOM/lib386/nt/kernel32.lib ./kernel32.a
            ;;
        -luser32)
            cp $WATCOM/lib386/nt/user32.lib ./libuser32.a
            cp $WATCOM/lib386/nt/user32.lib ./user32.a
            ;;
        -lgdi32)
            cp $WATCOM/lib386/nt/gdi32.lib ./libgdi32.a
            cp $WATCOM/lib386/nt/gdi32.lib ./gdi32.a
            ;;
        -lwinmm)
            cp $WATCOM/lib386/nt/winmm.lib ./libwinmm.a
            cp $WATCOM/lib386/nt/winmm.lib ./winmm.a
            ;;
        -lcomdlg32)
            cp $WATCOM/lib386/nt/comdlg32.lib ./libcomdlg32.a
            cp $WATCOM/lib386/nt/comdlg32.lib ./comdlg32.a
            ;;
        -lcomctl32)
            cp $WATCOM/lib386/nt/comctl32.lib ./libcomctl32.a
            cp $WATCOM/lib386/nt/comctl32.lib ./comctl32.a
            ;;
        -lddraw)
            cp $WATCOM/lib386/nt/ddraw.lib ./libddraw.a
            cp $WATCOM/lib386/nt/ddraw.lib ./ddraw.a
            ;;
        -ldsound)
            cp $WATCOM/lib386/nt/directx/dsound.lib ./libdsound.a
            cp $WATCOM/lib386/nt/directx/dsound.lib ./dsound.a
            ;;
        -lth32)
            cp $WATCOM/lib386/nt/th32.lib ./libth32.a
            cp $WATCOM/lib386/nt/th32.lib ./th32.a
            ;;
        -lwsock32)
            cp $WATCOM/lib386/nt/wsock32.lib ./libwsock32.a
            cp $WATCOM/lib386/nt/wsock32.lib ./wsock32.a
            ;;
        -lversion)
            cp $WATCOM/lib386/nt/version.lib ./libversion.a
            cp $WATCOM/lib386/nt/version.lib ./version.a
            ;;
        -lole32)
            cp $WATCOM/lib386/nt/ole32.lib ./libole32.a
            cp $WATCOM/lib386/nt/ole32.lib ./ole32.a
            ;;
    esac
done

echo >owcc_libs_converted.h "/* dummy header */"
