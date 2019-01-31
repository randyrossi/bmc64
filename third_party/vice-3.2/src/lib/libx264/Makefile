# Makefile

include config.mak

vpath %.c $(SRCPATH)
vpath %.h $(SRCPATH)
vpath %.S $(SRCPATH)
vpath %.asm $(SRCPATH)
vpath %.rc $(SRCPATH)

GENERATED =

all: default
default:

SOURCE_FILES = \
	x264.c \
	x264dll.c \
	common/bitstream.c \
	common/cabac.c \
	common/common.c \
	common/cpu.c \
	common/dct.c \
	common/deblock.c \
	common/frame.c \
	common/macroblock.c \
	common/mc.c \
	common/mvpred.c \
	common/oclobj-msvc.h \
	common/opencl.c \
	common/osdep.c \
	common/pixel.c \
	common/predict.c \
	common/quant.c \
	common/rectangle.c \
	common/set.c \
	common/threadpool.c \
	common/vlc.c \
	common/win32thread.c \
	common/aarch64/mc-c.c \
	common/aarch64/predict-c.c \
	common/arm/mc-c.c \
	common/arm/predict-c.c \
	common/ppc/dct.c \
	common/ppc/deblock.c \
	common/ppc/mc.c \
	common/ppc/pixel.c \
	common/ppc/predict.c \
	common/ppc/quant.c \
	common/x86/mc-c.c \
	common/x86/predict-c.c \
	encoder/analyse.c \
	encoder/cabac.c \
	encoder/cavlc.c \
	encoder/encoder.c \
	encoder/lookahead.c \
	encoder/macroblock.c \
	encoder/me.c \
	encoder/ratecontrol.c \
	encoder/rdo.c \
	encoder/set.c \
	encoder/slicetype.c \
	encoder/slicetype-cl.c \
	extras/getopt.c \
	filters/filters.c \
	filters/video/cache.c \
	filters/video/crop.c \
	filters/video/depth.c \
	filters/video/fix_vfr_pts.c \
	filters/video/internal.c \
	filters/video/resize.c \
	filters/video/select_every.c \
	filters/video/source.c \
	filters/video/video.c \
	input/avs.c \
	input/ffms.c \
	input/input.c \
	input/lavf.c \
	input/raw.c \
	input/thread.c \
	input/timecode.c \
	input/y4m.c \
	output/flv.c \
	output/flv_bytestream.c \
	output/matroska.c \
	output/matroska_ebml.c \
	output/mp4.c \
	output/mp4_lsmash.c \
	output/raw.c \
	tools/checkasm.c

HEADER_FILES = \
	x264.h \
	x264cli.h \
	common/bitstream.h \
	common/cabac.h \
	common/common.h \
	common/cpu.h \
	common/dct.h \
	common/frame.h \
	common/macroblock.h \
	common/mc.h \
	common/opencl.h \
	common/osdep.h \
	common/pixel.h \
	common/predict.h \
	common/quant.h \
	common/rectangle.h \
	common/set.h \
	common/threadpool.h \
	common/win32thread.h \
	common/aarch64/dct.h \
	common/aarch64/mc.h \
	common/aarch64/pixel.h \
	common/aarch64/predict.h \
	common/aarch64/quant.h \
	common/arm/dct.h \
	common/arm/mc.h \
	common/arm/pixel.h \
	common/arm/predict.h \
	common/arm/quant.h \
	common/opencl/x264-cl.h \
	common/ppc/dct.h \
	common/ppc/mc.h \
	common/ppc/pixel.h \
	common/ppc/ppccommon.h \
	common/ppc/predict.h \
	common/ppc/quant.h \
	common/x86/dct.h \
	common/x86/mc.h \
	common/x86/pixel.h \
	common/x86/predict.h \
	common/x86/quant.h \
	common/x86/util.h \
	dos/getopt.h \
	encoder/analyse.h \
	encoder/macroblock.h \
	encoder/me.h \
	encoder/ratecontrol.h \
	encoder/set.h \
	extras/avisynth_c.h \
	extras/avxsynth_c.h \
	extras/cl.h \
	extras/cl_platform.h \
	extras/getopt.h \
	extras/inttypes.h \
	extras/stdint.h \
	extras/windowsPorts/basicDataTypeConversions.h \
	extras/windowsPorts/windows2linux.h \
	filters/filters.h \
	filters/video/internal.h \
	filters/video/video.h \
	input/input.h \
	output/flv_bytestream.h \
	output/matroska_ebml.h \
	output/output.h

