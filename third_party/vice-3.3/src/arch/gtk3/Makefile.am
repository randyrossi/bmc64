

- x128: Bring either VDC or VICII display to front depending on 40/80 col key## Makefile.am for the native GTK3 version of `libarch.a'.

if WIN32_COMPILE
extra_includes = -I$(top_srcdir)/src/arch/mingw32-pcap/wpcap
else
extra_includes =
endif

SUBDIRS = data novte widgets

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	$(extra_includes) \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/crtc \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/rs232drv \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/vdc \
	-I$(top_srcdir)/src/arch/gtk3/widgets \
	-I$(top_srcdir)/src/arch/gtk3/widgets/base \
	-I$(top_srcdir)/src/arch/gtk3/novte \
	-I$(top_builddir)/src/arch/gtk3/novte \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c64dtv \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/plus4 \
	-I$(top_srcdir)/src/scpu64 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/samplerdrv \
	-I$(top_srcdir)/src/c64/cart \
	-I$(top_srcdir)/src/imagecontents \
	-I$(top_srcdir)/src/monitor \
	-I$(top_srcdir)/src/tapeport \
	-I$(top_srcdir)/src/hvsc


noinst_LIBRARIES = libarch.a

libarch_a_SOURCES = \
	archdep.c \
	blockdev.c \
	c128ui.c \
	c64dtvui.c \
	c64scui.c \
	c64ui.c \
	cairo_renderer.c \
	cbm2ui.c \
	cbm5x0ui.c \
	coproc.c \
	dynlib.c \
	fullscreen.c \
	gtk3main.c \
	joy-osx-hid.c \
	joy-osx-hidmgr.c \
	joy-osx-hidutil.c \
	joy-osx.c \
	joy-unix-usb.c \
	joy-unix.c \
	joy-win32-dinput-handle.c \
	joy-win32.c \
	kbd.c \
	linenoise.c \
	mousedrv.c \
	opengl_renderer.c \
	petui.c \
	plus4ui.c \
	quartz_renderer.c \
	rawnetarch.c \
	rs232dev.c \
	scpu64ui.c \
	signals.c \
	ui.c \
	uiabout.c \
	uicart.c \
	uicmdline.c \
	uicommands.c \
	uicompiletimefeatures.c \
	uidata.c \
	uidatasette.c \
	uidebug.c \
	uidiskattach.c \
	uidiskcreate.c \
	uiedit.c \
	uifliplist.c \
	uimachinemenu.c \
	uimachinewindow.c \
	uimedia.c \
	uimenu.c \
	uimon-fallback.c \
	uimon.c \
	uinetplay.c \
	uinetplay_new.c \
	uisettings.c \
	uisidattach.c \
	uismartattach.c \
	uisnapshot.c \
	uistatusbar.c \
	uitapeattach.c \
	uitapecreate.c \
	uivideo.c \
	uivsidmenu.c \
	uivsidwindow.c \
	vic20ui.c \
	video.c \
	vsidui.c \
	vsyncarch.c

EXTRA_DIST = \
	archdep.h \
	archdep_unix.c \
	archdep_unix.h \
	archdep_win32.c \
	archdep_win32.h \
	cairo_renderer.h \
	coproc.h \
	debug_gtk3.h \
	dynlib-unix.c \
	dynlib-win32.c \
	joy-osx-hid.h \
	joy-osx-hidlib.h \
	joy-osx.h \
	joy-unix.h \
	joy-win32-dinput-handle.h \
	joy-win32.h \
	joy.h \
	kbd.h \
	linenoise.h \
	make-bindist_osx.sh \
	make-bindist_win32.sh \
	mousedrv.h \
	opengl_renderer.h \
	quartz_renderer.h \
	rawnetarch_unix.c \
	rawnetarch_win32.c \
	rs232-unix-dev.c \
	rs232-win32-dev.c \
	ui.h \
	uiabout.h \
	uicart.h \
	uicmdline.h \
	uicommands.h \
	uicompiletimefeatures.h \
	uidata.h \
	uidatasette.h \
	uidebug.h \
	uidiskattach.h \
	uidiskcreate.h \
	uiedit.h \
	uifliplist.h \
	uimachinemenu.h \
	uimachinewindow.h \
	uimedia.h \
	uimenu.h \
	uimonarch.h \
	uimon-fallback.h \
	uinetplay.h \
	uinetplay_new.h \
	uisettings.h \
	uisidattach.h \
	uismartattach.h \
	uisnapshot.h \
	uistatusbar.h \
	uitapeattach.h \
	uitapecreate.h \
	uivideo.h \
	uivsidmenu.h \
	uivsidwindow.h \
	videoarch.h \
	winjoy.h \
	Info.plist \
	Resources/VICE.icns \
	vice-launcher.sh \
	x11-launcher.sh
