AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c64/cart \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/tape \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/viciisc \
	-I$(top_srcdir)/src/video \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/vdc \
	-I$(top_srcdir)/src/rtc \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/samplerdrv \
	-I$(top_srcdir)/src/tapeport

if SUPPORT_XSCPU64
noinst_LIBRARIES = libscpu64.a
else
noinst_LIBRARIES =
endif

libscpu64_a_SOURCES = \
	scpu64-cmdline-options.c \
	scpu64-cmdline-options.h \
	scpu64-resources.c \
	scpu64-resources.h \
	scpu64-snapshot.c \
	scpu64-snapshot.h \
	scpu64.c \
	scpu64.h \
	scpu64cpu.c \
	scpu64cpu.h \
	scpu64gluelogic.c \
	scpu64gluelogic.h \
	scpu64mem.c \
	scpu64mem.h \
	scpu64memsnapshot.c \
	scpu64memsnapshot.h \
	scpu64meminit.c \
	scpu64meminit.h \
	scpu64model.c \
	scpu64rom.c \
	scpu64rom.h \
	scpu64stubs.c

EXTRA_DIST =
