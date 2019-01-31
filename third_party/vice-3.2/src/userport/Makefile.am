AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/c64/cart \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/rtc \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/samplerdrv \
	-I$(top_srcdir)/src/diag

noinst_LIBRARIES = libuserport.a

libuserport_a_SOURCES = \
	userport.c \
	userport.h \
	userport_4bit_sampler.c \
	userport_4bit_sampler.h \
	userport_8bss.c \
	userport_8bss.h \
	userport_dac.c \
	userport_dac.h \
	userport_diag_586220_harness.c \
	userport_diag_586220_harness.h \
	userport_digimax.c \
	userport_digimax.h \
	userport_joystick.c \
	userport_joystick.h \
	userport_rtc_58321a.c \
	userport_rtc_58321a.h \
	userport_rtc_ds1307.c \
	userport_rtc_ds1307.h
