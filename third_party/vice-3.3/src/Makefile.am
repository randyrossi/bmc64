# We have to override the automake default, because we need to use
# $(CXX) instead of $(CC) when compiling with reSID.
LINK = @LINKCC@ $(CFLAGS) $(LDFLAGS) -o $@
CCLD = @LINKCC@

ARCH_LIBS = @ARCH_LIBS@

if DUMMY_COMPILE
SUBDIRS = buildtools

else

SUBDIRS = \
	@RESID_DIR@ \
	@RESID_DTV_DIR@ \
	samplerdrv \
	joyport \
	sounddrv \
	mididrv \
	socketdrv \
	hwsiddrv \
	iodrv \
	drive \
	vdrive \
	fsdevice \
	diskimage \
	iecbus \
	serial \
	parallel \
	tape \
	imagecontents \
	fileio \
	video \
	raster \
	vicii \
	viciisc \
	viciivsid \
	vdc \
	crtc \
	gfxoutputdrv \
	printerdrv \
	rs232drv \
	sid \
	monitor \
	core \
	rtc \
	userport \
	tapeport \
	diag \
	c64 \
	scpu64 \
	c64dtv \
	c128 \
	vic20 \
	pet \
	plus4 \
	cbm2 \
	arch/shared \
	arch \
	lib \
	hvsc

endif

DIST_SUBDIRS = \
	sounddrv \
	mididrv \
	socketdrv \
	hwsiddrv \
	iodrv \
	samplerdrv \
	joyport \
	drive \
	vdrive \
	fsdevice \
	diskimage \
	iecbus \
	serial \
	parallel \
	tape \
	imagecontents \
	fileio \
	video \
	raster \
	vicii \
	viciisc \
	viciivsid \
	vdc \
	crtc \
	gfxoutputdrv \
	printerdrv \
	rs232drv \
	sid \
	monitor \
	core \
	rtc \
	diag \
	userport \
	tapeport \
	c64 \
	scpu64 \
	c64dtv \
	c128 \
	vic20 \
	pet \
	plus4 \
	cbm2 \
	arch \
	@RESIDSUB@ \
	@RESIDDTVSUB@ \
	lib \
	buildtools \
	hvsc

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/arch/unix/readline \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/monitor \
	-I$(top_srcdir)/src/plus4 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/vdrive \
	-I$(top_srcdir)/src/diskimage \
	-I$(top_srcdir)/src/imagecontents \
	-I$(top_srcdir)/src/diag \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/rtc \
	-I$(top_srcdir)/src/tapeport \
	-I$(top_srcdir)/src/tape \
	-I$(top_srcdir)/src/socketdrv \
	-I$(top_srcdir)/src/hvsc

noinst_HEADERS = \
	6510core.h \
	acia.h \
	alarm.h \
	archapi.h \
	attach.h \
	autostart.h \
	autostart-prg.h \
	blockdev.h \
	c128ui.h \
	c64ui.h \
	cartio.h \
	cartridge.h \
	catweaselmkiii.h \
	cbm2ui.h \
	cbmdos.h \
	cbmimage.h \
	charset.h \
	cia.h \
	clkguard.h \
	clipboard.h \
	cmdline.h \
	color.h \
	config.h.in \
	console.h \
	crc32.h \
	datasette.h \
	debug.h \
	digimaxcore.c \
	diskconstants.h \
	diskimage.h \
	dma.h \
	dynlib.h \
	embedded.h \
	export.h \
	fileio.h \
	findpath.h \
	fixpoint.h \
	fsdevice.h \
	flash040.h \
	fliplist.h \
	fullscreen.h \
	gcr.h \
	gfxoutput.h \
	h6809regs.h \
	hardsid.h \
	iecbus.h \
	iecdrive.h \
	imagecontents.h \
	info.h \
	init.h \
	initcmdline.h \
	interrupt.h \
	ioutil.h \
	kbdbuf.h \
	keyboard.h \
	lib.h \
	libm_math.h \
	log.h \
	machine-bus.h \
	machine-drive.h \
	machine-printer.h \
	machine-video.h \
	machine.h \
	main.h \
	mainc64cpu.c \
	mainc64cpu.h \
	main65816cpu.c \
	main65816cpu.h \
	maincpu.h \
	mainviccpu.c \
	mem.h \
	midi.h \
	mididrv.h \
	monitor.h \
	mos6510.h \
	mos6510dtv.h \
	network.h \
	opencbm.h \
	opencbmlib.h \
	palette.h \
	parallel.h \
	parsid.h \
	petui.h \
	piacore.h \
	plus4ui.h \
	printer.h \
	ps2mouse.h \
	r65c02.h \
	ram.h \
	rawfile.h \
	rawnet.h \
	rawnetarch.h \
	resources.h \
	riot.h \
	romset.h \
	rs232dev.h \
	rs232drv.h \
	rs232net.h \
	rsuser.h \
	scpu64ui.h \
	screenshot.h \
	serial.h \
	sidcart.h \
	signals.h \
	snapshot.h \
	sound.h \
	ssi2001.h \
	sysfile.h \
	tap.h \
	tape.h \
	tpi.h \
	traps.h \
	types.h \
	uiapi.h \
	uicmdline.h \
	uicolor.h \
	uimon.h \
	usleep.h \
	util.h \
	version.h.in \
	version.h \
	via.h \
	vic20ui.h \
	vice.h \
	vice_sdl.h \
	vice-event.h \
	vicemaxpath.h \
	vicesocket.h \
	vicefeatures.h \
	vicii.h \
	video.h \
	viewport.h \
	vsidui.h \
	vsync.h \
	vsyncapi.h \
	wdc65816.h \
	z80regs.h \
	zfile.h \
	zipcode.h

base_sources = \
	alarm.c \
	attach.c \
	autostart.c \
	autostart-prg.c \
	cbmdos.c \
	cbmimage.c \
	charset.c \
	clipboard.c \
	clkguard.c \
	cmdline.c \
	color.c \
	crc32.c \
	datasette.c \
	debug.c \
	dma.c \
	embedded.c \
	event.c \
	findpath.c \
	fliplist.c \
	gcr.c \
	info.c \
	init.c \
	initcmdline.c \
	interrupt.c \
	ioutil.c \
	kbdbuf.c \
	keyboard.c \
	lib.c \
	libm_math.c \
	log.c \
	machine-bus.c \
	machine.c \
	main.c \
	network.c \
	opencbmlib.c \
	palette.c \
	ram.c \
	rawfile.c \
	rawnet.c \
	resources.c \
	romset.c \
	screenshot.c \
	snapshot.c \
	socket.c \
	sound.c \
	sysfile.c \
	traps.c \
	util.c \
	vicefeatures.c \
	vsync.c \
	zfile.c \
	zipcode.c

ps2mouse_sources = \
	ps2mouse.c

midi_sources = \
	midi.c

cpucore_sources = \
	6510core.c \
	65c02core.c \
	65816core.c

cpudtvcore_sources = \
	6510dtvcore.c

COMMON_EXTRA_DIST = \
	6510core.c \
	65816core.c \
	65c02core.c \
	6510dtvcore.c \
	ChangeLog \
	ChangeLog.1 \
	ChangeLog.2 \
	ChangeLog.3 \
	ChangeLog.4 \
	ChangeLog.5 \
	ChangeLog.6 \
	aciacore.c \
	debug.h.in \
	fixpoint.c \
	piacore.c \
	sedinfocontrib_h.sh \
	vice-version.sh \
	vice-version.sh.in \
	wrap-u-ar.sh

# RESID_EXTRA_DIST is a list of files in the resid directory that need to be
# included in the source archive, this list was added to fix the fact that
# 'make dist' only produced a proper/workable source archive when done on a
# platform with a c++ compiler and resid enabled.

RESID_EXTRA_DIST = \
	resid/aclocal.m4 \
	resid/AUTHORS \
	resid/ChangeLog \
	resid/configure \
	resid/configure.in \
	resid/COPYING \
	resid/dac.cc \
	resid/dac.h \
	resid/envelope.cc \
	resid/envelope.h \
	resid/extfilt.cc \
	resid/extfilt.h \
	resid/filter.cc \
	resid/filter.h \
	resid/INSTALL \
	resid/Makefile.am \
	resid/Makefile.in \
	resid/NEWS \
	resid/pot.cc \
	resid/pot.h \
	resid/README \
	resid/README.VICE \
	resid/resid-config.h \
	resid/samp2src.pl \
	resid/sid.cc \
	resid/sid.h \
	resid/siddefs.h.in \
	resid/spline.h \
	resid/THANKS \
	resid/TODO \
	resid/version.cc \
	resid/voice.cc \
	resid/voice.h \
	resid/wave.cc \
	resid/wave.h \
	resid/wave6581__ST.dat \
	resid/wave6581__ST.h \
	resid/wave6581_P_T.dat \
	resid/wave6581_P_T.h \
	resid/wave6581_PS_.dat \
	resid/wave6581_PS_.h \
	resid/wave6581_PST.dat \
	resid/wave6581_PST.h \
	resid/wave8580__ST.dat \
	resid/wave8580__ST.h \
	resid/wave8580_P_T.dat \
	resid/wave8580_P_T.h \
	resid/wave8580_PS_.dat \
	resid/wave8580_PS_.h \
	resid/wave8580_PST.dat \
	resid/wave8580_PST.h

# RESIDDTV_EXTRA_DIST is a list of files in the resid directory that need to be
# included in the source archive, this list was added to fix the fact that
# 'make dist' only produced a proper/workable source archive when done on a
# platform with a c++ compiler and resid enabled.

RESIDDTV_EXTRA_DIST = \
	resid-dtv/aclocal.m4 \
	resid-dtv/AUTHORS \
	resid-dtv/bittrain.h \
	resid-dtv/ChangeLog \
	resid-dtv/configure \
	resid-dtv/configure.in \
	resid-dtv/COPYING \
	resid-dtv/envelope.cc \
	resid-dtv/envelope.h \
	resid-dtv/extfilt.cc \
	resid-dtv/extfilt.h \
	resid-dtv/filter.cc \
	resid-dtv/filter.h \
	resid-dtv/INSTALL \
	resid-dtv/Makefile.am \
	resid-dtv/Makefile.in \
	resid-dtv/NEWS \
	resid-dtv/README \
	resid-dtv/residdtv-config.h \
	resid-dtv/sid.cc \
	resid-dtv/sid.h \
	resid-dtv/siddtvdefs.h.in \
	resid-dtv/version.cc \
	resid-dtv/voice.cc \
	resid-dtv/voice.h \
	resid-dtv/wave.cc \
	resid-dtv/wave.h

if RESID_DIR_USED

EXTRA_DIST = $(COMMON_EXTRA_DIST)

else

EXTRA_DIST = $(COMMON_EXTRA_DIST) $(RESID_EXTRA_DIST) $(RESIDDTV_EXTRA_DIST)

endif

if NATIVE_TOOLS
GENFILES = $(builddir)/geninfocontrib bin2c palette2c
endif

