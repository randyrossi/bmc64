
# Mmmh...  This is a real hack.

DEFS = @DEFS@ -DANSI_ARROWS -DHAVE_TCGETATTR -DHIDE -DUSE_DIRENT -DSYS_UNIX

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libreadline.a

libreadline_a_SOURCES = \
	editline.c \
	editline.h \
	unix.h \
	complete.c \
	editline2.c \
	sysunix.c \
	version.c

EXTRA_DIST = README testit.c
