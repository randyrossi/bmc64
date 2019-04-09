if HAVE_RESID
resid_libadd = resid.o
resid_dtv_libadd = resid-dtv.o
else
resid_libadd =
resid_dtv_libadd =
endif

SUFFIXES = .def

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	@RESID_INCLUDES@ \
	@RESID_DTV_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/joyport

noinst_LIBRARIES = libsid.a libsid_dtv.a

libsid_a_SOURCES = \
	catweaselmkiii.c \
	fastsid.c \
	fastsid.h \
	hardsid.c \
	parsid.c \
	sid-cmdline-options.c \
	sid-cmdline-options.h \
	sid-resources.c \
	sid-resources.h \
	sid-snapshot.c \
	sid-snapshot.h \
	sid.c \
	sid.h \
	ssi2001.c \
	wave6581.h \
	wave8580.h

EXTRA_libsid_a_SOURCES = \
	resid.cc \
	resid.h

libsid_a_LIBADD = $(resid_libadd)

siddir = $(top_srcdir)/src/sid

libsid_dtv_a_SOURCES = $(libsid_a_SOURCES)

EXTRA_libsid_dtv_a_SOURCES = \
	resid-dtv.cc \
	resid.h

libsid_dtv_a_LIBADD = $(resid_dtv_libadd)

sid_dtvdir = $(top_srcdir)/src/sid
