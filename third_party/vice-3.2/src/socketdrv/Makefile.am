# Makefile.am for `libsocketdrv.a'.

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libsocketdrv.a

libsocketdrv_a_SOURCES = \
	socketdrv.c

EXTRA_DIST = \
	socket-amiga-drv.c \
	socket-amiga-impl.h \
	socket-beos-impl.h \
	socket-dos-impl.h \
	socket-unix-impl.h \
	socket-win32-drv.c \
	socket-win32-impl.h \
	socketimpl.h