if EMBEDDED
EMBEDDED_H_FILES = \
	c128basic64.h \
	c128basichi.h \
	c128basiclo.h \
	c128chargde.h \
	c128chargen.h \
	c128chargfr.h \
	c128chargse.h \
	c128chargch.h \
	c128kernal.h \
	c128kernal64.h \
	c128kernalde.h \
	c128kernalfi.h \
	c128kernalfr.h \
	c128kernalit.h \
	c128kernalno.h \
	c128kernalse.h \
	c128kernalch.h \
	c64basic.h \
	c64kernal.h \
	c64chargen.h \
	cbm2basic128.h \
	cbm2basic256.h \
	cbm2basic500.h \
	cbm2chargen500.h \
	cbm2chargen600.h \
	cbm2chargen700.h \
	cbm2kernal.h \
	cbm2kernal500.h \
	drived1541II.h \
	drived1571cr.h \
	drivedos1001.h \
	drivedos1540.h \
	drivedos1541.h \
	drivedos1551.h \
	drivedos1570.h \
	drivedos1571.h \
	drivedos1581.h \
	drivedos2031.h \
	drivedos2040.h \
	drivedos3040.h \
	drivedos4040.h \
	petbasic1.h \
	petbasic2.h \
	petbasic4.h \
	petchargen.h \
	petedit1g.h \
	petedit2b.h \
	petedit2g.h \
	petedit4b40.h \
	petedit4b80.h \
	petedit4g40.h \
	petkernal1.h \
	petkernal2.h \
	petkernal4.h \
	superpet_char.h \
	superpet_waterloo_a000.h \
	superpet_waterloo_b000.h \
	superpet_waterloo_c000.h \
	superpet_waterloo_d000.h \
	superpet_waterloo_e000.h \
	superpet_waterloo_f000.h \
	plus43plus1hi.h \
	plus43plus1lo.h \
	plus4basic.h \
	plus4kernal.h \
	printermps803.h \
	printernl10cbm.h \
	vic20basic.h \
	vic20chargen.h \
	vic20kernal.h \
	vdc_comp_vpl.h \
	vdc_deft_vpl.h \
	vicii_c64hq_vpl.h \
	vicii_c64s_vpl.h \
	vicii_ccs64_vpl.h \
	vicii_community_colors_vpl.h \
	vicii_deekay_vpl.h \
	vicii_frodo_vpl.h \
	vicii_godot_vpl.h \
	vicii_pc64_vpl.h \
	vicii_pepto_ntsc_vpl.h \
	vicii_pepto_ntsc_sony_vpl.h \
	vicii_pepto_pal_vpl.h \
	vicii_pepto_palold_vpl.h \
	vicii_ptoing_vpl.h \
	vicii_rgb_vpl.h \
	vicii_vice_vpl.h \
	viciidtv_spiff_vpl.h \
	crtc_amber_vpl.h \
	crtc_green_vpl.h \
	crtc_white_vpl.h \
	ted_yape_pal_vpl.h \
	ted_yape_ntsc_vpl.h \
	ted_colodore_ted_vpl.h \
	vic_mike_ntsc_vpl.h \
	vic_mike_pal_vpl.h \
	vic_vice_vpl.h

if NATIVE_TOOLS
EMBEDDED_DEPENDS = bin2c $(srcdir)/buildtools/bin2c.sh palette2c $(srcdir)/buildtools/palette2c.sh
else
EMBEDDED_DEPENDS = $(srcdir)/buildtools/bin2c.sh $(srcdir)/buildtools/palette2c.sh
endif

else
EMBEDDED_H_FILES =
endif

if OWCC
OWCC_LIBS = owcc_libs_converted.h
else
OWCC_LIBS =
endif

if USE_SVN_REVISION
SVN_VERSION_HEADER = svnversion.h
else
SVN_VERSION_HEADER =
endif

BUILT_SOURCES = $(EMBEDDED_H_FILES) $(OWCC_LIBS) $(SVN_VERSION_HEADER) infocontrib.h

.PHONY: svnversion.h.indirect

svnversion.h.indirect:
	$(SHELL) $(srcdir)/buildtools/gensvnversion.sh $(SVNVERSION) $(top_srcdir) $(srcdir)/svnversion.h

svnversion.h: svnversion.h.indirect

if NATIVE_TOOLS
$(builddir)/geninfocontrib:	$(top_srcdir)/doc/vice.texi $(srcdir)/buildtools/infocontrib.sed $(srcdir)/buildtools/geninfocontrib.c
	$(NATIVE_TOOLS_COMPILER) -o $(builddir)/geninfocontrib $(srcdir)/buildtools/geninfocontrib.c

infocontrib.h:	$(top_srcdir)/doc/vice.texi $(srcdir)/buildtools/infocontrib.sed $(builddir)/geninfocontrib
	@echo "generating infocontrib.h..."
	$(builddir)/geninfocontrib $(top_srcdir)/ ../ doc/vice.texi src/infocontrib.h src/buildtools/infocontrib.sed AUTHORS README doc/html/index.html man/vice.1
else
infocontrib.h:	$(top_srcdir)/doc/vice.texi $(srcdir)/buildtools/infocontrib.sed
	@echo "generating infocontrib.h..."
	$(SHELL) $(srcdir)/buildtools/geninfocontrib_h.sh infocontrib.h <$(top_srcdir)/doc/vice.texi | $(srcdir)/sedinfocontrib_h.sh $(srcdir)/buildtools/infocontrib.sed >infocontrib.h
	$(SHELL) $(srcdir)/buildtools/geninfocontrib_h.sh AUTHORS | $(srcdir)/sedinfocontrib_h.sh $(srcdir)/buildtools/infocontrib.sed >../AUTHORS
	sed -e 's/\\/@/g' -e 's/ /%/g' <$(top_srcdir)/README >./README.tmp
	$(SHELL) $(srcdir)/buildtools/geninfocontrib_h.sh README <README.tmp >README2.tmp
	sed -e 's/@/\\/g' -e 's/%/ /g' <README2.tmp >../README
	rm -f README*.tmp
	cp $(top_srcdir)/doc/html/index.html ./index.tmp
	$(SHELL) $(srcdir)/buildtools/geninfocontrib_h.sh indexhtml <index.tmp >../doc/html/index.html
	rm -f index.tmp
	cp $(top_srcdir)/man/vice.1 man.tmp
	$(SHELL) $(srcdir)/buildtools/geninfocontrib_h.sh vice1 <man.tmp >man2.tmp
	cat man2.tmp | $(srcdir)/sedinfocontrib_h.sh $(srcdir)/buildtools/infocontrib.sed >../man/vice.1
	rm -f man*.tmp
	rm -f team.tmp
endif

if OWCC
owcc_libs_converted.h:
	$(SHELL) $(srcdir)/buildtools/convert_owcc_libs.sh $(LIBS)
endif

if NATIVE_TOOLS
bin2c: $(srcdir)/buildtools/bin2c.c
	$(NATIVE_TOOLS_COMPILER) -o bin2c $(srcdir)/buildtools/bin2c.c

palette2c: $(srcdir)/buildtools/palette2c.c
	$(NATIVE_TOOLS_COMPILER) -o palette2c $(srcdir)/buildtools/palette2c.c

c128basic64.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/basic64 c128basic64.h "static BYTE c128basic64_embedded[C128_BASIC64_ROM_SIZE]"

c128basichi.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/basichi c128basichi.h "static BYTE c128basichi_embedded[C128_BASIC_ROM_IMAGEHI_SIZE]"

c128basiclo.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/basiclo c128basiclo.h "static BYTE c128basiclo_embedded[C128_BASIC_ROM_IMAGELO_SIZE]"

c128chargde.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/chargde c128chargde.h "static BYTE chargen_de[C128_CHARGEN_ROM_SIZE]"

c128chargen.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/chargen c128chargen.h "static BYTE chargen_int[C128_CHARGEN_ROM_SIZE]"

c128chargfr.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/chargfr c128chargfr.h "static BYTE chargen_fr[C128_CHARGEN_ROM_SIZE]"

c128chargse.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/chargse c128chargse.h "static BYTE chargen_se[C128_CHARGEN_ROM_SIZE]"

c128chargch.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/chargch c128chargch.h "static BYTE chargen_ch[C128_CHARGEN_ROM_SIZE]"

c128kernal.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/kernal c128kernal.h "static BYTE kernal_int[C128_KERNAL_ROM_IMAGE_SIZE]"

c128kernal64.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/kernal64 c128kernal64.h "static BYTE c128kernal64_embedded[C128_KERNAL64_ROM_SIZE]"

c128kernalde.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/kernalde c128kernalde.h "static BYTE kernal_de[C128_KERNAL_ROM_IMAGE_SIZE]"

c128kernalfi.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/kernalfi c128kernalfi.h "static BYTE kernal_fi[C128_KERNAL_ROM_IMAGE_SIZE]"

c128kernalfr.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/kernalfr c128kernalfr.h "static BYTE kernal_fr[C128_KERNAL_ROM_IMAGE_SIZE]"

c128kernalit.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/kernalit c128kernalit.h "static BYTE kernal_it[C128_KERNAL_ROM_IMAGE_SIZE]"

c128kernalno.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/kernalno c128kernalno.h "static BYTE kernal_no[C128_KERNAL_ROM_IMAGE_SIZE]"

c128kernalse.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/kernalse c128kernalse.h "static BYTE kernal_se[C128_KERNAL_ROM_IMAGE_SIZE]"

c128kernalch.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C128/kernalch c128kernalch.h "static BYTE kernal_ch[C128_KERNAL_ROM_IMAGE_SIZE]"

c64basic.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C64/basic c64basic.h "BYTE c64memrom_basic64_rom[C64_BASIC_ROM_SIZE]"

c64kernal.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C64/kernal c64kernal.h "BYTE c64memrom_kernal64_rom[C64_KERNAL_ROM_SIZE]"

c64chargen.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/C64/chargen c64chargen.h "BYTE mem_chargen_rom[C64_CHARGEN_ROM_SIZE]"

cbm2basic128.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/CBM-II/basic.128 cbm2basic128.h "static BYTE cbm2basic128_embedded[0x4000]"

cbm2basic256.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/CBM-II/basic.256 cbm2basic256.h "static BYTE cbm2basic256_embedded[0x4000]"

cbm2basic500.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/CBM-II/basic.500 cbm2basic500.h "static BYTE cbm2basic500_embedded[0x4000]"

cbm2chargen500.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/CBM-II/chargen.500 cbm2chargen500.h "static BYTE cbm2chargen500_embedded[0x1000]"

cbm2chargen600.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/CBM-II/chargen.600 cbm2chargen600.h "static BYTE cbm2chargen600_embedded[0x1000]"

cbm2chargen700.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/CBM-II/chargen.700 cbm2chargen700.h "static BYTE cbm2chargen700_embedded[0x1000]"

cbm2kernal.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/CBM-II/kernal cbm2kernal.h "static BYTE cbm2kernal_embedded[0x2000]"

