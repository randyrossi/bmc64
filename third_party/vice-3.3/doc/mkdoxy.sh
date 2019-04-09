#!/bin/bash

#
# mkdoxy.sh - Generate doxygen documentation
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

################################################################################
#
# to get the most out of doxygen we must make sure that it only "sees" the files
# which belong to a respective configuration. this is done by first specifying
# all directories which contain actual sourcefiles in the "getinputs" function,
# and then excluding files which are not required (eg for a specific emulator)
# in the "getexcludes" function.
#
# adding a new configuration:
# - update "getinputs" and make sure that for that config only directories which
#   contain sourcefiles for this config go into the INPUT variable.
# - generate the documentation, and look at the file list
# - update "getexcludes" and make sure that for this config all files which do
#   not belong to it are excluded and end up in the EXCLUDE variable.
# - generate the documentation again and verify all is ok :) 
# - add this config to the list of working ones in the howto
#
# http://www.stack.nl/~dimitri/doxygen/config.html - Doyxgen config
# http://qof.sourceforge.net/doxy/reference.html - Doxygen reference
################################################################################

################################################################################
# this function returns all directories which contain input files for a specifc
# configuration.
#
# $1    machine
# $2    port
# $3    ui
#
# returns path(es) in INPUT
################################################################################
function getinputs
{
INPUT=" ../src"
INPUT+=" mainpage.dox"

ARCH_INPUT=" ../src/arch"

ARCH_AMIGAOS_INPUT=" ../src/arch/amigaos"
ARCH_AMIGAOS_INPUT+=" ../src/arch/amigaos/info-files"
ARCH_AMIGAOS_INPUT+=" ../src/arch/amigaos/info-files/morphos"
ARCH_AMIGAOS_INPUT+=" ../src/arch/amigaos/mui"

ARCH_BEOS_INPUT=" ../src/arch/beos"

ARCH_MSDOS_INPUT=" ../src/arch/msdos"

ARCH_OS2_INPUT=" ../src/arch/os2"
ARCH_OS2_INPUT+=" ../src/arch/os2/dialogs"
ARCH_OS2_INPUT+=" ../src/arch/os2/doc"
ARCH_OS2_INPUT+=" ../src/arch/os2/fonts"
ARCH_OS2_INPUT+=" ../src/arch/os2/icons"
ARCH_OS2_INPUT+=" ../src/arch/os2/kbd"
ARCH_OS2_INPUT+=" ../src/arch/os2/snippets"
ARCH_OS2_INPUT+=" ../src/arch/os2/unused"
ARCH_OS2_INPUT+=" ../src/arch/os2/unused/pm"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/ctools"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/incl"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/x128"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/x64"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/x64dtv"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/x64sc"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/xcbm2"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/xpet"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/xplus4"
ARCH_OS2_INPUT+=" ../src/arch/os2/vac++/xvic"
ARCH_OS2_INPUT+=" ../src/arch/os2/watcom"

ARCH_RISCOS_INPUT=" ../src/arch/riscos"
ARCH_RISCOS_INPUT+=" ../src/arch/riscos/binfiles"

#../src/arch/sdl
#../src/arch/sdl/dingoo-files
#../src/arch/sdl/syllable-files
#../src/arch/sdl/win32-msvc
#../src/arch/sdl/win32-watcom
#../src/arch/sdl/win64-msvc
#../src/arch/sdl/winmips-msvc
#../src/arch/sdl/xbox

if [ "$3" = "gtk3" ]; then
    # FIXME: add unix specific directories used in GTK3 here
    ARCH_UNIX_INPUT=" "
else
    ARCH_UNIX_INPUT=" ../src/arch/unix"
    ARCH_UNIX_INPUT+=" ../src/arch/unix/gui"
    ARCH_UNIX_INPUT+=" ../src/arch/unix/readline"
fi

#../src/arch/unix/hpux

#../src/arch/unix/macosx
#../src/arch/unix/macosx/Resources
#../src/arch/unix/macosx/Resources/English.lproj
#../src/arch/unix/macosx/Resources/English.lproj/DisassemblyWindow.nib
#../src/arch/unix/macosx/Resources/English.lproj/DriveSettings.nib
#../src/arch/unix/macosx/Resources/English.lproj/FilePanelImageContents.nib
#../src/arch/unix/macosx/Resources/English.lproj/IECDriveSettings.nib
#../src/arch/unix/macosx/Resources/English.lproj/IOTreeWindow.nib
#../src/arch/unix/macosx/Resources/English.lproj/JoystickSettings.nib
#../src/arch/unix/macosx/Resources/English.lproj/KeyboardSettings.nib
#../src/arch/unix/macosx/Resources/English.lproj/MemoryWindow.nib
#../src/arch/unix/macosx/Resources/English.lproj/NetplayControl.nib
#../src/arch/unix/macosx/Resources/English.lproj/PrinterSettings.nib
#../src/arch/unix/macosx/Resources/English.lproj/RecordHistory.nib
#../src/arch/unix/macosx/Resources/English.lproj/RecordMedia.nib
#../src/arch/unix/macosx/Resources/English.lproj/RegisterWindow.nib
#../src/arch/unix/macosx/Resources/English.lproj/ResourceEditor.nib
#../src/arch/unix/macosx/Resources/English.lproj/SIDSettings.nib
#../src/arch/unix/macosx/Resources/English.lproj/SoundSettings.nib
#../src/arch/unix/macosx/Resources/English.lproj/VICEInformation.nib
#../src/arch/unix/macosx/Resources/English.lproj/VideoSettings.nib
#../src/arch/unix/macosx/Resources/English.lproj/x128.nib
#../src/arch/unix/macosx/Resources/English.lproj/x64.nib
#../src/arch/unix/macosx/Resources/English.lproj/x64dtv.nib
#../src/arch/unix/macosx/Resources/English.lproj/x64sc.nib
#../src/arch/unix/macosx/Resources/English.lproj/xcbm2.nib
#../src/arch/unix/macosx/Resources/English.lproj/xpet.nib
#../src/arch/unix/macosx/Resources/English.lproj/xplus4.nib
#../src/arch/unix/macosx/Resources/English.lproj/xvic.nib
#../src/arch/unix/macosx/VICE.xcodeproj
#../src/arch/unix/macosx/cocoa
#../src/arch/unix/macosx/cocoa/dialog
#../src/arch/unix/macosx/cocoa/menu
#../src/arch/unix/macosx/cocoa/view

#../src/arch/unix/minix

#../src/arch/unix/next_open_rhap

#../src/arch/unix/qnx4

#../src/arch/unix/qnx6

#../src/arch/unix/sco_sol

#../src/arch/unix/skyos
#../src/arch/unix/skyos/icons

#../src/arch/unix/vms

ARCH_UNIX_X11_INPUT=" ../src/arch/unix/x11"
ARCH_UNIX_GNOME_INPUT=" ../src/arch/unix/x11/gnome"
ARCH_UNIX_XAW_INPUT=" ../src/arch/unix/x11/xaw"
ARCH_UNIX_XAW_INPUT+=" ../src/arch/unix/x11/xaw/widgets"

#../src/arch/watcom-source-wrappers

#../src/arch/win32
#../src/arch/win32/evc
#../src/arch/win32/mingw32
#../src/arch/win32/msvc
#../src/arch/win32/msvc/wpcap
#../src/arch/win32/msvc/wpcap/net
#../src/arch/win32/utils
#../src/arch/win32/watcom
#../src/arch/win64

ARCH_GTK3_INPUT=" ../src/arch/gtk3"
ARCH_GTK3_INPUT+=" ../src/arch/gtk3/widgets"
ARCH_GTK3_INPUT+=" ../src/arch/gtk3/widgets/base"


INPUT+=" ../src/core"
INPUT+=" ../src/diag"
INPUT+=" ../src/diskimage"
INPUT+=" ../src/drive"
INPUT+=" ../src/drive/iec"
INPUT+=" ../src/fileio"
INPUT+=" ../src/fsdevice"
INPUT+=" ../src/gfxoutputdrv"
INPUT+=" ../src/hwsiddrv"
INPUT+=" ../src/iecbus"
INPUT+=" ../src/imagecontents"
INPUT+=" ../src/monitor"
INPUT+=" ../src/parallel"
INPUT+=" ../src/printerdrv"
INPUT+=" ../src/raster"
INPUT+=" ../src/rs232drv"
INPUT+=" ../src/rtc"
INPUT+=" ../src/samplerdrv"
INPUT+=" ../src/serial"
INPUT+=" ../src/sounddrv"
INPUT+=" ../src/tape"
INPUT+=" ../src/tapeport"
INPUT+=" ../src/userport"
INPUT+=" ../src/vdrive"
INPUT+=" ../src/video"

# external libs
LIB_INPUT=" ../src/lib"
LIB_INPUT+=" ../src/lib/libffmpeg"
LIB_INPUT+=" ../src/lib/libffmpeg/libavcodec"
LIB_INPUT+=" ../src/lib/libffmpeg/libavdevice"
LIB_INPUT+=" ../src/lib/libffmpeg/libavformat"
LIB_INPUT+=" ../src/lib/libffmpeg/libavutil"
LIB_INPUT+=" ../src/lib/libffmpeg/libswscale"
# FIXME: add subdirs
LIB_INPUT+=" ../src/lib/liblame"
# FIXME: add subdirs
LIB_INPUT+=" ../src/lib/libx264"
LIB_INPUT+=" ../src/lib/p64"

# chips
CRTC_INPUT=" ../src/crtc"
VDC_INPUT=" ../src/vdc"
VICII_INPUT=" ../src/vicii"
VICIISC_INPUT=" ../src/viciisc"

SID_INPUT=" ../src/resid"
SID_INPUT+=" ../src/sid"

DTVSID_INPUT=" ../src/resid-dtv"
DTVSID_INPUT+=" ../src/sid"

# machines

VSID_INPUT=" ../src/c64"

C64_INPUT=" ../src/c64"
C64_INPUT+=" ../src/c64/cart"
C64_INPUT+=" ../src/drive/iec/c64exp"
C64_INPUT+=" ../src/drive/iecieee"
C64_INPUT+=" ../src/drive/ieee"
C64_INPUT+=" ../src/drive/tcbm"

C128_INPUT=" ../src/c128"
C128_INPUT+=" ../src/drive/iec128dcr"
C128_INPUT+=" ../src/drive/iecieee"
C128_INPUT+=" ../src/drive/ieee"
C128_INPUT+=" ../src/drive/tcbm"

DTV_INPUT=" ../src/c64dtv"
DTV_INPUT+=" ../src/drive/iecieee"
DTV_INPUT+=" ../src/drive/ieee"
DTV_INPUT+=" ../src/drive/tcbm"

CBM2_INPUT=" ../src/cbm2"
CBM2_INPUT+=" ../src/drive/iecieee"
CBM2_INPUT+=" ../src/drive/ieee"
CBM2_INPUT+=" ../src/drive/tcbm"

PET_INPUT=" ../src/pet"
PET_INPUT+=" ../src/drive/iecieee"
PET_INPUT+=" ../src/drive/ieee"
PET_INPUT+=" ../src/drive/tcbm"

PLUS4_INPUT=" ../src/plus4"
PLUS4_INPUT+=" ../src/drive/iec/plus4exp"
PLUS4_INPUT+=" ../src/drive/iecieee"
PLUS4_INPUT+=" ../src/drive/ieee"
PLUS4_INPUT+=" ../src/drive/tcbm"

VIC20_INPUT=" ../src/vic20"
VIC20_INPUT+=" ../src/vic20/cart"
VIC20_INPUT+=" ../src/drive/iecieee"
VIC20_INPUT+=" ../src/drive/ieee"
VIC20_INPUT+=" ../src/drive/tcbm"

# machine
case "$1" in
"vsid")
    INPUT+="$VSID_INPUT"
    INPUT+="$SID_INPUT $VICII_INPUT"
   ;;
