## Makefile.am for the BEOS version of `libarch.a'.

SUFFIXES=.def

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/vdrive \
	-I$(top_srcdir)/src/monitor \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c64/cart \
	-I$(top_srcdir)/src/c64dtv \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/plus4 \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/imagecontents \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/platform \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/samplerdrv

noinst_LIBRARIES = libarch.a

libarch_a_SOURCES = \
	archdep.c \
	archdep.h \
	blockdev.c \
	c128ui.cc \
	c64ui.cc \
	c64dtvui.cc \
	cbm2ui.cc \
	cbm5x0ui.cc \
	console.c \
	joy.cc \
	joy.h \
	kbd.c \
	kbd.h \
	mousedrv.cc \
	mousedrv.h \
	petui.cc \
	plus4ui.cc \
	scpu64ui.cc \
	signals.c \
	statusbar.cc \
	statusbar.h \
	ui.cc \
	ui.h \
	ui_autostart.cc \
	ui_autostart.h \
	ui_cbm2.cc \
	ui_cbm2.h \
	ui_cbm5x0.cc \
	ui_cbm5x0.h \
	ui_cia.cc \
	ui_cia.h \
	ui_datasette.cc \
	ui_datasette.h \
	ui_drive.cc \
	ui_drive.h \
	ui_device.cc \
	ui_device.h \
	ui_file.cc \
	ui_file.h \
	ui_ide64.cc \
	ui_ide64.h \
	ui_ide64usb.cc \
	ui_ide64usb.h \
	ui_joystick.cc \
	ui_joystick.h \
	ui_keymap.cc \
	ui_keymap.h \
	ui_netplay.cc \
	ui_netplay.h \
	ui_pet.cc \
	ui_pet.h \
	ui_printer.cc \
	ui_printer.h \
	ui_ram.cc \
	ui_ram.h \
	ui_sid.cc \
	ui_sid.h \
	ui_sidcart.cc \
	ui_sidcart.h \
	ui_sound.cc \
	ui_sound.h \
	ui_ted.cc \
	ui_ted.h \
	ui_vic20.cc \
	ui_vic20.h \
	ui_vic.cc \
	ui_vic.h \
	ui_vicii.cc \
	ui_vicii.h \
	ui_video.cc \
	ui_video.h \
	uimon.c \
	vic20ui.cc \
	viceapp.cc \
	viceapp.h \
	vicemenu.cc \
	vicemenu.h \
	vicewindow.cc \
	vicewindow.h \
	video.cc \
	videoarch.h \
	vsidui.cc \
	vsyncarch.c

EXTRA_DIST = \
	constants.txt \
	genconstants.c \
	make-bindist.sh \
	ppc-ar.sh \
	ppc-cc.sh

BUILT_SOURCES = constants.h

if NATIVE_TOOLS
genconstants:	$(srcdir)/genconstants.c
	$(NATIVE_TOOLS_COMPILER) -o genconstants $(srcdir)/genconstants.c \
		-I$(top_builddir)/src -I$(top_srcdir)/src \
		-I$(top_srcdir)/src/joyport @ARCH_INCLUDES@

constants.h:	$(srcdir)/constants.txt genconstants
	./genconstants $(srcdir)/constants.txt constants.h
else
constants.h:	$(srcdir)/constants.txt
	echo "need native tools support to generate "$@
endif
