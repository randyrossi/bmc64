# Makefile.am for the Mac OS X library `libarchmacosxcocoa.a'.

SUBDIRS = view menu dialog

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/vdrive \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libarchmacosxcocoa.a

noinst_HEADERS = \
	kbd.h \
	viceapplication.h \
	viceapplicationprotocol.h \
	vicemachine.h \
	vicemachinenotifier.h \
	vicemachineprotocol.h \
	vicenotifications.h \
	videoarch.h \
	videoparam.h
   
libarchmacosxcocoa_a_SOURCES = \
	ui-resources.c \
	ui-cmdline-options.c \
	console.m \
	fullscreen.m \
	kbd.m \
	main.m \
	mousedrv.m \
	ui.m \
	uimon.m \
	viceapplication.m \
	vicemachine.m \
	vicemachinenotifier.m \
	video.m \
	vsidui.m