"x128")
    INPUT+="$C128_INPUT"
    INPUT+="$SID_INPUT $VICII_INPUT $VDC_INPUT"
   ;;
"x64")
    INPUT+="$C64_INPUT"
    INPUT+="$SID_INPUT $VICII_INPUT"
   ;;
"x64dtv")
    INPUT+="$DTV_INPUT"
    INPUT+="$DTVSID_INPUT $SID_INPUT"
   ;;
"x64sc")
    INPUT+="$C64_INPUT"
    INPUT+="$SID_INPUT $VICIISC_INPUT"
   ;;
"xcbm2")
    INPUT+="$CBM2_INPUT"
    INPUT+="$CRTC_INPUT"
   ;;
"xcbm5x0")
    INPUT+="$CBM2_INPUT"
    INPUT+="$SID_INPUT $VICII_INPUT"
   ;;
"xpet")
    INPUT+="$PET_INPUT"
    INPUT+="$CRTC_INPUT"
   ;;
"xplus4")
    INPUT+="$PLUS4_INPUT"
    INPUT+="$TED_INPUT"
   ;;
"xvic")
    INPUT+="$VIC20_INPUT"
    INPUT+="$VIC_INPUT"
   ;;
"xscpu64")
    INPUT+="$C64_INPUT"
    INPUT+="$SID_INPUT $VICII_INPUT"
   ;;
