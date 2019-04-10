# Makefile.am for the SDL version of `libarch.a'.

if WIN32_COMPILE
extra_includes = -I$(top_srcdir)/src/arch/mingw32-pcap/wpcap
else
extra_includes =
endif
 
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

noinst_LIBRARIES = libarch.a

#libarch_a_VERDEP_SOURCES = \
#	video_sdl1.c

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
	text.c \
	demo.h \
	demo.c
