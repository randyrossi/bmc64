# Makefile.am for the AMIGAOS version of `libarch.a'.

if DUMMY_COMPILE
SUBDIRS = info-files
else
SUBDIRS = mui
endif

bin_PROGRAMS = x64ns x64dtvns x64scns x128ns xvicns xplus4ns xcbm2ns xcbm5x0ns xpetns xscpu64ns

x64ns_SOURCES = x64ns.c

x64dtvns_SOURCES = x64dtvns.c

x64scns_SOURCES = x64scns.c

xscpu64ns_SOURCES = xscpu64ns.c

x128ns_SOURCES = x128ns.c

xvicns_SOURCES = xvicns.c

xplus4ns_SOURCES = xplus4ns.c

xcbm2ns_SOURCES = xcbm2ns.c

xcbm5x0ns_SOURCES = xcbm5x0ns.c

xpetns_SOURCES = xpetns.c

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_builddir)/src/arch/amigaos \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c64dtv \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/plus4 \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/vdrive \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/imagecontents \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/platform \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libarch.a

libarch_a_SOURCES = \
	ahi.c \
	ahi.h \
	amigamain.c \
	archdep.c \
	archdep.h \
	blockdev.c \
	c128ui.c \
	c128uires.h \
	c64ui.c \
	c64uires.h \
	c64dtvui.c \
	c64dtvuires.h \
	cbm2ui.c \
	cbm2uires.h \
	cbm5x0ui.c \
	cbm5x0uires.h \
	console.c \
	fullscreen.c \
	fullscreenarch.h \
	intl.c \
	intl_funcs.h \
	joy.c \
	joy.h \
	joyai.c \
	joyai.h \
	joyll.c \
	joyll.h \
	kbd.c \
	kbd.h \
	loadlibs.c \
	loadlibs.h \
	memory.h \
	mousedrv.c \
	mousedrv.h \
	noinlineargs.c \
	petui.c \
	petuires.h \
	plus4ui.c \
	plus4uires.h \
	pointer.c \
	pointer.h \
	private.h \
	rs232.c \
	scpu64ui.c \
	scpu64uires.h \
	screen-shot.c \
	screen-shot.h \
	signals.c \
	statusbar.c \
	statusbar.h \
	rawnetarch.c \
	timer.c \
	timer.h \
	ui.c \
	ui.h \
	uiapi.c \
	uiattach.c \
	uiattach.h \
	uic64cart.c \
	uic64cart.h \
	uicart.c \
	uicart.h \
	uicolor.c \
	uilib.c \
	uilib.h \
	uimon.c \
	uires.h \
	vic20ui.c \
	vic20uires.h \
	video.c \
	videoarch.h \
	vsidui.c \
	vsiduires.h \
	vsyncapi.c

EXTRA_DIST = \
	intl_text.c \
	fixdeps.c \
	genamigaintl.c \
	genintl_h.sh \
	genintltable.sh \
	intl.txt \
	make-bindist.sh

BUILT_SOURCES = intl.h intl_table.h

if NATIVE_TOOLS
genamigaintl:	$(srcdir)/intl.txt $(srcdir)/genamigaintl.c
	$(NATIVE_TOOLS_COMPILER) -o genamigaintl $(srcdir)/genamigaintl.c

intl.h intl_table.h:	$(srcdir)/intl.txt genamigaintl
	echo "generating intl.h and intl_table.h..."
	./genamigaintl $(srcdir)/intl.txt intl.h intl_table.h
else
intl.h: $(srcdir)/intl.txt
	echo "generating intl.h..."
	$(SHELL) $(srcdir)/genintl_h.sh <$(srcdir)/intl.txt >intl.h

intl_table.h: $(srcdir)/intl.txt
	echo "generating intl_table.h..."
	$(SHELL) $(srcdir)/genintltable.sh $(DEBUGBUILD) <$(srcdir)/intl.txt >intl_table.h
endif
