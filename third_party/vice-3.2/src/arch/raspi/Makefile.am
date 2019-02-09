# Makefile.am for the Unix version of `libarch.a'.

SUBDIRS = 

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/arch/unix/readline \
	-I$(top_srcdir)/src/rs232drv \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c64dtv \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/platform \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libarch.a

libarch_a_DEPENDENCIES = @VICE_UI_THREADS@
libarch_a_LIBADD = @VICE_UI_THREADS@

libarch_a_SOURCES = \
	archdep.h \
	archdep.c \
	joy.h \
	joy.c \
	kbd.h \
	kbd.c \
	mousedrv.h \
	mousedrv.c \
	missing.c \
	ui.h \
	ui.c \
	videoarch.h \
	videoarch.c \
	menu.h \
	menu.c \
	menu_usb.h \
	menu_usb.c \
	text.h \
	text.c