*)
   ;;
esac


# port
case "$2" in
"linux")
    ARCH_INPUT+="$ARCH_UNIX_INPUT"
    ;;
"win32")
    ARCH_INPUT+="$ARCH_WIN32_INPUT"
    ;;
"osx")
    ARCH_INPUT+="$ARCH_OSX_INPUT"
    ;;
*)
    ;;
esac

# gui
case "$3" in
"gtk3")
    ARCH_INPUT+="$ARCH_GTK3_INPUT"
   ;;
"gtk")
    ARCH_INPUT+="$ARCH_UNIX_X11_INPUT $ARCH_UNIX_GNOME_INPUT"
   ;;
"win32")
   ;;
"cocoa")
   ;;
"sdl")
   ;;
*)
   ;;
esac

    INPUT+="$ARCH_INPUT $LIB_INPUT"
    INCLUDE="$INPUT"
}

################################################################################
# this function returns all files which should be excluded from the input for a
# specific configuration.
#
# $1    machine
# $2    port
# $3    ui
#
# returns file(s) in EXCLUDE
################################################################################
function getexcludes
{
#echo "getting excludes for" $1 $2 $3

ALWAYS_EXCLUDE=" ../src/translate_text.c.po.c"
ALWAYS_EXCLUDE+=" ../src/monitor/mon_lex.c"
ALWAYS_EXCLUDE+=" ../src/monitor/mon_parse.c"

ARCH_LINUX_EXCLUDE=" ../src/arch/platform_aix_version.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/platform_freebsd_version.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/platform_netbsd_version.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/platform_openbsd_version.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/platform_qnx6_version.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/unix/vmstermios.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/unix/x11/qnxipc.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/unix/x11/qnxshm.c"
ARCH_LINUX_EXCLUDE+=" ../src/arch/unix/x11/qnxshm.h"
ARCH_LINUX_EXCLUDE+=" ../src/sounddrv/soundaix.c"
ARCH_LINUX_EXCLUDE+=" ../src/sounddrv/soundbeos.cc"
ARCH_LINUX_EXCLUDE+=" ../src/sounddrv/soundallegro.c"
ARCH_LINUX_EXCLUDE+=" ../src/sounddrv/soundhpux.c"
ARCH_LINUX_EXCLUDE+=" ../src/sounddrv/soundsgi.c"
ARCH_LINUX_EXCLUDE+=" ../src/sounddrv/soundsun.c"
ARCH_LINUX_EXCLUDE+=" ../src/sounddrv/soundcoreaudio.c"
ARCH_LINUX_EXCLUDE+=" ../src/sounddrv/soundmmos2.c"
ARCH_LINUX_EXCLUDE+=" ../src/gfxoutputdrv/quicktimedrv.c"
ARCH_LINUX_EXCLUDE+=" ../src/gfxoutputdrv/quicktimedrv.h"
ARCH_LINUX_EXCLUDE+=" ../src/video/render1x1_dingoo.h"
# FIXME: add non unix GTK3 stuff
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/archdep_win32.c"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/archdep_win32.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/dynlib-win32.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-osx-hid.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-osx-hid.c"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-osx-hidlib.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-osx-hidmgr.c"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-osx-hidutil.c"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-osx.c"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-osx.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-win32-dinput-handle.c"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-win32-dinput-handle.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-win32.c"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/joy-win32.h"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/rawnetarch_win32.c"
ARCH_LINUX_EXCLUDE+=" ../src/arch/gtk3/rs232-win32-dev.c"
# FIXME: add non unix SDL stuff

GUI_GTK_EXCLUDE+=" ../src/vice_sdl.h"

GUI_GTK3_EXCLUDE+=" ../src/vice_sdl.h"

C64EXCLUDE+=" ../src/monitor/asm6502dtv.c"
C64EXCLUDE+=" ../src/monitor/asmz80.c"
C64EXCLUDE+=" ../src/monitor/mon_assemblez80.c"
C64EXCLUDE+=" ../src/c1541.c"
C64EXCLUDE+=" ../src/bin2c.c"
C64EXCLUDE+=" ../src/cartconv.c"
C64EXCLUDE+=" ../src/petcat.c"
C64EXCLUDE+=" ../src/c128ui.h"
C64EXCLUDE+=" ../src/cbm2ui.h"
C64EXCLUDE+=" ../src/petui.h"
C64EXCLUDE+=" ../src/plus4ui.h"
C64EXCLUDE+=" ../src/vic20ui.h"
C64EXCLUDE+=" ../src/z80regs.h"
C64EXCLUDE+=" ../src/mainviccpu.c"

X64EXCLUDE="$C64EXCLUDE"
X64EXCLUDE+=" ../src/c64/vsidstubs.c"
X64EXCLUDE+=" ../src/c64/vsidmem.c"
X64EXCLUDE+=" ../src/c64/vsid.c"
X64EXCLUDE+=" ../src/vsidui.h"

X64SCEXCLUDE="$C64EXCLUDE"
X64SCEXCLUDE+=" ../src/c64/vsidstubs.c"
X64SCEXCLUDE+=" ../src/c64/vsidmem.c"
X64SCEXCLUDE+=" ../src/c64/vsid.c"
X64SCEXCLUDE+=" ../src/c64/c64mem.c"
X64SCEXCLUDE+=" ../src/c64/c64model.c"
X64SCEXCLUDE+=" ../src/6510core.c"
X64SCEXCLUDE+=" ../src/vsidui.h"
X64SCEXCLUDE+=" ../src/maincpu.c"

VSIDEXCLUDE="$C64EXCLUDE"

C64_GUI_GTK_EXCLUDE=" ../src/arch/unix/gui/c64dtvui.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/c128ui.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/cbm2ui.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/petui.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/plus4ui.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uic64dtv.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uic64dtv.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uicbm2cart.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uicbm2cart.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uicrtc.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uicrtc.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivec64dtv.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivec64dtv.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivec64vic20.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivec64vic20.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivec128.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivec128.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivepetcbm2.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivepetcbm2.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidriveplus4.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidriveplus4.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivevic20.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uidrivevic20.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uipetdww.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uipetdww.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uiplus4cart.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uiplus4cart.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uiprinteriecplus4.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uiprinteriecplus4.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uips2mouse.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uips2mouse.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uirs232petplus4cbm2.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uirs232petplus4cbm2.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uited.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uited.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uiv364speech.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uiv364speech.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uivdc.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uivdc.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uivic.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/uivic.h"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/vic20ui.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/x11/c64dtvicon.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/x11/c128icon.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/x11/cbm2icon.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/x11/peticon.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/x11/plus4icon.c"
C64_GUI_GTK_EXCLUDE+=" ../src/arch/unix/x11/vic20icon.c"

X64SC_GUI_GTK_EXCLUDE="$C64_GUI_GTK_EXCLUDE"
X64SC_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/vsidui.c"
X64SC_GUI_GTK_EXCLUDE+=" ../src/arch/unix/vsiduiunix.h"

X64SC_GUI_GTK_EXCLUDE="$C64_GUI_GTK_EXCLUDE"
X64SC_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/c64ui.c"
X64SC_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/vsidui.c"
X64SC_GUI_GTK_EXCLUDE+=" ../src/arch/unix/vsiduiunix.h"

VSID_GUI_GTK_EXCLUDE="$C64_GUI_GTK_EXCLUDE"
VSID_GUI_GTK_EXCLUDE+=" ../src/arch/unix/gui/c64ui.c"


C64_GUI_GTK3_EXCLUDE=" ../src/arch/gtk3/c64dtvui.c"
C64_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/c128ui.c"
C64_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/cbm2ui.c"
C64_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/cbm5x0ui.c"
C64_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/petui.c"
C64_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/plus4ui.c"
C64_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/scpu64ui.c"
C64_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/vic20ui.c"

X64SC_GUI_GTK3_EXCLUDE="$C64_GUI_GTK3_EXCLUDE"
X64SC_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/vsidui.c"
X64SC_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/vsidui.h"

X64SC_GUI_GTK3_EXCLUDE="$C64_GUI_GTK3_EXCLUDE"
X64SC_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/c64ui.c"
X64SC_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/vsidui.c"
X64SC_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/vsidui.h"

VSID_GUI_GTK3_EXCLUDE="$C64_GUI_GTK3_EXCLUDE"
VSID_GUI_GTK3_EXCLUDE+=" ../src/arch/gtk3/c64ui.c"

# machine
case "$1" in
"vsid")
    MACHINE_EXCLUDE="$VSIDEXCLUDE"
    GUI_GTK3_EXCLUDE+="$VSID_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$VSID_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$VSID_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$VSID_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$VSID_GUI_SDL_EXCLUDE"
   ;;
