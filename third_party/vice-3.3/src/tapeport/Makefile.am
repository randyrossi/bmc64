AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/rtc \
	-I$(top_srcdir)/src/diag

noinst_LIBRARIES = libtapeport.a

libtapeport_a_SOURCES = \
	cp-clockf83.c \
	cp-clockf83.h \
	dtl-basic-dongle.c \
	dtl-basic-dongle.h \
	sense-dongle.c \
	sense-dongle.h \
	tape_diag_586220_harness.c \
	tape_diag_586220_harness.h \
	tapecart.c \
	tapecart.h \
	tapecart-loader.h \
	tapelog.c \
	tapelog.h \
	tapeport.c \
	tapeport.h
