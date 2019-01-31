# Makefile.am for `libgfxoutputdrv.a'.

AM_CPPFLAGS = \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	@FFMPEG_INCLUDES@ \
	@QUICKTIME_INCLUDES@ \
	@ARCH_INCLUDES@

noinst_LIBRARIES = libgfxoutputdrv.a

# These sources are linked only under certain circumstances.
EXTRA_libgfxoutputdrv_a_SOURCES = \
	ffmpegdrv.c \
	ffmpegdrv.h \
	ffmpeglib.c \
	ffmpeglib.h \
	gifdrv.c \
	gifdrv.h \
	jpegdrv.c \
	jpegdrv.h \
	pngdrv.c \
	pngdrv.h \
	quicktimedrv.h \
	quicktimedrv.c

# These sources are always built.
libgfxoutputdrv_a_SOURCES = \
	bmpdrv.c \
	bmpdrv.h \
	doodledrv.c \
	gfxoutput.c \
	godotdrv.c \
	godotdrv.h \
	iffdrv.c \
	iffdrv.h \
	koaladrv.c \
	nativedrv.c \
	nativedrv.h \
	pcxdrv.c \
	pcxdrv.h \
	ppmdrv.c \
	ppmdrv.h

libgfxoutputdrv_a_DEPENDENCIES = @GFXOUTPUT_DRIVERS@
libgfxoutputdrv_a_LIBADD = @GFXOUTPUT_DRIVERS@

