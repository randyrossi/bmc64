if VICE_QUIET
.SILENT:
endif

# Process this file with automake to produce Makefile.in

SUBDIRS = src build data man doc

# COMMON_EXTRA_DIST is what used to be the EXTRA_DIST, when adding dist
# files, please add them to the COMMON_EXTRA_DIST list.

COMMON_EXTRA_DIST = \
	.descr \
	AUTHORS \
	autogen.sh \
	config.rpath \
	configure.proto \
	ChangeLog \
	ChangeLog.1 \
	ChangeLog.2 \
	ChangeLog.3 \
	ChangeLog.4 \
	COPYING \
	FEEDBACK \
	INSTALL \
	NEWS \
	gcccpu.sh \
	tixbuildinfo \
	vice-mint-sdl.spec \
	vice-mint-sdl.spec.in

EXTRA_DIST = $(COMMON_EXTRA_DIST)

if SUPPORT_X64SC
  X64SC_INCLUDED = yes
else
  X64SC_INCLUDED = no
endif

if SUPPORT_XSCPU64
  XSCPU64_INCLUDED = yes
else
  XSCPU64_INCLUDED = no
endif

.PHONY: vsid x64 x64sc x128 x64dtv xvic xpet xplus4 xcbm2 xcbm5x0 xscpu64 c1541 petcat cartconv
vsid:
	(cd src; $(MAKE) vsid-all)

x64:
	(cd src; $(MAKE) x64-all)

if SUPPORT_X64SC
x64sc:
	(cd src; $(MAKE) x64sc-all)
endif

x128:
	(cd src; $(MAKE) x128-all)

if SUPPORT_X64DTV
x64dtv:
	(cd src; $(MAKE) x64dtv-all)
endif

if SUPPORT_XSCPU64
xscpu64:
	(cd src; $(MAKE) xscpu64-all)
endif

xvic:
	(cd src; $(MAKE) xvic-all)

xpet:
	(cd src; $(MAKE) xpet-all)

xplus4:
	(cd src; $(MAKE) xplus4-all)

xcbm2:
	(cd src; $(MAKE) xcbm2-all)

xcbm5x0:
	(cd src; $(MAKE) xcbm5x0-all)

c1541:
	(cd src; $(MAKE) c1541-all)

petcat:
	(cd src; $(MAKE) petcat-all)

cartconv:
	(cd src; $(MAKE) cartconv)

ffmpeglibs:
	(cd src/lib; $(MAKE) ffmpeglibs)

ffmpeglibs-install:
	(cd src/lib; $(MAKE) ffmpeglibs-install)

ffmpeglibs-clean:
	(cd src/lib; $(MAKE) clean)

install: installvice

# If we have no ChangeLog (which should be generated from SVN), generate a
# dummy one.

ChangeLog:
	touch ChangeLog

if AMIGA_COMPILE
if SDL_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_amiga.sh $(STRIP) $(VICE_VERSION) $(host_cpu) $(host_os) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) $(top_srcdir) $(SDL_VERSION) $(EXEEXT)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_amiga.sh $(STRIP) $(VICE_VERSION) $(host_cpu) $(host_os) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) $(top_srcdir) $(SDL_VERSION) $(EXEEXT)
endif
endif

if BEOS_COMPILE
if SDL_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_beos.sh $(STRIP) $(VICE_VERSION) $(host_cpu) $(host_os) @ENABLE_ARCH@ nozip $(XSCPU64_INCLUDED) $(top_srcdir) $(SDL_VERSION)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_beos.sh $(STRIP) $(VICE_VERSION) $(host_cpu) $(host_os) @ENABLE_ARCH@ zip $(XSCPU64_INCLUDED) $(top_srcdir) $(SDL_VERSION)
endif
endif

if UNIX_MACOSX_COMPILE
if NATIVE_GTK3_COMPILE
bindist:
	/bin/bash @ARCH_SRC_DIR@/../gtk3/make-bindist_osx.sh $(top_srcdir) $(STRIP) $(VICE_VERSION) @ENABLE_ARCH@ nozip

bindistzip:
	/bin/bash @ARCH_SRC_DIR@/../gtk3/make-bindist_osx.sh $(top_srcdir) $(STRIP) $(VICE_VERSION) @ENABLE_ARCH@ zip
else
bindist:
	FW_DIR=@FW_DIR@ /bin/bash @ARCH_SRC_DIR@/../sdl/make-bindist_osx.sh $(top_srcdir) $(STRIP) $(VICE_VERSION) @ENABLE_ARCH@ nozip $(SDL_VERSION)

bindistzip:
	FW_DIR=@FW_DIR@ /bin/bash @ARCH_SRC_DIR@/../sdl/make-bindist_osx.sh $(top_srcdir) $(STRIP) $(VICE_VERSION) @ENABLE_ARCH@ zip $(SDL_VERSION)
endif
endif

if UNIX_SKYOS_COMPILE
if SDL_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_skyos.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) $(top_srcdir)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_skyos.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) $(top_srcdir)
else
bindist:
	$(SHELL) @ARCH_SRC_DIR@/skyos/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) $(top_srcdir)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/skyos/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) $(top_srcdir)
endif
endif