"x128")
    MACHINE_EXCLUDE="$X128EXCLUDE"
    GUI_GTK3_EXCLUDE+="$X128_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$X128_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$X128_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$X128_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$X128_GUI_SDL_EXCLUDE"
   ;;
"x64")
    MACHINE_EXCLUDE="$X64EXCLUDE"
    GUI_GTK3_EXCLUDE+="$X64_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$X64_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$X64_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$X64_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$X64_GUI_SDL_EXCLUDE"
   ;;
"x64dtv")
    MACHINE_EXCLUDE="$X64DTVEXCLUDE"
    GUI_GTK3_EXCLUDE+="$X64DTV_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$X64DTV_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$X64DTV_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$X64DTV_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$X64DTV_GUI_SDL_EXCLUDE"
   ;;
"x64sc")
    MACHINE_EXCLUDE="$X64SCEXCLUDE"
    GUI_GTK3_EXCLUDE+="$X64SC_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$X64SC_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$X64SC_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$X64SC_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$X64SC_GUI_SDL_EXCLUDE"
   ;;
"xcbm2")
    MACHINE_EXCLUDE="$XCBM2EXCLUDE"
    GUI_GTK3_EXCLUDE+="$XCBM2_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$XCBM2_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$XCBM2_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$XCBM2_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$XCBM2_GUI_SDL_EXCLUDE"
   ;;
