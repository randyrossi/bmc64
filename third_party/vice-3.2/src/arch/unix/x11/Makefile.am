# Makefile.am for the Unix X11 specific code `libx11ui.a'.

if GNOMEUI
SUBDIRS = gnome
else
SUBDIRS = xaw
endif

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/arch/unix/gui

noinst_LIBRARIES = libx11ui.a

EXTRA_libx11ui_a_SOURCES = \
	openGL_sync.c \
	xrandr.c \
	xrandr.h \
	vidmode.c \
	vidmode.h

libx11ui_a_SOURCES = \
	c128icon.c \
	c64icon.c \
	c64dtvicon.c \
	cbm2icon.c \
	focus.c \
	focus.h \
	fullscreen.c \
	peticon.c \
	plus4icon.c \
	qnxipc.h \
	qnxshm.c \
	qnxshm.h \
	renderxv.c \
	renderxv.h \
	scpu64icon.c \
	ui-cmdline-options.c \
	ui-resources.c \
	vic20icon.c \
	x11menu.h \
	x11ui.h

libx11ui_a_DEPENDENCIES = @FULLSCREEN_DRIVERS@ @VICE_SYNC@

libx11ui_a_LIBADD = @FULLSCREEN_DRIVERS@ @VICE_SYNC@

.PHONY: xaw gnome
xaw:
	(cd xaw; $(MAKE))
gnome:
	(cd gnome; $(MAKE))