ASM_FILES = \
	common/aarch64/asm.S \
	common/aarch64/dct-a.S \
	common/aarch64/deblock-a.S \
	common/aarch64/mc-a.S \
	common/aarch64/pixel-a.S \
	common/aarch64/predict-a.S \
	common/aarch64/quant-a.S \
	common/arm/asm.S \
	common/arm/cpu-a.S \
	common/arm/dct-a.S \
	common/arm/deblock-a.S \
	common/arm/mc-a.S \
	common/arm/pixel-a.S \
	common/arm/predict-a.S \
	common/arm/quant-a.S \
	common/x86/bitstream-a.asm \
	common/x86/cabac-a.asm \
	common/x86/const-a.asm \
	common/x86/cpu-a.asm \
	common/x86/dct-32.asm \
	common/x86/dct-64.asm \
	common/x86/dct-a.asm \
	common/x86/deblock-a.asm \
	common/x86/mc-a.asm \
	common/x86/mc-a2.asm \
	common/x86/pixel-32.asm \
	common/x86/pixel-a.asm \
	common/x86/predict-a.asm \
	common/x86/quant-a.asm \
	common/x86/sad16-a.asm \
	common/x86/sad-a.asm \
	common/x86/trellis-64.asm \
	common/x86/x86inc.asm \
	common/x86/x86util.asm \
	tools/checkasm-a.asm

EXTRA_FILES = \
	AUTHORS \
	config.guess \
	config.sub \
	configure \
	COPYING \
	Makefile \
	version.sh \
	x264res.rc \
	common/opencl/bidir.cl \
	common/opencl/downscale.cl \
	common/opencl/intra.cl \
	common/opencl/motionsearch.cl \
	common/opencl/subpel.cl \
	common/opencl/weightp.cl \
	extras/gas-preprocessor.pl \
	doc/ratecontrol.txt \
	doc/regression_test.txt \
	doc/standards.txt \
	doc/threads.txt \
	doc/vui.txt \
	tools/digress/__init__.py \
	tools/digress/cli.py \
	tools/digress/comparers.py \
	tools/digress/constants.py \
	tools/digress/errors.py \
	tools/digress/testing.py \
	tools/digress/scm/__init__.py \
	tools/digress/scm/dummy.py \
	tools/digress/scm/git.py \
	tools/cltostr.sh \
	tools/countquant_x264.pl \
	tools/msvsdepend.sh \
	tools/q_matrix_jvt.cfg \
	tools/test_x264.py

SRCS = common/mc.c common/predict.c common/pixel.c common/macroblock.c \
       common/frame.c common/dct.c common/cpu.c common/cabac.c \
       common/common.c common/osdep.c common/rectangle.c \
       common/set.c common/quant.c common/deblock.c common/vlc.c \
       common/mvpred.c common/bitstream.c \
       encoder/analyse.c encoder/me.c encoder/ratecontrol.c \
       encoder/set.c encoder/macroblock.c encoder/cabac.c \
       encoder/cavlc.c encoder/encoder.c encoder/lookahead.c

SRCCLI = x264.c input/input.c input/timecode.c input/raw.c input/y4m.c \
         output/raw.c output/matroska.c output/matroska_ebml.c \
         output/flv.c output/flv_bytestream.c filters/filters.c \
         filters/video/video.c filters/video/source.c filters/video/internal.c \
         filters/video/resize.c filters/video/cache.c filters/video/fix_vfr_pts.c \
         filters/video/select_every.c filters/video/crop.c filters/video/depth.c

SRCSO =
OBJS =
OBJSO =
OBJCLI =

OBJCHK = tools/checkasm.o

CONFIG := $(shell cat config.h)

# GPL-only files
ifneq ($(findstring HAVE_GPL 1, $(CONFIG)),)
SRCCLI +=
endif

# Optional module sources
ifneq ($(findstring HAVE_AVS 1, $(CONFIG)),)
SRCCLI += input/avs.c
endif

ifneq ($(findstring HAVE_THREAD 1, $(CONFIG)),)
SRCCLI += input/thread.c
SRCS   += common/threadpool.c
endif

ifneq ($(findstring HAVE_WIN32THREAD 1, $(CONFIG)),)
SRCS += common/win32thread.c
endif

ifneq ($(findstring HAVE_LAVF 1, $(CONFIG)),)
SRCCLI += input/lavf.c
endif

ifneq ($(findstring HAVE_FFMS 1, $(CONFIG)),)
SRCCLI += input/ffms.c
endif