"xcbm5x0")
    MACHINE_EXCLUDE="$XCBM5X0EXCLUDE"
    GUI_GTK3_EXCLUDE+="$XCBM5X0_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$XCBM5X0_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$XCBM5X0_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$XCBM5X0_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$XCBM5X0_GUI_SDL_EXCLUDE"
   ;;
"xpet")
    MACHINE_EXCLUDE="$XPETEXCLUDE"
    GUI_GTK3_EXCLUDE+="$XPET_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$XPET_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$XPET_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$XPET_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$XPET_GUI_SDL_EXCLUDE"
   ;;
"xplus4")
    MACHINE_EXCLUDE="$XPLUS4EXCLUDE"
    GUI_GTK3_EXCLUDE+="$XPLUS4_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$XPLUS4_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$XPLUS4_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$XPLUS4_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$XPLUS4_GUI_SDL_EXCLUDE"
   ;;
"xvic")
    MACHINE_EXCLUDE="$XVICEXCLUDE"
    GUI_GTK3_EXCLUDE+="$XVIC_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$XVIC_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$XVIC_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$XVIC_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$XVIC_GUI_SDL_EXCLUDE"
   ;;
"xscpu64")
    MACHINE_EXCLUDE="$XSCPU64EXCLUDE"
    GUI_GTK3_EXCLUDE+="$XSCPU64_GUI_GTK3_EXCLUDE"
    GUI_GTK_EXCLUDE+="$XSCPU64_GUI_GTK_EXCLUDE"
    GUI_WIN32_EXCLUDE+="$XSCPU64_GUI_WIN32_EXCLUDE"
    GUI_COCOA_EXCLUDE+="$XSCPU64_GUI_COCOA_EXCLUDE"
    GUI_SDL_EXCLUDE+="$XSCPU64_GUI_SDL_EXCLUDE"
   ;;