cbm2kernal500.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/CBM-II/kernal.500 cbm2kernal500.h "static BYTE cbm2kernal500_embedded[0x2000]"

drived1541II.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/d1541II drived1541II.h "static BYTE drive_rom1541ii_embedded[DRIVE_ROM1541II_SIZE]"

drived1571cr.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/d1571cr drived1571cr.h "static BYTE drive_rom1571cr[DRIVE_ROM1571CR_SIZE]"

drivedos1001.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos1001 drivedos1001.h "static BYTE drive_rom1001[DRIVE_ROM1001_SIZE]"

drivedos1540.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos1540 drivedos1540.h "static BYTE drive_rom1540_embedded[DRIVE_ROM1540_SIZE]"

drivedos1541.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos1541 drivedos1541.h "static BYTE drive_rom1541_embedded[DRIVE_ROM1541_SIZE]"

drivedos1551.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos1551 drivedos1551.h "static BYTE drive_rom1551[DRIVE_ROM1551_SIZE]"

drivedos1570.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos1570 drivedos1570.h "static BYTE drive_rom1570[DRIVE_ROM1570_SIZE]"

drivedos1571.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos1571 drivedos1571.h "static BYTE drive_rom1571[DRIVE_ROM1571_SIZE]"

drivedos1581.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos1581 drivedos1581.h "static BYTE drive_rom1581[DRIVE_ROM1581_SIZE]"

#drivedos2000.h: $(EMBEDDED_DEPENDS) bin2c
#	./bin2c $(top_srcdir)/data/DRIVES/dos2000 drivedos2000.h "static BYTE drive_rom2000[DRIVE_ROM2000_SIZE]"
#
#drivedos4000.h: $(EMBEDDED_DEPENDS) bin2c
#	./bin2c $(top_srcdir)/data/DRIVES/dos4000 drivedos4000.h "static BYTE drive_rom4000[DRIVE_ROM4000_SIZE]"

drivedos2031.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos2031 drivedos2031.h "static BYTE drive_rom2031[DRIVE_ROM2031_SIZE]"

drivedos2040.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos2040 drivedos2040.h "static BYTE drive_rom2040[DRIVE_ROM2040_SIZE]"

drivedos3040.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos3040 drivedos3040.h "static BYTE drive_rom3040[DRIVE_ROM3040_SIZE]"

drivedos4040.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/DRIVES/dos4040 drivedos4040.h "static BYTE drive_rom4040[DRIVE_ROM4040_SIZE]"

petbasic1.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/basic1 petbasic1.h "BYTE petbasic1_embedded[0x2000]"

petbasic2.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/basic2 petbasic2.h "BYTE petbasic2_embedded[0x2000]"

petbasic4.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/basic4 petbasic4.h "BYTE petbasic4_embedded[0x3000]"

petchargen.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/chargen petchargen.h "BYTE petchargen_embedded[0x800]"

petedit1g.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/edit1g petedit1g.h "BYTE petedit1g_embedded[0x800]"

petedit2b.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/edit2b petedit2b.h "BYTE petedit2b_embedded[0x800]"

petedit2g.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/edit2g petedit2g.h "BYTE petedit2g_embedded[0x800]"

petedit4b40.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/edit4b40 petedit4b40.h "BYTE petedit4b40_embedded[0x800]"

petedit4b80.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/edit4b80 petedit4b80.h "BYTE petedit4b80_embedded[0x800]"

petedit4g40.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/edit4g40 petedit4g40.h "BYTE petedit4g40_embedded[0x800]"

petkernal1.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/kernal1 petkernal1.h "BYTE petkernal1_embedded[0x1000]"

petkernal2.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/kernal2 petkernal2.h "BYTE petkernal2_embedded[0x1000]"

petkernal4.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/kernal4 petkernal4.h "BYTE petkernal4_embedded[0x1000]"

superpet_char.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/characters.901640-01.bin superpet_char.h "BYTE superpet_char_embedded[0x1000]"

superpet_waterloo_a000.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/waterloo-a000.901898-01.bin superpet_waterloo_a000.h "BYTE superpet_waterloo_a000_embedded[0x1000]"

superpet_waterloo_b000.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/waterloo-b000.901898-02.bin superpet_waterloo_b000.h "BYTE superpet_waterloo_b000_embedded[0x1000]"

superpet_waterloo_c000.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/waterloo-c000.901898-03.bin superpet_waterloo_c000.h "BYTE superpet_waterloo_c000_embedded[0x1000]"

superpet_waterloo_d000.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/waterloo-d000.901898-04.bin superpet_waterloo_d000.h "BYTE superpet_waterloo_d000_embedded[0x1000]"

superpet_waterloo_e000.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/waterloo-e000.901897-01.bin superpet_waterloo_e000.h "BYTE superpet_waterloo_e000_embedded[0x800]"

superpet_waterloo_f000.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PET/waterloo-f000.901898-05.bin superpet_waterloo_f000.h "BYTE superpet_waterloo_f000_embedded[0x1000]"

plus43plus1hi.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PLUS4/3plus1hi plus43plus1hi.h "BYTE extromhi1[PLUS4_KERNAL_ROM_SIZE]"

plus43plus1lo.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PLUS4/3plus1lo plus43plus1lo.h "BYTE extromlo1[PLUS4_BASIC_ROM_SIZE]"

plus4basic.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PLUS4/basic plus4basic.h "BYTE plus4memrom_basic_rom[PLUS4_BASIC_ROM_SIZE]"

plus4kernal.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PLUS4/kernal plus4kernal.h "BYTE plus4memrom_kernal_rom[PLUS4_KERNAL_ROM_SIZE]"

printermps803.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PRINTER/mps803 printermps803.h "static BYTE charset[512][7]"

printernl10cbm.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/PRINTER/nl10-cbm printernl10cbm.h "static BYTE drv_nl10_rom[NL10_ROM_SIZE]"

vic20basic.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/VIC20/basic vic20basic.h "BYTE vic20memrom_basic_rom[VIC20_BASIC_ROM_SIZE]"

vic20chargen.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/VIC20/chargen vic20chargen.h "BYTE vic20chargen_embedded[VIC20_CHARGEN_ROM_SIZE]"

vic20kernal.h: $(EMBEDDED_DEPENDS) bin2c
	./bin2c $(top_srcdir)/data/VIC20/kernal vic20kernal.h "BYTE vic20memrom_kernal_rom[VIC20_KERNAL_ROM_SIZE]"

vdc_comp_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C128/vdc_comp.vpl vdc_comp_vpl.h "unsigned char vdc_comp_vpl[]"

vdc_deft_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C128/vdc_deft.vpl vdc_deft_vpl.h "unsigned char vdc_deft_vpl[]"

vicii_c64hq_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/c64hq.vpl vicii_c64hq_vpl.h "unsigned char vicii_c64hq_vpl[]"

vicii_c64s_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/c64s.vpl vicii_c64s_vpl.h "unsigned char vicii_c64s_vpl[]"

vicii_ccs64_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/ccs64.vpl vicii_ccs64_vpl.h "unsigned char vicii_ccs64_vpl[]"

vicii_community_colors_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/community-colors.vpl vicii_community_colors_vpl.h "unsigned char vicii_community_colors_vpl[]"

vicii_deekay_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/deekay.vpl vicii_deekay_vpl.h "unsigned char vicii_deekay_vpl[]"

vicii_frodo_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/frodo.vpl vicii_frodo_vpl.h "unsigned char vicii_frodo_vpl[]"

vicii_godot_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/godot.vpl vicii_godot_vpl.h "unsigned char vicii_godot_vpl[]"

vicii_pc64_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/pc64.vpl vicii_pc64_vpl.h "unsigned char vicii_pc64_vpl[]"

vicii_pepto_ntsc_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/pepto-ntsc.vpl vicii_pepto_ntsc_vpl.h "unsigned char vicii_pepto_ntsc_vpl[]"

vicii_pepto_ntsc_sony_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/pepto-ntsc-sony.vpl vicii_pepto_ntsc_sony_vpl.h "unsigned char vicii_pepto_ntsc_sony_vpl[]"

vicii_pepto_pal_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/pepto-pal.vpl vicii_pepto_pal_vpl.h "unsigned char vicii_pepto_pal_vpl[]"

vicii_pepto_palold_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/pepto-palold.vpl vicii_pepto_palold_vpl.h "unsigned char vicii_pepto_palold_vpl[]"

vicii_ptoing_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/ptoing.vpl vicii_ptoing_vpl.h "unsigned char vicii_ptoing_vpl[]"

vicii_rgb_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/rgb.vpl vicii_rgb_vpl.h "unsigned char vicii_rgb_vpl[]"

vicii_vice_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64/vice.vpl vicii_vice_vpl.h "unsigned char vicii_vice_vpl[]"

viciidtv_spiff_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/C64DTV/spiff.vpl viciidtv_spiff_vpl.h "unsigned char viciidtv_spiff_vpl[]"

crtc_amber_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/PET/amber.vpl crtc_amber_vpl.h "unsigned char crtc_amber_vpl[]"

crtc_green_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/PET/green.vpl crtc_green_vpl.h "unsigned char crtc_green_vpl[]"

crtc_white_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/PET/white.vpl crtc_white_vpl.h "unsigned char crtc_white_vpl[]"

ted_yape_pal_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/PLUS4/yape-pal.vpl ted_yape_pal_vpl.h "unsigned char ted_yape_pal_vpl[]"

ted_yape_ntsc_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/PLUS4/yape-ntsc.vpl ted_yape_ntsc_vpl.h "unsigned char ted_yape_ntsc_vpl[]"

ted_colodore_ted_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/PLUS4/colodore_ted.vpl ted_colodore_ted_vpl.h "unsigned char ted_colodore_ted_vpl[]"

vic_mike_ntsc_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/VIC20/mike-ntsc.vpl vic_mike_ntsc_vpl.h "unsigned char vic_mike_ntsc_vpl[]"

vic_mike_pal_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/VIC20/mike-pal.vpl vic_mike_pal_vpl.h "unsigned char vic_mike_pal_vpl[]"

vic_vice_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	./palette2c $(top_srcdir)/data/VIC20/vice.vpl vic_vice_vpl.h "unsigned char vic_vice_vpl[]"

else