ifneq ($(findstring HAVE_GPAC 1, $(CONFIG)),)
SRCCLI += output/mp4.c
endif

ifneq ($(findstring HAVE_LSMASH 1, $(CONFIG)),)
SRCCLI += output/mp4_lsmash.c
endif

# MMX/SSE optims
ifneq ($(AS),)
X86SRC0 = const-a.asm cabac-a.asm dct-a.asm deblock-a.asm mc-a.asm \
          mc-a2.asm pixel-a.asm predict-a.asm quant-a.asm \
          cpu-a.asm dct-32.asm bitstream-a.asm
ifneq ($(findstring HIGH_BIT_DEPTH, $(CONFIG)),)
X86SRC0 += sad16-a.asm
else
X86SRC0 += sad-a.asm
endif
X86SRC = $(X86SRC0:%=common/x86/%)

ifeq ($(ARCH),X86)
ARCH_X86 = yes
ASMSRC   = $(X86SRC) common/x86/pixel-32.asm
endif

ifeq ($(ARCH),X86_64)
ARCH_X86 = yes
ASMSRC   = $(X86SRC:-32.asm=-64.asm) common/x86/trellis-64.asm
endif

ifdef ARCH_X86
SRCS   += common/x86/mc-c.c common/x86/predict-c.c
OBJASM  = $(ASMSRC:%.asm=%.o)
$(OBJASM): common/x86/x86inc.asm common/x86/x86util.asm
OBJCHK += tools/checkasm-a.o
endif
endif

# AltiVec optims
ifeq ($(ARCH),PPC)
ifneq ($(AS),)
SRCS += common/ppc/mc.c common/ppc/pixel.c common/ppc/dct.c \
        common/ppc/quant.c common/ppc/deblock.c \
        common/ppc/predict.c
endif
endif

# NEON optims
ifeq ($(ARCH),ARM)
ifneq ($(AS),)
ASMSRC += common/arm/cpu-a.S common/arm/pixel-a.S common/arm/mc-a.S \
          common/arm/dct-a.S common/arm/quant-a.S common/arm/deblock-a.S \
          common/arm/predict-a.S
SRCS   += common/arm/mc-c.c common/arm/predict-c.c
OBJASM  = $(ASMSRC:%.S=%.o)
endif
endif

# AArch64 NEON optims
ifeq ($(ARCH),AARCH64)
ifneq ($(AS),)
ASMSRC += common/aarch64/dct-a.S     \
          common/aarch64/deblock-a.S \
          common/aarch64/mc-a.S      \
          common/aarch64/pixel-a.S   \
          common/aarch64/predict-a.S \
          common/aarch64/quant-a.S
SRCS   += common/aarch64/mc-c.c      \
          common/aarch64/predict-c.c
OBJASM  = $(ASMSRC:%.S=%.o)
endif
endif

ifneq ($(HAVE_GETOPT_LONG),1)
SRCCLI += extras/getopt.c
endif

ifeq ($(SYS),WINDOWS)
OBJCLI += $(if $(RC), x264res.o)
ifneq ($(SONAME),)
SRCSO  += x264dll.c
OBJSO  += $(if $(RC), x264res.dll.o)
endif
endif

