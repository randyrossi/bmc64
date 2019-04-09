AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/c64/cart \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/lib/p64

noinst_LIBRARIES = libvic20cart.a libmascuerade.a

libvic20cart_a_SOURCES = \
	behrbonz.c \
	behrbonz.h \
	debugcart.c \
	debugcart.h \
	finalexpansion.c \
	finalexpansion.h \
	ioramcart.c \
	ioramcart.h \
	megacart.c \
	megacart.h \
	ultimem.c \
	ultimem.h \
	vic-fp.c \
	vic-fp.h \
	vic20-generic.c \
	vic20-generic.h \
	vic20-ieee488.c \
	vic20-ieee488.h \
	vic20-midi.c \
	vic20-midi.h \
	vic20-sidcart.c \
	vic20cart.c \
	vic20cart.h \
	vic20cartmem.c \
	vic20cartmem.h

libmascuerade_a_SOURCES = \
	mascuerade-stubs.h \
	mascuerade-stubs.c
