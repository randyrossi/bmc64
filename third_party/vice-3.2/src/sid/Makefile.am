if HAVE_RESID
resid_libadd = resid.o
else
resid_libadd =
endif

SUFFIXES = .def

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	@RESID_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/joyport

noinst_LIBRARIES = libsid.a

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

EXTRA_DIST = resid-dtv.cc

libsid_a_LIBADD = $(resid_libadd)

siddir = $(top_srcdir)/src/sid