c128basic64.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/basic64 c128basic64.h "static BYTE c128basic64_embedded[C128_BASIC64_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128basichi.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/basichi c128basichi.h "static BYTE c128basichi_embedded[C128_BASIC_ROM_IMAGEHI_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128basiclo.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/basiclo c128basiclo.h "static BYTE c128basiclo_embedded[C128_BASIC_ROM_IMAGELO_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128chargde.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/chargde c128chargde.h "static BYTE chargen_de[C128_CHARGEN_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128chargen.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/chargen c128chargen.h "static BYTE chargen_int[C128_CHARGEN_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128chargfr.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/chargfr c128chargfr.h "static BYTE chargen_fr[C128_CHARGEN_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128chargse.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/chargse c128chargse.h "static BYTE chargen_se[C128_CHARGEN_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128chargch.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/chargch c128chargch.h "static BYTE chargen_ch[C128_CHARGEN_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128kernal.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/kernal c128kernal.h "static BYTE kernal_int[C128_KERNAL_ROM_IMAGE_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128kernal64.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/kernal64 c128kernal64.h "static BYTE c128kernal64_embedded[C128_KERNAL64_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128kernalde.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/kernalde c128kernalde.h "static BYTE kernal_de[C128_KERNAL_ROM_IMAGE_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128kernalfi.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/kernalfi c128kernalfi.h "static BYTE kernal_fi[C128_KERNAL_ROM_IMAGE_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128kernalfr.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/kernalfr c128kernalfr.h "static BYTE kernal_fr[C128_KERNAL_ROM_IMAGE_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128kernalit.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/kernalit c128kernalit.h "static BYTE kernal_it[C128_KERNAL_ROM_IMAGE_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128kernalno.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/kernalno c128kernalno.h "static BYTE kernal_no[C128_KERNAL_ROM_IMAGE_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128kernalse.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/kernalse c128kernalse.h "static BYTE kernal_se[C128_KERNAL_ROM_IMAGE_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c128kernalch.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C128/kernalch c128kernalch.h "static BYTE kernal_ch[C128_KERNAL_ROM_IMAGE_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c64basic.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C64/basic c64basic.h "BYTE c64memrom_basic64_rom[C64_BASIC_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c64kernal.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C64/kernal c64kernal.h "BYTE c64memrom_kernal64_rom[C64_KERNAL_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

c64chargen.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/C64/chargen c64chargen.h "BYTE mem_chargen_rom[C64_CHARGEN_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

cbm2basic128.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/CBM-II/basic.128 cbm2basic128.h "static BYTE cbm2basic128_embedded[0x4000]" $(CROSS) $(SHELL) $(top_srcdir)

cbm2basic256.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/CBM-II/basic.256 cbm2basic256.h "static BYTE cbm2basic256_embedded[0x4000]" $(CROSS) $(SHELL) $(top_srcdir)

cbm2basic500.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/CBM-II/basic.500 cbm2basic500.h "static BYTE cbm2basic500_embedded[0x4000]" $(CROSS) $(SHELL) $(top_srcdir)

cbm2chargen500.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/CBM-II/chargen.500 cbm2chargen500.h "static BYTE cbm2chargen500_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

cbm2chargen600.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/CBM-II/chargen.600 cbm2chargen600.h "static BYTE cbm2chargen600_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

cbm2chargen700.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/CBM-II/chargen.700 cbm2chargen700.h "static BYTE cbm2chargen700_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

cbm2kernal.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/CBM-II/kernal cbm2kernal.h "static BYTE cbm2kernal_embedded[0x2000]" $(CROSS) $(SHELL) $(top_srcdir)

cbm2kernal500.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/CBM-II/kernal.500 cbm2kernal500.h "static BYTE cbm2kernal500_embedded[0x2000]" $(CROSS) $(SHELL) $(top_srcdir)

drived1541II.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/d1541II drived1541II.h "static BYTE drive_rom1541ii_embedded[DRIVE_ROM1541II_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drived1571cr.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/d1571cr drived1571cr.h "static BYTE drive_rom1571cr[DRIVE_ROM1571CR_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos1001.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos1001 drivedos1001.h "static BYTE drive_rom1001[DRIVE_ROM1001_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos1540.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos1540 drivedos1540.h "static BYTE drive_rom1540_embedded[DRIVE_ROM1540_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos1541.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos1541 drivedos1541.h "static BYTE drive_rom1541_embedded[DRIVE_ROM1541_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos1551.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos1551 drivedos1551.h "static BYTE drive_rom1551[DRIVE_ROM1551_SIZE]" $(CROSS) $(SHELL)  $(top_srcdir)

drivedos1570.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos1570 drivedos1570.h "static BYTE drive_rom1570[DRIVE_ROM1570_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos1571.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos1571 drivedos1571.h "static BYTE drive_rom1571[DRIVE_ROM1571_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos1581.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos1581 drivedos1581.h "static BYTE drive_rom1581[DRIVE_ROM1581_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

#drivedos2000.h: $(EMBEDDED_DEPENDS)
#	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos2000 drivedos2000.h "static BYTE drive_rom2000[DRIVE_ROM2000_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)
#
#drivedos4000.h: $(EMBEDDED_DEPENDS)
#	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos4000 drivedos4000.h "static BYTE drive_rom4000[DRIVE_ROM4000_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos2031.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos2031 drivedos2031.h "static BYTE drive_rom2031[DRIVE_ROM2031_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos2040.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos2040 drivedos2040.h "static BYTE drive_rom2040[DRIVE_ROM2040_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos3040.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos3040 drivedos3040.h "static BYTE drive_rom3040[DRIVE_ROM3040_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

drivedos4040.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/DRIVES/dos4040 drivedos4040.h "static BYTE drive_rom4040[DRIVE_ROM4040_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

petbasic1.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/basic1 petbasic1.h "BYTE petbasic1_embedded[0x2000]" $(CROSS) $(SHELL) $(top_srcdir)

petbasic2.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/basic2 petbasic2.h "BYTE petbasic2_embedded[0x2000]" $(CROSS) $(SHELL) $(top_srcdir)

petbasic4.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/basic4 petbasic4.h "BYTE petbasic4_embedded[0x3000]" $(CROSS) $(SHELL) $(top_srcdir)

petchargen.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/chargen petchargen.h "BYTE petchargen_embedded[0x800]" $(CROSS) $(SHELL) $(top_srcdir)

petedit1g.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/edit1g petedit1g.h "BYTE petedit1g_embedded[0x800]" $(CROSS) $(SHELL) $(top_srcdir)

petedit2b.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/edit2b petedit2b.h "BYTE petedit2b_embedded[0x800]" $(CROSS) $(SHELL) $(top_srcdir)

petedit2g.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/edit2g petedit2g.h "BYTE petedit2g_embedded[0x800]" $(CROSS) $(SHELL) $(top_srcdir)

petedit4b40.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/edit4b40 petedit4b40.h "BYTE petedit4b40_embedded[0x800]" $(CROSS) $(SHELL) $(top_srcdir)

petedit4b80.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/edit4b80 petedit4b80.h "BYTE petedit4b80_embedded[0x800]" $(CROSS) $(SHELL) $(top_srcdir)

petedit4g40.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/edit4g40 petedit4g40.h "BYTE petedit4g40_embedded[0x800]" $(CROSS) $(SHELL) $(top_srcdir)

petkernal1.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/kernal1 petkernal1.h "BYTE petkernal1_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

petkernal2.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/kernal2 petkernal2.h "BYTE petkernal2_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

petkernal4.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/kernal4 petkernal4.h "BYTE petkernal4_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

superpet_char.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/characters.901640-01.bin superpet_char.h "BYTE superpet_char_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

superpet_waterloo_a000.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/waterloo-a000.901898-01.bin superpet_waterloo_a000.h "BYTE superpet_waterloo_a000_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

superpet_waterloo_b000.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/waterloo-b000.901898-02.bin superpet_waterloo_b000.h "BYTE superpet_waterloo_b000_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

superpet_waterloo_c000.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/waterloo-c000.901898-03.bin superpet_waterloo_c000.h "BYTE superpet_waterloo_c000_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

superpet_waterloo_d000.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/waterloo-d000.901898-04.bin superpet_waterloo_d000.h "BYTE superpet_waterloo_d000_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

superpet_waterloo_e000.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/waterloo-e000.901897-01.bin superpet_waterloo_e000.h "BYTE superpet_waterloo_e000_embedded[0x800]" $(CROSS) $(SHELL) $(top_srcdir)

superpet_waterloo_f000.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PET/waterloo-f000.901898-05.bin superpet_waterloo_f000.h "BYTE superpet_waterloo_f000_embedded[0x1000]" $(CROSS) $(SHELL) $(top_srcdir)

plus43plus1hi.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PLUS4/3plus1hi plus43plus1hi.h "BYTE extromhi1[PLUS4_KERNAL_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

plus43plus1lo.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PLUS4/3plus1lo plus43plus1lo.h "BYTE extromlo1[PLUS4_BASIC_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

plus4basic.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PLUS4/basic plus4basic.h "BYTE plus4memrom_basic_rom[PLUS4_BASIC_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

plus4kernal.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PLUS4/kernal plus4kernal.h "BYTE plus4memrom_kernal_rom[PLUS4_KERNAL_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

printermps803.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PRINTER/mps803 printermps803.h "static BYTE charset[512][7]" $(CROSS) $(SHELL) $(top_srcdir)

printernl10cbm.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/PRINTER/nl10-cbm printernl10cbm.h "static BYTE  drv_nl10_rom[NL10_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

vic20basic.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/VIC20/basic vic20basic.h "BYTE vic20memrom_basic_rom[VIC20_BASIC_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

vic20chargen.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/VIC20/chargen vic20chargen.h "BYTE vic20chargen_embedded[VIC20_CHARGEN_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

vic20kernal.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/bin2c.sh $(top_srcdir)/data/VIC20/kernal vic20kernal.h "BYTE vic20memrom_kernal_rom[VIC20_KERNAL_ROM_SIZE]" $(CROSS) $(SHELL) $(top_srcdir)

vdc_comp_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C128/vdc-comp.vpl vdc_comp_vpl.h "unsigned char vdc_comp_vpl[]"

vdc_deft_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C128/vdc-deft.vpl vdc_deft_vpl.h "unsigned char vdc_deft_vpl[]"

vicii_c64hq_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/c64hq.vpl vicii_c64hq_vpl.h "unsigned char vicii_c64hq_vpl[]"

vicii_c64s_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/c64s.vpl vicii_c64s_vpl.h "unsigned char vicii_c64s_vpl[]"

vicii_ccs64_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/ccs64.vpl vicii_ccs64_vpl.h "unsigned char vicii_ccs64_vpl[]"

vicii_community_colors_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/community-colors.vpl vicii_community_colors_vpl.h "unsigned char vicii_community_colors_vpl[]"

vicii_deekay_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/deekay.vpl vicii_deekay_vpl.h "unsigned char vicii_deekay_vpl[]"

vicii_frodo_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/frodo.vpl vicii_frodo_vpl.h "unsigned char vicii_frodo_vpl[]"

vicii_godot_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/godot.vpl vicii_godot_vpl.h "unsigned char vicii_godot_vpl[]"

vicii_pc64_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/pc64.vpl vicii_pc64_vpl.h "unsigned char vicii_pc64_vpl[]"

vicii_pepto_ntsc_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/pepto-ntsc.vpl vicii_pepto_ntsc_vpl.h "unsigned char vicii_pepto_ntsc_vpl[]"

vicii_pepto_ntsc_sony_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/pepto-ntsc-sony.vpl vicii_pepto_ntsc_sony_vpl.h "unsigned char vicii_pepto_ntsc_sony_vpl[]"

vicii_pepto_pal_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/pepto-pal.vpl vicii_pepto_pal_vpl.h "unsigned char vicii_pepto_pal_vpl[]"

vicii_pepto_palold_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/pepto-palold.vpl vicii_pepto_palold_vpl.h "unsigned char vicii_pepto_palold_vpl[]"

vicii_ptoing_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/ptoing.vpl vicii_ptoing_vpl.h "unsigned char vicii_ptoing_vpl[]"

vicii_rgb_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/rgb.vpl vicii_rgb_vpl.h "unsigned char vicii_ptoing_vpl[]"

vicii_vice_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64/vice.vpl vicii_vice_vpl.h "unsigned char vicii_vice_vpl[]"

viciidtv_spiff_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/C64DTV/spiff.vpl viciidtv_spiff_vpl.h "unsigned char viciidtv_spiff_vpl[]"

crtc_amber_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/PET/amber.vpl crtc_amber_vpl.h "unsigned char crtc_amber_vpl[]"

crtc_green_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/PET/green.vpl crtc_green_vpl.h "unsigned char crtc_green_vpl[]"

crtc_white_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/PET/white.vpl crtc_white_vpl.h "unsigned char crtc_white_vpl[]"

ted_yape_pal_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/PLUS4/yape-pal.vpl ted_yape_pal_vpl.h "unsigned char ted_yape_pal_vpl[]"

ted_yape_ntsc_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/PLUS4/yape-ntsc.vpl ted_yape_ntsc_vpl.h "unsigned char ted_yape_ntsc_vpl[]"

ted_colodore_ted_vpl.h: $(EMBEDDED_DEPENDS) palette2c
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/PLUS4/colodore_ted.vpl ted_colodore_ted_vpl.h "unsigned char ted_colodore_ted_vpl[]"

vic_mike_ntsc_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/VIC20/mike-ntsc.vpl vic_mike_ntsc_vpl.h "unsigned char vic_mike_ntsc_vpl[]"

vic_mike_pal_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/VIC20/mike-pal.vpl vic_mike_pal_vpl.h "unsigned char vic_mike_pal_vpl[]"

vic_vice_vpl.h: $(EMBEDDED_DEPENDS)
	$(SHELL) $(srcdir)/buildtools/palette2c.sh <$(top_srcdir)/data/VIC20/vice.vpl vic_vice_vpl.h "unsigned char vic_vice_vpl[]"
endif

archdep_lib = $(top_builddir)/src/arch/shared/libarchdep.a
c128_lib = $(top_builddir)/src/c128/libc128.a
c64_lib = $(top_builddir)/src/c64/libc64.a
c64sc_lib = $(top_builddir)/src/c64/libc64sc.a
scpu64_lib = $(top_builddir)/src/scpu64/libscpu64.a
c64dtv_lib = $(top_builddir)/src/c64dtv/libc64dtv.a
c64dtvstubs_lib = $(top_builddir)/src/c64dtv/libc64dtvstubs.a
c64c128_lib = $(top_builddir)/src/c64/libc64c128.a
c64scpu64_lib = $(top_builddir)/src/c64/libc64scpu64.a
c64c64dtv_lib = $(top_builddir)/src/c64/libc64c64dtv.a
c64cart_lib = $(top_builddir)/src/c64/cart/libc64cart.a
c64cartsystem_lib = $(top_builddir)/src/c64/cart/libc64cartsystem.a
c64commoncart_lib = $(top_builddir)/src/c64/cart/libc64commoncart.a
core_lib = $(top_builddir)/src/core/libcore.a
crtc_lib = $(top_builddir)/src/crtc/libcrtc.a
diag_lib = $(top_builddir)/src/diag/libdiag.a
diskimage_lib = $(top_builddir)/src/diskimage/libdiskimage.a
drive_lib = $(top_builddir)/src/drive/libdrive.a
driveiec_lib = $(top_builddir)/src/drive/iec/libdriveiec.a
driveiec128dcr_lib = $(top_builddir)/src/drive/iec128dcr/libdriveiec128dcr.a
driveiecieee_lib = $(top_builddir)/src/drive/iecieee/libdriveiecieee.a
driveiecc64exp_lib = $(top_builddir)/src/drive/iec/c64exp/libdriveiecc64exp.a
driveiecplus4exp_lib = $(top_builddir)/src/drive/iec/plus4exp/libdriveiecplus4exp.a
driveieee_lib = $(top_builddir)/src/drive/ieee/libdriveieee.a
drivetcbm_lib = $(top_builddir)/src/drive/tcbm/libdrivetcbm.a
fsdevice_lib = $(top_builddir)/src/fsdevice/libfsdevice.a
fileio_lib = $(top_builddir)/src/fileio/libfileio.a
gfxoutputdrv_lib = $(top_builddir)/src/gfxoutputdrv/libgfxoutputdrv.a
hwsiddrv_lib = $(top_builddir)/src/hwsiddrv/libhwsiddrv.a
hvsc_lib = $(top_builddir)/src/hvsc/libhvsc.a
iecbus_lib = $(top_builddir)/src/iecbus/libiecbus.a
imagecontents_lib = $(top_builddir)/src/imagecontents/libimagecontents.a
iodrv_lib = $(top_builddir)/src/iodrv/libiodrv.a
joyport_lib = $(top_builddir)/src/joyport/libjoyport.a
mascuerade_lib = $(top_builddir)/src/vic20/cart/libmascuerade.a
mididrv_lib = $(top_builddir)/src/mididrv/libmididrv.a
monitor_lib = $(top_builddir)/src/monitor/libmonitor.a
p64_lib = $(top_builddir)/src/lib/p64/libp64.a
parallel_lib = $(top_builddir)/src/parallel/libparallel.a
pet_lib = $(top_builddir)/src/pet/libpet.a
plus4_lib = $(top_builddir)/src/plus4/libplus4.a
printerdrv_lib = $(top_builddir)/src/printerdrv/libprinterdrv.a
rs232drv_lib = $(top_builddir)/src/rs232drv/librs232drv.a
raster_lib = $(top_builddir)/src/raster/libraster.a
rtc_lib = $(top_builddir)/src/rtc/librtc.a
samplerdrv_lib = $(top_builddir)/src/samplerdrv/libsamplerdrv.a
serial_lib = $(top_builddir)/src/serial/libserial.a
sid_lib = $(top_builddir)/src/sid/libsid.a
sid_dtv_lib = $(top_builddir)/src/sid/libsid_dtv.a
socketdrv_lib = $(top_builddir)/src/socketdrv/libsocketdrv.a
sounddrv_lib = $(top_builddir)/src/sounddrv/libsounddrv.a
tape_lib = $(top_builddir)/src/tape/libtape.a
tapeport_lib = $(top_builddir)/src/tapeport/libtapeport.a
userport_lib = $(top_builddir)/src/userport/libuserport.a
vdc_lib = $(top_builddir)/src/vdc/libvdc.a
vdrive_lib = $(top_builddir)/src/vdrive/libvdrive.a
vic20_lib = $(top_builddir)/src/vic20/libvic20.a
vic20cart_lib = $(top_builddir)/src/vic20/cart/libvic20cart.a
vicii_lib = $(top_builddir)/src/vicii/libvicii.a
viciidtv_lib = $(top_builddir)/src/vicii/libviciidtv.a
viciisc_lib = $(top_builddir)/src/viciisc/libviciisc.a
viciivsid_lib = $(top_builddir)/src/viciivsid/libviciivsid.a
video_lib = $(top_builddir)/src/video/libvideo.a
vsid_lib = $(top_builddir)/src/c64/libvsid.a
xcbm2_lib = $(top_builddir)/src/cbm2/libcbm2.a
xcbm5x0_lib = $(top_builddir)/src/cbm2/libcbm5x0.a

common_libs = @ARCH_LIBS@ @LIBOBJS@ @READLINE@
resid_libs = @RESID_LIBS@
resid_dtv_libs = @RESID_DTV_LIBS@
ffmpeg_libs = @FFMPEG_LIBS@

# external libraries required for all emulators
emu_extlibs = @UI_LIBS@ @SDL_EXTRA_LIBS@ @SOUND_LIBS@ @QUICKTIME_LIBS@ @JOY_LIBS@ @GFXOUTPUT_LIBS@ @ZLIB_LIBS@ @DYNLIB_LIBS@ @ARCH_LIBS@ $(archdep_lib)

driver_libs = $(joyport_lib) $(samplerdrv_lib) $(sounddrv_lib) $(mididrv_lib) $(socketdrv_lib) $(hwsiddrv_lib) $(iodrv_lib) $(gfxoutputdrv_lib) $(printerdrv_lib) $(rs232drv_lib) $(diskimage_lib) $(fsdevice_lib) $(tape_lib) $(fileio_lib) $(serial_lib) $(core_lib)

if HAVE_RAWDRIVE
blockdev_lib = @ARCH_DIR@/blockdev.o
else
blockdev_lib =
endif

if HAVE_REALDEVICE
realdevice_lib = opencbmlib.o @ARCH_DIR@/dynlib.o
else
realdevice_lib =
endif

if SUPPORT_X64DTV
x64dtv_owbin = x64dtvow
x64dtv_bin = x64dtv
else
x64dtv_owbin =
x64dtv_bin =
endif

if SUPPORT_X64SC
x64sc_owbin = x64scow
x64sc_bin = x64sc
else
x64sc_owbin =
x64sc_bin =
endif

if SUPPORT_XSCPU64
xscpu64_owbin = xscpu64ow
xscpu64_bin = xscpu64
else
xscpu64_owbin =
xscpu64_bin =
endif

if SUPPORT_CMDLINE_TOOLS
c1541 = c1541
petcat = petcat
cartconv = cartconv
else
c1541 =
petcat =
cartconv =
endif

# workaround for extra exe creation
vsidow_SOURCES = maincpu.c
x128ow_SOURCES = maincpu.c
x64ow_SOURCES = maincpu.c
xscpu64ow_SOURCES = maincpu.c
x64dtvow_SOURCES = maincpu.c
x64scow_SOURCES = maincpu.c
xcbm2ow_SOURCES = maincpu.c
xcbm5x0ow_SOURCES = maincpu.c
xpetow_SOURCES = maincpu.c
xplus4ow_SOURCES = maincpu.c
xvicow_SOURCES = maincpu.c

if OWCC
OW_progs = vsidow x64ow $(x64sc_owbin) x128ow $(x64dtv_owbin) xvicow xpetow xplus4ow xcbm2ow xcbm5x0ow $(xscpu64_owbin)
else
OW_progs =
endif

bin_PROGRAMS = vsid x64 $(x64sc_bin) x128 $(x64dtv_bin) xvic xpet xplus4 xcbm2 xcbm5x0 $(xscpu64_bin) $(c1541) $(petcat) $(cartconv) $(OW_progs)

EXTRA_PROGRAMS =

# vsid
vsid_libs =  \
	$(archdep_lib) \
	$(vsid_lib) \
	$(sid_lib) \
	$(monitor_lib) \
	$(sounddrv_lib) \
	$(mididrv_lib) \
	$(socketdrv_lib) \
	$(hwsiddrv_lib) \
	$(iodrv_lib) \
	$(serial_lib) \
	$(core_lib) \
	$(viciivsid_lib) \
	$(raster_lib) \
	$(video_lib) \
	$(common_libs) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(joyport_lib) \
	$(hvsc_lib)

vsid_SOURCES = $(base_sources) $(midi_sources)
vsid_LDADD = $(vsid_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
vsid_DEPENDENCIES = $(vsid_libs)
vsid_LDFLAGS = @vsid_LDFLAGS@

.PHONY: vsid-all
vsid-all: \
	$(BUILT_SOURCES) \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	libvsid \
	libraster \
	libsid \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhvsc \
	libhwsiddrv \
	libiodrv \
	libviciivsid \
	libvideo \
	libarch \
	libarchdep \
	libmonitor \
	libserial \
	libcore \
	libjoyport \
	vsid$(EXEEXT)

# x64
x64_libs =  \
	$(archdep_lib) \
	$(tapeport_lib) \
	$(c64_lib) \
	$(tapeport_lib) \
	$(c64cartsystem_lib) \
	$(c64cart_lib) \
	$(c64commoncart_lib) \
	$(driveiec_lib) \
	$(driveiecieee_lib) \
	$(driveiecc64exp_lib) \
	$(driveieee_lib) \
	$(drive_lib) \
	$(drivetcbm_lib) \
	$(iecbus_lib) \
	$(p64_lib) \
	$(parallel_lib) \
	$(vdrive_lib) \
	$(sid_lib) \
	$(monitor_lib) \
	$(driver_libs) \
	$(vicii_lib) \
	$(raster_lib) \
	$(userport_lib) \
	$(diag_lib) \
	$(rtc_lib) \
	$(video_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(hvsc_lib)

x64_SOURCES = $(base_sources) $(midi_sources)
x64_LDADD = $(x64_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
x64_DEPENDENCIES = $(x64_libs)
x64_LDFLAGS = @x64_LDFLAGS@

.PHONY: x64-all
x64-all: \
	$(BUILT_SOURCES) \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	libc64 \
	libc64cartsystem \
	libc64cart \
	libc64commoncart \
	libdiag \
	libdriveiec \
	libdriveiecieee \
	libdriveiecc64exp \
	libdriveieee \
	libdrive \
	libdrivetcbm \
	libp64 \
	libparallel \
	libvdrive \
	libraster \
	libsid \
	libsamplerdrv \
	libjoyport \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libvicii \
	libvideo \
	libarch \
	libmonitor \
	libdiskimage \
	libiecbus \
	libserial \
	libfsdevice \
	libtape \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	libuserport \
	libtapeport \
	x64$(EXEEXT)

# x64sc
x64sc_libs =  \
	$(archdep_lib) \
	$(tapeport_lib) \
	$(c64sc_lib) \
	$(tapeport_lib) \
	$(c64cartsystem_lib) \
	$(c64cart_lib) \
	$(c64commoncart_lib) \
	$(driveiec_lib) \
	$(driveiecieee_lib) \
	$(driveiecc64exp_lib) \
	$(driveieee_lib) \
	$(drive_lib) \
	$(drivetcbm_lib) \
	$(p64_lib) \
	$(iecbus_lib) \
	$(parallel_lib) \
	$(vdrive_lib) \
	$(sid_lib) \
	$(monitor_lib) \
	$(driver_libs) \
	$(viciisc_lib) \
	$(raster_lib) \
	$(userport_lib) \
	$(diag_lib) \
	$(rtc_lib) \
	$(video_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(hvsc_lib)

x64sc_SOURCES = $(base_sources) $(midi_sources)
x64sc_LDADD = $(x64sc_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
x64sc_DEPENDENCIES = $(x64sc_libs)
x64sc_LDFLAGS = @x64_LDFLAGS@

.PHONY: x64sc-all
x64sc-all: \
	$(BUILT_SOURCES) \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	libc64sc \
	libc64cartsystem \
	libc64cart \
	libc64commoncart \
	libdiag \
	libdriveiec \
	libdriveiecieee \
	libdriveiecc64exp \
	libdriveieee \
	libdrive \
	libdrivetcbm \
	libp64 \
	libparallel \
	libvdrive \
	libraster \
	libsid \
	libsamplerdrv \
	libjoyport \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libviciisc \
	libvideo \
	libarch \
	libmonitor \
	libdiskimage \
	libiecbus \
	libserial \
	libfsdevice \
	libtape \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	libuserport \
	libtapeport \
	x64sc$(EXEEXT)

# x64dtv
x64dtv_libs =  \
	$(archdep_lib) \
	$(c64dtv_lib) \
	$(c64c64dtv_lib) \
	$(driveiec_lib) \
	$(driveiecieee_lib) \
	$(driveiecc64exp_lib) \
	$(driveieee_lib) \
	$(drive_lib) \
	$(drivetcbm_lib) \
	$(p64_lib) \
	$(iecbus_lib) \
	$(parallel_lib) \
	$(userport_lib) \
	$(diag_lib) \
	$(vdrive_lib) \
	$(sid_dtv_lib) \
	$(monitor_lib) \
	$(driver_libs) \
	$(viciidtv_lib) \
	$(raster_lib) \
	$(rtc_lib) \
	$(video_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(resid_dtv_libs) \
	$(ffmpeg_libs) \
	$(c64dtvstubs_lib) \
	$(joyport_lib) \
	$(hvsc_lib)

x64dtv_SOURCES = $(base_sources) $(ps2mouse_sources)
x64dtv_LDADD = $(x64dtv_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
x64dtv_DEPENDENCIES = $(x64dtv_libs)
x64dtv_LDFLAGS = @x64_LDFLAGS@

.PHONY: x64dtv-all
x64dtv-all: \
	$(BUILT_SOURCES) \
	@RESID_DTV_DEP@ \
	@FFMPEG_DEP@ \
	libc64dtv \
	libc64dtvstubs \
	libc64c64dtv \
	libdiag \
	libdriveiec \
	libdriveiecieee \
	libdriveiecc64exp \
	libdriveieee \
	libdrive \
	libdrivetcbm \
	libp64 \
	libparallel \
	libvdrive \
	libraster \
	libsid_dtv \
	libsamplerdrv \
	libjoyport \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libviciidtv \
	libvideo \
	libarch \
	libmonitor \
	libdiskimage \
	libiecbus \
	libserial \
	libfsdevice \
	libtape \
	libuserport \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	x64dtv$(EXEEXT)

# xscpu64
xscpu64_libs =  \
	$(archdep_lib) \
	$(scpu64_lib) \
	$(c64cartsystem_lib) \
	$(c64cart_lib) \
	$(c64commoncart_lib) \
	$(c64scpu64_lib) \
	$(driveiec_lib) \
	$(driveiecieee_lib) \
	$(driveiecc64exp_lib) \
	$(driveieee_lib) \
	$(drive_lib) \
	$(drivetcbm_lib) \
	$(p64_lib) \
	$(iecbus_lib) \
	$(parallel_lib) \
	$(vdrive_lib) \
	$(sid_lib) \
	$(monitor_lib) \
	$(driver_libs) \
	$(viciisc_lib) \
	$(raster_lib) \
	$(userport_lib) \
	$(diag_lib) \
	$(rtc_lib) \
	$(video_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(hvsc_lib)

xscpu64_SOURCES = $(base_sources) $(midi_sources)
xscpu64_LDADD = $(xscpu64_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
xscpu64_DEPENDENCIES = $(xscpu64_libs)
xscpu64_LDFLAGS = @xscpu64_LDFLAGS@

.PHONY: xscpu64-all
xscpu64-all: \
	$(BUILT_SOURCES) \
	libscpu64 \
	libc64scpu64 \
	libc64cartsystem \
	libc64cart \
	libc64commoncart \
	libsamplerdrv \
	libjoyport \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libdiag \
	libdriveiec \
	libdriveiecieee \
	libdriveiecc64exp \
	libdriveieee \
	libdrive \
	libdrivetcbm \
	libp64 \
	libparallel \
	libvdrive \
	libarch \
	libsid \
	libviciisc \
	libvideo \
	libraster \
	libmonitor \
	libdiskimage \
	libiecbus \
	libserial \
	libfsdevice \
	libtape \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	libuserport \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	xscpu64$(EXEEXT)

# x128
x128_libs =  \
	$(archdep_lib) \
	$(tapeport_lib) \
	$(c128_lib) \
	$(tapeport_lib) \
	$(c64cartsystem_lib) \
	$(c64cart_lib) \
	$(c64commoncart_lib) \
	$(c64c128_lib) \
	$(driveiec_lib) \
	$(driveiec128dcr_lib) \
	$(driveiecieee_lib) \
	$(driveiecc64exp_lib) \
	$(driveieee_lib) \
	$(drive_lib) \
	$(drivetcbm_lib) \
	$(p64_lib) \
	$(iecbus_lib) \
	$(parallel_lib) \
	$(vdrive_lib) \
	$(sid_lib) \
	$(monitor_lib) \
	$(driver_libs) \
	$(vicii_lib) \
	$(vdc_lib) \
	$(raster_lib) \
	$(userport_lib) \
	$(diag_lib) \
	$(rtc_lib) \
	$(video_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(hvsc_lib)

x128_SOURCES = $(base_sources) $(midi_sources)
x128_LDADD = $(x128_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
x128_DEPENDENCIES = $(x128_libs)
x128_LDFLAGS = @x128_LDFLAGS@

.PHONY: x128-all
x128-all: \
	$(BUILT_SOURCES) \
	libc128 \
	libc64c128 \
	libc64cartsystem \
	libc64cart \
	libc64commoncart \
	libsamplerdrv \
	libjoyport \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libdiag \
	libdriveiec \
	libdriveiec128dcr \
	libdriveiecieee \
	libdriveiecc64exp \
	libdriveieee \
	libdrive \
	libdrivetcbm \
	libp64 \
	libparallel \
	libvdrive \
	libarch \
	libsid \
	libvicii \
	libvdc \
	libraster \
	libvideo \
	libmonitor \
	libdiskimage \
	libiecbus \
	libserial \
	libfsdevice \
	libtape \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	libuserport \
	libtapeport \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	x128$(EXEEXT)

# xvic
xvic_libs =  \
	$(archdep_lib) \
	$(tapeport_lib) \
	$(vic20cart_lib) \
	$(vic20_lib) \
	$(c64commoncart_lib) \
	$(mascuerade_lib) \
	$(driveiec_lib) \
	$(driveiecieee_lib) \
	$(driveieee_lib) \
	$(drive_lib) \
	$(drivetcbm_lib) \
	$(p64_lib) \
	$(iecbus_lib) \
	$(parallel_lib) \
	$(vdrive_lib) \
	$(monitor_lib) \
	$(sid_lib) \
	$(driver_libs) \
	$(raster_lib) \
	$(userport_lib) \
	$(diag_lib) \
	$(rtc_lib) \
	$(video_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(hvsc_lib)

xvic_SOURCES = $(base_sources) $(midi_sources)
xvic_LDADD = $(xvic_libs) $(emu_extlibs) $(mascuerade_lib) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
xvic_DEPENDENCIES = $(xvic_libs)
xvic_LDFLAGS = @xvic_LDFLAGS@

.PHONY: xvic-all
xvic-all: \
	$(BUILT_SOURCES) \
	libdiag \
	libdriveiec \
	libdriveiecieee \
	libdriveieee \
	libdrive \
	libdrivetcbm \
	libp64 \
	libparallel \
	libvdrive \
	libraster \
	libsamplerdrv \
	libjoyport \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libvic20 \
	libvic20cart \
	libc64commoncart \
	libmascuerade \
	libvideo \
	libarch \
	libmonitor \
	libdiskimage \
	libiecbus \
	libserial \
	libsid \
	libfsdevice \
	libtape \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	libuserport \
	libtapeport \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	xvic$(EXEEXT)

# xpet
xpet_libs =  \
	$(archdep_lib) \
	$(tapeport_lib) \
	$(pet_lib) \
	$(diag_lib) \
	$(driveiecieee_lib) \
	$(driveieee_lib) \
	$(drive_lib) \
	$(driveiec_lib) \
	$(drivetcbm_lib) \
	$(p64_lib) \
	$(parallel_lib) \
	$(vdrive_lib) \
	$(monitor_lib) \
	$(sid_lib) \
	$(driver_libs) \
	$(crtc_lib) \
	$(raster_lib) \
	$(userport_lib) \
	$(video_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(joyport_lib) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(rtc_lib) \
	$(hvsc_lib)

xpet_SOURCES = $(base_sources)
xpet_LDADD = $(xpet_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
xpet_DEPENDENCIES = $(xpet_libs) 
xpet_LDFLAGS = @xpet_LDFLAGS@

.PHONY: xpet-all
xpet-all: \
	$(BUILT_SOURCES) \
	libdiag \
	libdriveiec \
	libdriveiecieee \
	libdriveieee \
	libdrive \
	libdrivetcbm \
	libp64 \
	libparallel \
	libvdrive \
	libraster \
	libcrtc \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libpet \
	libvideo \
	libsid \
	libarch \
	libmonitor \
	libdiskimage \
	libserial \
	libfsdevice \
	libtape \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	libuserport \
	libtapeport \
	libjoyport \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	xpet$(EXEEXT)

# xplus4
xplus4_libs =  \
	$(archdep_lib) \
	$(tapeport_lib) \
	$(plus4_lib) \
	$(diag_lib) \
	$(driveiec_lib) \
	$(driveiecieee_lib) \
	$(driveiecplus4exp_lib) \
	$(drivetcbm_lib) \
	$(drive_lib) \
	$(driveieee_lib) \
	$(parallel_lib) \
	$(driveieee_lib) \
	$(p64_lib) \
	$(iecbus_lib) \
	$(vdrive_lib) \
	$(monitor_lib) \
	$(sid_lib) \
	$(driver_libs) \
	$(raster_lib) \
	$(rtc_lib) \
	$(userport_lib) \
	$(video_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(joyport_lib) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(hvsc_lib)

xplus4_SOURCES = $(base_sources)
xplus4_LDADD = $(xplus4_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
xplus4_DEPENDENCIES = $(xplus4_libs)
xplus4_LDFLAGS = @xplus4_LDFLAGS@

.PHONY: xplus4-all
xplus4-all: \
	$(BUILT_SOURCES) \
	libdiag \
	libdriveiec \
	libdriveiecieee \
	libdriveiecplus4exp \
	libdrivetcbm \
	libdrive \
	libdriveieee \
	libparallel \
	libp64 \
	libvdrive \
	libraster \
	libsamplerdrv \
	libjoyport \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libplus4 \
	libvideo \
	libsid \
	libarch \
	libmonitor \
	libdiskimage \
	libiecbus \
	libserial \
	libfsdevice \
	libtape \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	libuserport \
	libtapeport \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	xplus4$(EXEEXT)

# xcbm2
xcbm2_libs = \
	$(archdep_lib) \
	$(tapeport_lib) \
	$(xcbm2_lib) \
	$(diag_lib) \
	$(driveiecieee_lib) \
	$(driveieee_lib) \
	$(drive_lib) \
	$(driveiec_lib) \
	$(drivetcbm_lib) \
	$(p64_lib) \
	$(parallel_lib) \
	$(vdrive_lib) \
	$(monitor_lib) \
	$(sid_lib) \
	$(driver_libs) \
	$(crtc_lib) \
	$(raster_lib) \
	$(video_lib) \
	$(userport_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(joyport_lib) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(rtc_lib) \
	$(hvsc_lib)

xcbm2_SOURCES = $(base_sources)
xcbm2_LDADD = $(xcbm2_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
xcbm2_DEPENDENCIES = $(xcbm2_libs) 
xcbm2_LDFLAGS = @xcbm2_LDFLAGS@

.PHONY: xcbm2-all
xcbm2-all: \
	$(BUILT_SOURCES) \
	libcbm2 \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libarch \
	libsid \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	libdiag \
	libdriveiec \
	libdriveiecieee \
	libdriveieee \
	libdrive \
	libdrivetcbm \
	libp64 \
	libparallel \
	libvdrive \
	libraster \
	libcrtc \
	libvideo \
	libmonitor \
	libdiskimage \
	libserial \
	libfsdevice \
	libtape \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	libuserport \
	libtapeport \
	libjoyport \
	xcbm2$(EXEEXT)

# xcbm5x0
xcbm5x0_libs = \
	$(archdep_lib) \
	$(tapeport_lib) \
	$(xcbm5x0_lib) \
	$(diag_lib) \
	$(driveiecieee_lib) \
	$(driveieee_lib) \
	$(drive_lib) \
	$(drivetcbm_lib) \
	$(driveiec_lib) \
	$(p64_lib) \
	$(parallel_lib) \
	$(vdrive_lib) \
	$(monitor_lib) \
	$(sid_lib) \
	$(driver_libs) \
	$(vicii_lib) \
	$(raster_lib) \
	$(rtc_lib) \
	$(video_lib) \
	$(userport_lib) \
	$(common_libs) \
	$(imagecontents_lib) \
	$(joyport_lib) \
	$(resid_libs) \
	$(ffmpeg_libs) \
	$(hvsc_lib)

xcbm5x0_SOURCES = $(base_sources)
xcbm5x0_LDADD = $(xcbm5x0_libs) $(emu_extlibs) @TFE_LIBS@ @NETPLAY_LIBS@ @READLINE_LIBS@
xcbm5x0_DEPENDENCIES = $(xcbm5x0_libs) 
xcbm5x0_LDFLAGS = @xcbm2_LDFLAGS@

.PHONY: xcbm5x0-all
xcbm5x0-all: \
	$(BUILT_SOURCES) \
	libcbm5x0 \
	libsamplerdrv \
	libjoyport \
	libsounddrv \
	libmididrv \
	libsocketdrv \
	libhwsiddrv \
	libiodrv \
	libgfxoutputdrv \
	libprinterdrv \
	librs232drv \
	libarch \
	libsid \
	@RESID_DEP@ \
	@FFMPEG_DEP@ \
	libdiag \
	libdriveiec \
	libdriveiecieee \
	libdriveieee \
	libdrive \
	libdrivetcbm \
	libp64 \
	libparallel \
	libvdrive \
	libraster \
	libvicii \
	libvideo \
	libmonitor \
	libdiskimage \
	libserial \
	libfsdevice \
	libtape \
	libimagecontents \
	libfileio \
	libcore \
	librtc \
	libtapeport \
	xcbm5x0$(EXEEXT)

# c1541
c1541_SOURCES = \
	c1541.c \
	cbmdos.c \
	charset.c \
	findpath.c \
	gcr.c \
	cbmimage.c \
	info.c \
	ioutil.c \
	lib.c \
	log.c \
	rawfile.c \
	resources.c \
	util.c \
	zfile.c \
	zipcode.c

if OWCC
ARCH_EXTRA_OBJECTS = @ARCH_DIR@/libarch.a
else
ARCH_EXTRA_OBJECTS = 	@ARCH_DIR@/archdep.o
endif

# FIXME: I don't like linking `archdep.o' this way, but we cannot use
# the generic `ARCH_LIBS', because we would have to link against X11
# libs too then.
c1541_LDADD = \
	$(ARCH_EXTRA_OBJECTS) \
	$(archdep_lib) \
	$(p64_lib) \
	$(blockdev_lib) \
	$(realdevice_lib) \
	$(vdrive_lib) \
	$(imagecontents_lib) \
	$(fileio_lib) \
	$(diskimage_lib) \
	$(tape_lib) \
	$(serial_lib) \
	$(socketdrv_lib) \
	@SDL_EXTRA_LIBS@ \
	@READLINE@ @READLINE_LIBS@ \
	@ZLIB_LIBS@ @DYNLIB_LIBS@

.PHONY: c1541-all
c1541-all: \
	$(BUILT_SOURCES) \
	libarch \
	libp64 \
	libvdrive \
	libdiskimage \
	libserial \
	libtape \
	libimagecontents \
	libfileio \
	libsocketdrv \
	c1541$(EXEEXT)

# petcat
petcat_SOURCES = \
	charset.c \
	findpath.c \
	ioutil.c \
	lib.c \
	log.c \
	petcat.c \
	rawfile.c \
	resources.c \
	util.c \
	zfile.c \
	zipcode.c

petcat_LDADD = \
	$(ARCH_EXTRA_OBJECTS) \
	$(archdep_lib) \
	$(fileio_lib) \
	$(socketdrv_lib) \
	@SDL_EXTRA_LIBS@ \
	@ZLIB_LIBS@

.PHONY: petcat-all
petcat-all: \
	$(BUILT_SOURCES) \
	libarch \
	libfileio \
	libsocketdrv \
	petcat$(EXEEXT)

# cartconv
cartconv_SOURCES = cartconv.c

# distclean
DISTCLEANFILES = $(BUILT_SOURCES) $(GENFILES)

# Special targets to build libraries in subdirectories.

.PHONY: libvsid libc64 libc64sc libc128 libc64dtv libc64dtvstubs libvic20 \
	libpet libplus4 libcbm2 libcbm5x0 libdrive libvdrive libprinterdrv \
	librs232drv libgfxoutputdrv libsamplerdrv libjoyport libsounddrv \
	libmididrv libsocketdrv libhwsiddrv libiodrv libdiskimage libfsdevice \
	libtape libarch libarchdep libvicii libviciisc libviciidtv \
	libviciivsid libvdc libsid libcrtc libvideo libserial libparallel \
	libcore libiecbus libfileio librtc libuserport libtapeport libp64 \
	libscpu64 libdiag \
	@RESID_DEP@ @RESID_DTV_DEP@ @FFMPEG_DEP@

libarch:
	@(cd arch && $(MAKE))
libarchdep:
	@(echo "making libarchdep.a")
	@(cd arch/shared && $(MAKE))
libc128:
	@echo "making all in c128"
	@(cd c128 && $(MAKE))
libc64:
	@echo "making libc64.a in c64"
	@(cd c64 && $(MAKE) libc64.a)
libscpu64:
	@echo "making libscpu64.a in scpu64"
	@(cd scpu64 && $(MAKE) libscpu64.a)
libc64sc:
	@echo "making libc64sc.a in c64"
	@(cd c64 && $(MAKE) libc64sc.a)
libc64c128:
	@echo "making libc64c128.a in c64"
	@(cd c64 && $(MAKE) libc64c128.a)
libc64scpu64:
	@echo "making libc64scpu64.a in c64"
	@(cd c64 && $(MAKE) libc64scpu64.a)
libc64c64dtv:
	@echo "making libc64c64dtv.a in c64"
	@(cd c64 && $(MAKE) libc64c64dtv.a)
libc64cart:
	@echo "making libc64cart in c64"
	@(cd c64 && $(MAKE) libc64cart)
libc64cartsystem:
	@echo "making libc64cartsystem in c64"
	@(cd c64 && $(MAKE) libc64cartsystem)
libc64commoncart:
	@echo "making libc64commoncart in c64"
	@(cd c64 && $(MAKE) libc64commoncart)
libc64dtv:
	@echo "making libc64dtv.a in c64dtv"
	@(cd c64dtv && $(MAKE) libc64dtv.a)
libc64dtvstubs:
	@echo "making libc64dtvstubs.a in c64dtv"
	@(cd c64dtv && $(MAKE) libc64dtvstubs.a)
libcbm2:
	@echo "making libcbm2.a in cbm2"
	@(cd cbm2 && $(MAKE) libcbm2.a)
libcbm5x0:
	@echo "making libcbm5x0.a in cbm2"
	@(cd cbm2 && $(MAKE) libcbm5x0.a)
libcore:
	@echo "making all in core"
	@(cd core && $(MAKE))
libcrtc:
	@echo "making all in crtc"
	@(cd crtc && $(MAKE))
libdiag:
	@echo "making all in diag"
	@(cd diag && $(MAKE))
libdiskimage:
	@echo "making all in diskimage"
	@(cd diskimage && $(MAKE))
libdrive:
	@echo "making all in drive"
	@(cd drive && $(MAKE))
libp64:
	@echo "making libp64 in lib"
	@(cd lib && $(MAKE) libp64)
libdriveiec:
	@(cd drive && $(MAKE) libdriveiec)
libdriveiec128dcr:
	@(cd drive && $(MAKE) libdriveiec128dcr)
libdriveiecc64exp:
	@(cd drive/iec && $(MAKE) libdriveiecc64exp)
libdriveiecieee:
	@(cd drive && $(MAKE) libdriveiecieee)
libdriveiecplus4exp:
	@(cd drive/iec && $(MAKE) libdriveiecplus4exp)
libdriveieee:
	@(cd drive && $(MAKE) libdriveieee)
libdrivetcbm:
	@(cd drive && $(MAKE) libdrivetcbm)
libffmpeg:
	@(cd lib/libffmpeg && $(MAKE) VICEBUILD=1)
libfileio:
	@echo "making all in fileio"
	@(cd fileio && $(MAKE))
libfsdevice:
	@echo "making all in fsdevice"
	@(cd fsdevice && $(MAKE))
libgfxoutputdrv:
	@echo "making all in gfxoutputdrv"
	@(cd gfxoutputdrv && $(MAKE))
libhvsc:
	@echo "making all in hvsc"
	@(cd hvsc && $(MAKE))
libhwsiddrv:
	@echo "making all in hwsiddrv"
	@(cd hwsiddrv && $(MAKE))
libiecbus:
	@echo "making all in iecbus"
	@(cd iecbus && $(MAKE))
libimagecontents:
	@echo "making all in imagecontents"
	@(cd imagecontents && $(MAKE))
libiodrv:
	@echo "making all in iodrv"
	@(cd iodrv && $(MAKE))
libmascuerade:
	@echo "making libmascuerade.a in vic20/cart"
	@(cd vic20/cart && $(MAKE) libmascuerade.a)
libmididrv:
	@echo "making libmididrv.a in mididrv"
	@(cd mididrv && $(MAKE) libmididrv.a)
libmonitor:
	@echo "making all in monitor"
	@(cd monitor && $(MAKE))
libparallel:
	@echo "making all in parallel"
	@(cd parallel && $(MAKE))
libpet:
	@echo "making all in pet"
	@(cd pet && $(MAKE))
libplus4:
	@echo "making all in plus4"
	@(cd plus4 && $(MAKE))
libprinterdrv:
	@echo "making all in printerdrv"
	@(cd printerdrv && $(MAKE))
librs232drv:
	@echo "making all in rs232drv"
	@(cd rs232drv && $(MAKE))
libraster:
	@echo "making libraster.a in raster"
	@(cd raster && $(MAKE) libraster.a)
librtc:
	@echo "making librtc.a in rtc"
	@(cd rtc && $(MAKE) librtc.a)
libresid:
	@echo "making all in resid"
	@(cd resid && $(MAKE))
libresiddtv:
	@echo "making libresiddtv.a in resid-dtv"
	@(cd resid-dtv && $(MAKE) libresiddtv.a)
libsamplerdrv:
	@echo "making all in samplerdrv"
	@(cd samplerdrv && $(MAKE))
libjoyport:
	@echo "making all in joyport"
	@(cd joyport && $(MAKE))
libserial:
	@echo "making all in serial"
	@(cd serial && $(MAKE))
libsid:
	@echo "making libsid.a in sid"
	@(cd sid && $(MAKE) libsid.a)
libsid_dtv:
	@echo "making libsid_dtv.a in sid"
	@(cd sid && $(MAKE) libsid_dtv.a)
libsocketdrv:
	@echo "making libsocketdrv.a in socketdrv"
	@(cd socketdrv && $(MAKE) libsocketdrv.a)
libsounddrv:
	@echo "making all in sounddrv"
	@(cd sounddrv && $(MAKE))
libtape:
	@echo "making all in tape"
	@(cd tape && $(MAKE))
libuserport:
	@echo "making all in userport"
	@(cd userport && $(MAKE))
libtapeport:
	@echo "making all in tapeport"
	@(cd tapeport && $(MAKE))
libvdc:
	@echo "making all in vdc"
	@(cd vdc && $(MAKE))
libvdrive:
	@echo "making all in vdrive"
	@(cd vdrive && $(MAKE))
libvic20:
	@echo "making all in vic20"
	@(cd vic20 && $(MAKE))
libvic20cart:
	@echo "making libvic20cart.a in vic20/cart"
	@(cd vic20/cart && $(MAKE) libvic20cart.a)
libvicii:
	@echo "making libvicii.a in vicii"
	@(cd vicii && $(MAKE) libvicii.a)
libviciidtv:
	@echo "making libviciidtv.a in vicii"
	@(cd vicii && $(MAKE) libviciidtv.a)
libviciisc:
	@echo "making libviciisc.a in viciisc"
	@(cd viciisc && $(MAKE) libviciisc.a)
libviciivsid:
	@echo "making libviciivsid.a in viciivsid"
	@(cd viciivsid && $(MAKE) libviciivsid.a)
libvideo:
	@echo "making all in video"
	@(cd video && $(MAKE))
libvsid:
	@echo "making libvsid.a in c64"
	@(cd c64 && $(MAKE) psiddrv.h libvsid.a)

x64ow$(EXEEXT): x64$(EXEEXT)
	wrc -fe=x64ow$(EXEEXT) arch/win32/resc64.res x64$(EXEEXT)

x64dtvow$(EXEEXT): x64dtv$(EXEEXT)
	wrc -fe=x64dtvow$(EXEEXT) arch/win32/resc64dtv.res x64dtv$(EXEEXT)

xscpu64ow$(EXEEXT): xscpu64$(EXEEXT)
	wrc -fe=xscpu64ow$(EXEEXT) arch/win32/resscpu64.res xscpu64$(EXEEXT)

x64scow$(EXEEXT): x64sc$(EXEEXT)
	wrc -fe=x64scow$(EXEEXT) arch/win32/resc64.res x64sc$(EXEEXT)

x128ow$(EXEEXT): x128$(EXEEXT)
	wrc -fe=x128ow$(EXEEXT) arch/win32/resc128.res x128$(EXEEXT)

xcbm2ow$(EXEEXT): xcbm2$(EXEEXT)
	wrc -fe=xcbm2ow$(EXEEXT) arch/win32/rescbm2.res xcbm2$(EXEEXT)

xcbm5x0ow$(EXEEXT): xcbm5x0$(EXEEXT)
	wrc -fe=xcbm5x0ow$(EXEEXT) arch/win32/rescbm5x0.res xcbm5x0$(EXEEXT)

xpetow$(EXEEXT): xpet$(EXEEXT)
	wrc -fe=xpetow$(EXEEXT) arch/win32/respet.res xpet$(EXEEXT)

xplus4ow$(EXEEXT): xplus4$(EXEEXT)
	wrc -fe=xplus4ow$(EXEEXT) arch/win32/resplus4.res xplus4$(EXEEXT)

xvicow$(EXEEXT): xvic$(EXEEXT)
	wrc -fe=xvicow$(EXEEXT) arch/win32/resvic20.res xvic$(EXEEXT)

vsidow$(EXEEXT): vsid$(EXEEXT)
	wrc -fe=vsidow$(EXEEXT) arch/win32/resvsid.res vsid$(EXEEXT)

if HAVE_SHARED_FFMPEG
$(top_builddir)/src/libffmpeg.a: libffmpeg
	touch dummy.c
	$(CC) -o dummy.o -c dummy.c
	$(AR) cru $(top_builddir)/src/libffmpeg.a dummy.o
	$(RANLIB) $(top_builddir)/src/libffmpeg.a
	rm dummy.o dummy.c
endif

if HAVE_STATIC_FFMPEG
$(top_builddir)/src/lib/libffmpeg/libavformat/libavformat.a: libffmpeg

$(top_builddir)/src/lib/libffmpeg/libavcodec/libavcodec.a: libffmpeg

$(top_builddir)/src/lib/libffmpeg/libavutil/libavutil.a: libffmpeg

$(top_builddir)/src/lib/libffmpeg/libswscale/libswscale.a: libffmpeg

$(top_builddir)/src/lib/libffmpeg/libswresample/libswresample.a: libffmpeg

$(top_builddir)/src/lib/libffmpeg/lib/libmp3lame.a: libffmpeg

$(top_builddir)/src/lib/libffmpeg/lib/libx264.a: libffmpeg
endif

clean-ffmpeg:
	rm -f $(top_builddir)/src/libffmpeg.a

if HAVE_SHARED_FFMPEG
EXTRA_CLEAN = clean-ffmpeg
else
if HAVE_STATIC_FFMPEG
EXTRA_CLEAN = clean-ffmpeg
else
EXTRA_CLEAN =
endif
endif

clean: clean-recursive $(EXTRA_CLEAN)
	