*)
   ;;
esac

# port
case "$2" in
"linux")
    ARCH_EXCLUDE="$ARCH_LINUX_EXCLUDE"
   ;;
"win32")
    ARCH_EXCLUDE="$ARCH_WIN32_EXCLUDE"
   ;;
"osx")
    ARCH_EXCLUDE="$ARCH_OSX_EXCLUDE"
   ;;
*)
   ;;
esac

# gui
case "$3" in
"gtk3")
    GUI_EXCLUDE="$GUI_GTK3_EXCLUDE"
   ;;
"gtk")
    GUI_EXCLUDE="$GUI_GTK_EXCLUDE"
   ;;
"win32")
    GUI_EXCLUDE="$GUI_WIN32_EXCLUDE"
   ;;
"cocoa")
    GUI_EXCLUDE="$GUI_COCOA_EXCLUDE"
   ;;
"sdl")
    GUI_EXCLUDE="$GUI_SDL_EXCLUDE"
   ;;
*)
   ;;
esac

EXCLUDE="$ALWAYS_EXCLUDE $MACHINE_EXCLUDE $ARCH_EXCLUDE $GUI_EXCLUDE"

}

################################################################################
# this function creates the documentation for one specific configuration
#
# $1    machine
# $2    port
# $3    ui
################################################################################
function makedocs
{
    echo "making docs for "$1" source ("$2", "$3") ["$VERSION"]" 
    OUTPUT="./doxy/"$1"/"
    rm -rf $OUTPUT
    mkdir -p $OUTPUT
    getinputs $1 $2 $3
    getexcludes $1 $2 $3
    echo "INPUT="$INPUT
    echo "INCLUDE_PATH="$INCLUDE
    echo "EXCLUDE="$EXCLUDE
    echo "PREDEFINED="$PREDEFINED
(cat Doxyfile ;\
     echo "INPUT=" $INPUT ;\
     echo "OUTPUT_DIRECTORY=" $OUTPUT ;\
     echo "INCLUDE_PATH=" $INCLUDE ;\
     echo "EXCLUDE=" $EXCLUDE ;\
     echo "PREDEFINED="$PREDEFINE ;\
     echo "PROJECT_NAME="$1 ;\
     echo "PROJECT_NUMBER="$VERSION ;\
    ) | doxygen -
}