ifeq ($(HAVE_OPENCL),yes)
common/oclobj.h: common/opencl/x264-cl.h $(wildcard $(SRCPATH)/common/opencl/*.cl)
	cat $^ | $(SRCPATH)/tools/cltostr.sh $@
GENERATED += common/oclobj.h
SRCS += common/opencl.c encoder/slicetype-cl.c
endif

OBJS   += $(SRCS:%.c=%.o)
OBJCLI += $(SRCCLI:%.c=%.o)
OBJSO  += $(SRCSO:%.c=%.o)

DISTFILES = $(SOURCE_FILES) $(HEADER_FILES) $(ASM_FILES) $(EXTRA_FILES)

.PHONY: all default fprofiled clean distclean install install-* uninstall cli lib-* etags

cli: x264$(EXE)
lib-static: $(LIBX264)
lib-shared: $(SONAME)

$(LIBX264): $(GENERATED) .depend $(OBJS) $(OBJASM)
	rm -f $(LIBX264)
	$(AR)$@ $(OBJS) $(OBJASM)
	$(if $(RANLIB), $(RANLIB) $@)

$(SONAME): $(GENERATED) .depend $(OBJS) $(OBJASM) $(OBJSO)
	$(LD)$@ $(OBJS) $(OBJASM) $(OBJSO) $(SOFLAGS) $(LDFLAGS)

ifneq ($(EXE),)
.PHONY: x264 checkasm
x264: x264$(EXE)
checkasm: checkasm$(EXE)
endif

x264$(EXE): $(GENERATED) .depend $(OBJCLI) $(CLI_LIBX264)
	$(LD)$@ $(OBJCLI) $(CLI_LIBX264) $(LDFLAGSCLI) $(LDFLAGS)

checkasm$(EXE): $(GENERATED) .depend $(OBJCHK) $(LIBX264)
	$(LD)$@ $(OBJCHK) $(LIBX264) $(LDFLAGS)

$(OBJS) $(OBJASM) $(OBJSO) $(OBJCLI) $(OBJCHK): .depend

%.o: %.asm common/x86/x86inc.asm common/x86/x86util.asm
	$(AS) $(ASFLAGS) -o $@ $<
	-@ $(if $(STRIP), $(STRIP) -x $@) # delete local/anonymous symbols, so they don't show up in oprofile

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $<
	-@ $(if $(STRIP), $(STRIP) -x $@) # delete local/anonymous symbols, so they don't show up in oprofile

%.dll.o: %.rc x264.h
	$(RC) $(RCFLAGS)$@ -DDLL $<

%.o: %.rc x264.h
	$(RC) $(RCFLAGS)$@ $<

.depend: config.mak
	@rm -f .depend
	@echo 'dependency file generation...'
ifeq ($(COMPILER),CL)
	@$(foreach SRC, $(addprefix $(SRCPATH)/, $(SRCS) $(SRCCLI) $(SRCSO)), $(SRCPATH)/tools/msvsdepend.sh "$(CC)" "$(CFLAGS)" "$(SRC)" "$(SRC:$(SRCPATH)/%.c=%.o)" 1>> .depend;)
else
	@$(foreach SRC, $(addprefix $(SRCPATH)/, $(SRCS) $(SRCCLI) $(SRCSO)), $(CC) $(CFLAGS) $(SRC) $(DEPMT) $(SRC:$(SRCPATH)/%.c=%.o) $(DEPMM) 1>> .depend;)
endif

config.mak:
	./configure

depend: .depend
ifneq ($(wildcard .depend),)
include .depend
endif

SRC2 = $(SRCS) $(SRCCLI)
# These should cover most of the important codepaths
OPT0 = --crf 30 -b1 -m1 -r1 --me dia --no-cabac --direct temporal --ssim --no-weightb
OPT1 = --crf 16 -b2 -m3 -r3 --me hex --no-8x8dct --direct spatial --no-dct-decimate -t0  --slice-max-mbs 50
OPT2 = --crf 26 -b4 -m5 -r2 --me hex --cqm jvt --nr 100 --psnr --no-mixed-refs --b-adapt 2 --slice-max-size 1500
OPT3 = --crf 18 -b3 -m9 -r5 --me umh -t1 -A all --b-pyramid normal --direct auto --no-fast-pskip --no-mbtree
OPT4 = --crf 22 -b3 -m7 -r4 --me esa -t2 -A all --psy-rd 1.0:1.0 --slices 4
OPT5 = --frames 50 --crf 24 -b3 -m10 -r3 --me tesa -t2
OPT6 = --frames 50 -q0 -m9 -r2 --me hex -Aall
OPT7 = --frames 50 -q0 -m2 -r1 --me hex --no-cabac

ifeq (,$(VIDS))
fprofiled:
	@echo 'usage: make fprofiled VIDS="infile1 infile2 ..."'
	@echo 'where infiles are anything that x264 understands,'
	@echo 'i.e. YUV with resolution in the filename, y4m, or avisynth.'
else
fprofiled:
	$(MAKE) clean
	$(MAKE) x264$(EXE) CFLAGS="$(CFLAGS) $(PROF_GEN_CC)" LDFLAGS="$(LDFLAGS) $(PROF_GEN_LD)"
	$(foreach V, $(VIDS), $(foreach I, 0 1 2 3 4 5 6 7, ./x264$(EXE) $(OPT$I) --threads 1 $(V) -o $(DEVNULL) ;))
ifeq ($(COMPILER),CL)
# Because Visual Studio timestamps the object files within the PGD, it fails to build if they change - only the executable should be deleted
	rm -f x264$(EXE)
else
	rm -f $(SRC2:%.c=%.o)
endif
	$(MAKE) CFLAGS="$(CFLAGS) $(PROF_USE_CC)" LDFLAGS="$(LDFLAGS) $(PROF_USE_LD)"
	rm -f $(SRC2:%.c=%.gcda) $(SRC2:%.c=%.gcno) *.dyn pgopti.dpi pgopti.dpi.lock *.pgd *.pgc
endif

clean:
	rm -f $(OBJS) $(OBJASM) $(OBJCLI) $(OBJSO) $(SONAME) *.a *.lib *.exp *.pdb x264 x264.exe .depend TAGS
	rm -f checkasm checkasm.exe $(OBJCHK) $(GENERATED) x264_lookahead.clbin
	rm -f $(SRC2:%.c=%.gcda) $(SRC2:%.c=%.gcno) *.dyn pgopti.dpi pgopti.dpi.lock *.pgd *.pgc

distclean: clean
	rm -f config.mak x264_config.h config.h config.log x264.pc x264.def

install-cli: cli
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) x264$(EXE) $(DESTDIR)$(bindir)

install-lib-dev:
	$(INSTALL) -d $(DESTDIR)$(includedir)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL) -d $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL) -m 644 $(SRCPATH)/x264.h $(DESTDIR)$(includedir)
	$(INSTALL) -m 644 x264_config.h $(DESTDIR)$(includedir)
	$(INSTALL) -m 644 x264.pc $(DESTDIR)$(libdir)/pkgconfig

install-lib-static: lib-static install-lib-dev
	$(INSTALL) -m 644 $(LIBX264) $(DESTDIR)$(libdir)
	$(if $(RANLIB), $(RANLIB) $(DESTDIR)$(libdir)/$(LIBX264))

install-lib-shared: lib-shared install-lib-dev
ifneq ($(IMPLIBNAME),)
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -m 755 $(SONAME) $(DESTDIR)$(bindir)
	$(INSTALL) -m 644 $(IMPLIBNAME) $(DESTDIR)$(libdir)
else ifneq ($(SONAME),)
	ln -f -s $(SONAME) $(DESTDIR)$(libdir)/libx264.$(SOSUFFIX)
	$(INSTALL) -m 755 $(SONAME) $(DESTDIR)$(libdir)
endif

uninstall:
	rm -f $(DESTDIR)$(includedir)/x264.h $(DESTDIR)$(includedir)/x264_config.h $(DESTDIR)$(libdir)/libx264.a
	rm -f $(DESTDIR)$(bindir)/x264$(EXE) $(DESTDIR)$(libdir)/pkgconfig/x264.pc
ifneq ($(IMPLIBNAME),)
	rm -f $(DESTDIR)$(bindir)/$(SONAME) $(DESTDIR)$(libdir)/$(IMPLIBNAME)
else ifneq ($(SONAME),)
	rm -f $(DESTDIR)$(libdir)/$(SONAME) $(DESTDIR)$(libdir)/libx264.$(SOSUFFIX)
endif

etags: TAGS

TAGS:
	etags $(SRCS)

srcdir = $SRCPATH
top_srcdir = $SRCPATH
top_builddir := $(shell pwd)

distdir: $(DISTFILES)
	@srcdirstrip=`echo "$(srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	topsrcdirstrip=`echo "$(top_srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	list='$(DISTFILES)'; \
	  dist_files=`for file in $$list; do echo $$file; done | \
	  sed -e "s|^$$srcdirstrip/||;t" \
	      -e "s|^$$topsrcdirstrip/|$(top_builddir)/|;t"`; \
	case $$dist_files in \
	  */*) mkdir -p `echo "$$dist_files" | \
			   sed '/\//!d;s|^|$(distdir)/|;s,/[^/]*$$,,' | \
			   sort -u` ;; \
	esac; \
	for file in $$dist_files; do \
	  if test -f $$file || test -d $$file; then d=.; else d=$(srcdir); fi; \
	  if test -d $$d/$$file; then \
	    dir=`echo "/$$file" | sed -e 's,/[^/]*$$,,'`; \
	    if test -d "$(distdir)/$$file"; then \
	      find "$(distdir)/$$file" -type d ! -perm -700 -exec chmod u+rwx {} \;; \
	    fi; \
	    if test -d $(srcdir)/$$file && test $$d != $(srcdir); then \
	      cp -fpR $(srcdir)/$$file "$(distdir)$$dir" || exit 1; \
	      find "$(distdir)/$$file" -type d ! -perm -700 -exec chmod u+rwx {} \;; \
	    fi; \
	    cp -fpR $$d/$$file "$(distdir)$$dir" || exit 1; \
	  else \
	    test -f "$(distdir)/$$file" \
	    || cp -p $$d/$$file "$(distdir)/$$file" \
	    || exit 1; \
	  fi; \
	done
