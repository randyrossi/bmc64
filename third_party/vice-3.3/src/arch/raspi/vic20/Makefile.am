# Makefile.am for the raspi version of `libarch_vic20.a'.

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	$(extra_includes) \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c64/cart \
	-I$(top_srcdir)/src/c64dtv \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/plus4 \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/rs232drv \
	-I$(top_srcdir)/src/samplerdrv \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/vdrive \
	-I$(top_srcdir)/src/video \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/platform \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/gfxoutputdrv \
	-I$(top_srcdir)/src/tapeport \
	-I$(top_srcdir)/src/imagecontents \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libarch_vic20.a

libarch_vic20_a_SOURCES = \
	archdep_vic20.h \
	archdep_vic20.c \
	videoarch_vic20.h \
	videoarch_vic20.c \
	virtual_keyboard_vic20.h \
	virtual_keyboard_vic20.c \
	menu_vic20.h \
	menu_vic20.c \
	userport_vic20.h \
	userport_vic20.c