if UNIX_QNX6_COMPILE
if SDL_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_qnx6.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) $(top_srcdir) $(MAKE) $(host_cpu)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_qnx6.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) $(top_srcdir) $(MAKE) $(host_cpu)
else
bindist:
	$(SHELL) @ARCH_SRC_DIR@/qnx6/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) $(top_srcdir) $(MAKE) $(host_cpu)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/qnx6/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) $(top_srcdir) $(MAKE) $(host_cpu)
endif
endif

if UNIX_QNX4_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/qnx4/make-bindist.sh $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) $(top_srcdir)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/qnx4/make-bindist.sh $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) $(top_srcdir)
endif

if UNIX_SOLARIS_COMPILE
if SDL_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_scosol.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) sol $(infodir) $(mandir) $(top_srcdir) $(MAKE) $(host_cpu) $(host_os)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_scosol.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) sol $(infodir) $(mandir) $(top_srcdir) $(MAKE) $(host_cpu) $(host_os)
endif
endif

if UNIX_RHAPSODY_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/next_open_rhap/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) rhapsody $(top_srcdir) $(MAKE)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/next_open_rhap/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) rhapsody $(top_srcdir) $(MAKE)
endif

if UNIX_OPENSTEP_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/next_open_rhap/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) openstep $(top_srcdir) $(MAKE)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/next_open_rhap/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) openstep $(top_srcdir) $(MAKE)
endif

if UNIX_NEXTSTEP_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/next_open_rhap/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) nextstep $(top_srcdir) $(MAKE)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/next_open_rhap/make-bindist.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) nextstep $(top_srcdir) $(MAKE)
endif

if UNIX_OPENSERVER5_COMPILE
if SDL_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_scosol.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) sco5 $(infodir) $(mandir) $(top_srcdir) $(MAKE)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_scosol.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) sco5 $(infodir) $(mandir) $(top_srcdir) $(MAKE)
endif
endif

if UNIX_OPENSERVER6_COMPILE
if SDL_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_scosol.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) sco6 $(infodir) $(mandir) $(top_srcdir) $(MAKE)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_scosol.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) sco6 $(infodir) $(mandir) $(top_srcdir) $(MAKE)
endif
endif

if UNIX_UNIXWARE_COMPILE
if SDL_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_scosol.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) sco7 $(infodir) $(mandir) $(top_srcdir) $(MAKE)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_scosol.sh $(STRIP) $(VICE_VERSION) $(prefix) $(CROSS) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) sco7 $(infodir) $(mandir) $(top_srcdir) $(MAKE)
endif
endif

if WIN32_COMPILE
if NATIVE_GTK3_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_win32.sh $(STRIP) $(VICE_VERSION) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) $(top_srcdir) $(host_cpu) $(abs_top_builddir) $(CROSS) $(objdump) $(CC)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_win32.sh $(STRIP) $(VICE_VERSION) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) $(top_srcdir) $(host_cpu) $(abs_top_builddir) $(CROSS) $(objdump) $(CC)
else
if SDL_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_win32.sh $(STRIP) $(VICE_VERSION) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) $(top_srcdir) $(host_cpu) $(SDL_VERSION)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_win32.sh $(STRIP) $(VICE_VERSION) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) $(top_srcdir) $(host_cpu) $(SDL_VERSION)
endif
endif
endif

if SYLLABLE_COMPILE
bindist:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_syllable.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ nozip $(X64SC_INCLUDED) $(top_srcdir)

bindistzip:
	$(SHELL) @ARCH_SRC_DIR@/make-bindist_syllable.sh $(STRIP) $(VICE_VERSION) $(prefix) @ENABLE_ARCH@ zip $(X64SC_INCLUDED) $(top_srcdir)
endif

if HAVE_SHARED_FFMPEG
EXTRA_INSTALLS = ffmpeglibs-install
EXTRA_CLEANS = ffmpeglibs-clean
else
if HAVE_STATIC_FFMPEG
EXTRA_INSTALLS =
EXTRA_CLEANS = ffmpeglibs-clean
else
EXTRA_INSTALLS =
EXTRA_CLEANS =
endif
endif

if MAKE_BINDIST
installvice:
	@echo this platform doesn\'t use \"make install\", use \"make bindist\" instead
else
installvice: install-recursive $(EXTRA_INSTALLS)
endif

# Manually define the dist targets so the correct tar usage will be used.
dist-gzip:
	$(DOS2UNIX) `find $(top_srcdir) -name "*.sh"`
	$(DOS2UNIX) $(top_srcdir)/doc/vice.texi
	$(MAKE) distdir
	$(AMTAR) chf - $(distdir) | GZIP=$(GZIP_ENV) gzip -c >$(distdir).tar.gz
	$(am__remove_distdir)

dist dist-all:
	$(DOS2UNIX) `find $(top_srcdir) -name "*.sh"`
	$(DOS2UNIX) $(top_srcdir)/doc/vice.texi
	$(MAKE) distdir
	$(AMTAR) chf - $(distdir) | GZIP=$(GZIP_ENV) gzip -c >$(distdir).tar.gz
	$(am__remove_distdir)

$(top_srcdir)/configure.ac: $(top_srcdir)/configure.proto
	$(am__cd) $(srcdir) && $(SHELL) autogen.sh

clean: $(EXTRA_CLEANS) clean-recursive
