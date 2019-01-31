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
	-I$(top_srcdir)/src/vicii \
	-I$(top_srcdir)/src/video \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/vdc \
	-I$(top_srcdir)/src/rtc \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/samplerdrv \
	-I$(top_srcdir)/src/tapeport \
	-I$(top_srcdir)/src/core

noinst_LIBRARIES = libc128.a

libc128_a_SOURCES = \
	c128-cmdline-options.c \
	c128-cmdline-options.h \
	c128-resources.c \
	c128-resources.h \
	c128-snapshot.c \
	c128-snapshot.h \
	c128.c \
	c128.h \
	c128cia1.c \
	c128cpu.c \
	c128drive.c \
	c128embedded.c \
	c128fastiec.c \
	c128fastiec.h \
	c128mem.c \
	c128mem.h \
	c128meminit.c \
	c128meminit.h \
	c128memlimit.c \
	c128memlimit.h \
	c128memrom.c \
	c128memrom.h \
	c128memsnapshot.c \
	c128memsnapshot.h \
	c128mmu.c \
	c128mmu.h \
	c128model.c \
	c128model.h \
	c128rom.c \
	c128rom.h \
	c128romset.c \
	c128video.c \
	functionrom.c \
	functionrom.h \
	z80.c \
	z80.h \
	z80mem.c \
	z80mem.h