################################################################################
# this function creates and index.html entry page in ./doxy
################################################################################

function makeindex
{
    echo "making index.html"
    OUTPUT="./doxy/index.html"

    echo "<html><head>" > $OUTPUT
    echo "<title>VICE doxy</title></head><body>" >> $OUTPUT
    echo "<h1>VICE doxy</h1>" >> $OUTPUT

    for I in x64 x64sc x64dtv xscpu64 x128 xcbm2 xcbm5x0 xpet xplus4 xvic vsid; do \
        if [ -a ./doxy/$I ]; then \
            echo $I; \
            echo "<a href=\""$I"/html/index.html\">"$I"</a>" >> $OUTPUT; \
        fi; \
    done

    echo "</body></html>" >> $OUTPUT
}

################################################################################
# handle commandline arguments
################################################################################

# TODO: detect port/ui automatically
# TODO: optionally enable call graphs
# TODO: optionally enable source browser

# defaults
MACHINE="all"
PORT="linux"
GUI="gtk3"

# machine
case "$1" in
"all")
   MACHINE="all"
   ;;
"vsid")
   MACHINE="vsid"
   ;;
"x128")
   MACHINE="x128"
   ;;
"x64")
   MACHINE="x64"
   ;;
"x64dtv")
   MACHINE="x64dtv"
   ;;
