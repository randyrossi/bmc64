# Makefile.am for the Mac OS X library `libarchmacosxcocoauiview.a'.

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/vdrive \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libarchmacosxcocoaview.a

noinst_HEADERS = \
	consolewindow.h \
	controlwindow.h \
	driveview.h \
	fullscreenwindow.h \
	joystickview.h \
	keypressview.h \
	logview.h \
	soundview.h \
	statusview.h \
	tapeview.h \
	viceglview.h \
	vicewindow.h
   
libarchmacosxcocoaview_a_SOURCES = \
	consolewindow.m \
	controlwindow.m \
	driveview.m \
	fullscreenwindow.m \
	joystickview.m \
	logview.m \
	keypressview.m \
	soundview.m \
	statusview.m \
	tapeview.m \
	viceglview.m \
	vicewindow.m
