# Makefile.am for `libmididrv.a'.

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libmididrv.a

libmididrv_a_SOURCES = \
	midi-osx-drv.c \
	midi-unix-drv.c \
	midi-win32-drv.c
