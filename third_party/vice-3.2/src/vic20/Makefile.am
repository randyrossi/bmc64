SUBDIRS = cart

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/tape \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/video \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/vic20/cart \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/samplerdrv \
	-I$(top_srcdir)/src/tapeport \
	-I$(top_srcdir)/src/core

noinst_LIBRARIES = libvic20.a

libvic20_a_SOURCES = \
	vic-cmdline-options.c \
	vic-cmdline-options.h \
	vic-color.c \
	vic-color.h \
	vic-cycle.c \
	vic-cycle.h \
	vic-draw.c \
	vic-draw.h \
	vic-mem.c \
	vic-mem.h \
	vic-resources.c \
	vic-resources.h \
	vic-snapshot.c \
	vic-snapshot.h \
	vic-timing.c \
	vic-timing.h \
	vic.c \
	vic.h \
	victypes.h \
	vic20-cmdline-options.c \
	vic20-cmdline-options.h \
	vic20-resources.c \
	vic20-resources.h \
	vic20-snapshot.c \
	vic20-snapshot.h \
	vic20.c \
	vic20.h \
	vic20bus.c \
	vic20cpu.c \
	vic20datasette.c \
	vic20drive.c \
	vic20embedded.c \
	vic20export.c \
	vic20iec.c \
	vic20iec.h \
	vic20ieeevia.h \
	vic20ieeevia1.c \
	vic20ieeevia2.c \
	vic20io.c \
	vic20mem.c \
	vic20mem.h \
	vic20memrom.h \
	vic20memrom.c \
	vic20memsnapshot.c \
	vic20memsnapshot.h \
	vic20model.c \
	vic20model.h \
	vic20printer.c \
	vic20rom.c \
	vic20rom.h \
	vic20romset.c \
	vic20rsuser.c \
	vic20rsuser.h \
	vic20sound.c \
	vic20sound.h \
	vic20via.h \
	vic20via1.c \
	vic20via2.c \
	vic20video.c

.PHONY: libvic20cart libmascuerade

libvic20cart:
	(cd cart; $(MAKE) libvic20cart)

libmascuerade:
	(cd cart; $(MAKE) libmascuerade)