"x64sc")
   MACHINE="x64sc"
   ;;
"xcbm2")
   MACHINE="xcbm2"
   ;;
"xcbm5x0")
   MACHINE="xcbm5x0"
   ;;
"xpet")
   MACHINE="xpet"
   ;;
"xplus4")
   MACHINE="xplus4"
   ;;
"xvic")
   MACHINE="xvic"
   ;;
"xscpu64")
   MACHINE="xscpu64"
   ;;
*)
   ;;
esac

# port
case "$2" in
"linux")
   PORT="linux"
   GUI="gtk3"
   ;;
"win32")
   PORT="win32"
   GUI="gtk3"
   ;;
"osx")
   PORT="osx"
   GUI="gtk3"
   ;;
*)
   ;;
esac

# gui
case "$3" in
"xaw")
   GUI="xaw"
   ;;
"gtk")
   GUI="gtk"
   ;;
"gtk3")
   GUI="gtk3"
   ;;
"cocoa")
   GUI="cocoa"
   ;;
"sdl")
   GUI="sdl"
   ;;
*)
   ;;
esac

VERSION=`grep " VERSION " ../src/config.h | sed 's:#define VERSION \"\(.*\)\":\1:'`
PREDEFINED=`cpp -dD < ../src/config.h | grep define | sed -s 's:#define ::g' | sed -s 's: :=:' | sed -s 's:=$::' | grep -v '__' | tr '\n' ' '`

#echo $MACHINE $PORT $GUI
if [ "$MACHINE" = "all" ]; then
    makedocs "vsid" $PORT $GUI
    makedocs "x128" $PORT $GUI
    makedocs "x64" $PORT $GUI
    makedocs "x64dtv" $PORT $GUI
    makedocs "x64sc" $PORT $GUI
    makedocs "xcbm2" $PORT $GUI
    makedocs "xcbm5x0" $PORT $GUI
    makedocs "xpet" $PORT $GUI
    makedocs "xplus4" $PORT $GUI
    makedocs "xvic" $PORT $GUI
    makedocs "xscpu64" $PORT $GUI
else
    makedocs $MACHINE $PORT $GUI
fi

makeindex
