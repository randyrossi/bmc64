SUBDIRS = building html readmes

@ALTERNATE_DOCDIR_TRUE@realdocdir = $(prefix)/share/doc/vice
@ALTERNATE_DOCDIR_FALSE@realdocdir = $(VICEDIR)/doc

docdir = $(realdocdir)

MISC_DOX = \
	CIA-README.txt \
	coding-guidelines.txt \
	Documentation-Howto.txt \
	Doxygen-Howto.txt \
	ffmpeg-support.txt \
	hardware-sids.txt \
	iec-bus.txt \
	MacOSX-ResourceTree.txt \
	NLS-Howto.txt \
	ports-status.txt \
	SDL-support.txt \
	WinNT-Unicode.txt

TEXI_TOOLS = \
	fixdox.sh \
	t2h.pl \
	texi2chm.sh \
	texi2guide.sh

DOC_TOOLS = \
	checkdoc.c \
	checkdoc.mak \
	Doxyfile \
	mainpage.dox \
	mkdoxy.sh \
	src2doxy.sh \
	txt2doxy.sh

STYLE_TOOLS = \
	.indent.pro \
	indent-test.c \
	uncrustify.cfg
	
VICE_DOX = \
	vice.txt \
	vice.pdf \
	vice.chm \
	vice.hlp \
	vice.guide \
	vice.inf \
	vice.info
	
info_TEXINFOS = vice.texi

dist_doc_DATA = $(MISC_DOX) $(VICE_DOX)

EXTRA_DIST = $(DOC_TOOLS) $(TEXI_TOOLS) $(STYLE_TOOLS)

AM_MAKEINFOHTMLFLAGS = 
AM_MAKEINFOFLAGS = --no-split

$(builddir)/vice.txt:		$(srcdir)/vice.texi
	touch vicetmp.txt
	LANG=C $(MAKEINFO) -D$(PLATFORM_DOX_FLAGS) -o vicetmp.txt --no-headers $(srcdir)/vice.texi
	$(srcdir)/fixdox.sh txt <vicetmp.txt | LANG=C sed 's/ \+$$//' >$(builddir)/vice.txt
	rm -f vicetmp.txt

$(builddir)/vice.pdf:	$(srcdir)/vice.texi
if BUILD_PDF
	LANG=C sed 's/@heading NO WARRANTY/@center NO WARRANTY/g' <$(srcdir)/vice.texi >vicepdf.texi
	LANG=C $(TEXI2DVI) -q --clean --pdf "--texinfo=@set $(PLATFORM_DOX_FLAGS) " -o $(builddir)/vice.pdf vicepdf.texi
	rm -f vicepdf.texi

PDF_DEPS=$(builddir)/vice.pdf
else
	touch $(builddir)/vice.pdf

PDF_DEPS=
endif

$(builddir)/vice.guide:	$(srcdir)/vice.texi
if BUILD_AMIGAGUIDE
	cp $(srcdir)/vice.texi tmp.texi
	$(srcdir)/fixdox.sh guide
	$(srcdir)/texi2guide.sh $(MAKEGUIDE) $(builddir)/vice.guide vicetmp.texi $(host_cpu) $(host_os)
	rm -f tmp.texi vicetmp.texi

GUIDE_DEPS=$(builddir)/vice.guide
else
	touch $(srcdir)/vice.guide

GUIDE_DEPS=
endif

$(builddir)/vice.hlp:	$(srcdir)/vice.texi
if BUILD_HLP
	cp $(srcdir)/vice.texi tmp.texi
	$(srcdir)/fixdox.sh hlp
	$(MAKERTF) -Dplatformwindows --hpj=vice.hpj --output=vice.rtf vicetmp.texi
	$(HCRTF) -o VICE.HLP -xn vice.hpj
	rm -f vice.hpj vice.rtf vicetmp.texi
	mv VICE.HLP $(builddir)/vice.hlp

HLP_DEPS=$(builddir)/vice.hlp
else
	touch $(builddir)/vice.hlp

HLP_DEPS=
endif

$(builddir)/vice.chm:	$(srcdir)/vice.texi
if BUILD_CHM
	$(srcdir)/texi2chm.sh $(HHC) $(builddir)/vice.chm $(srcdir)/vice.texi $(srcdir)/fixdox.sh $(srcdir)/t2h.pl

CHM_DEPS=$(builddir)/vice.chm
else
	touch $(builddir)/vice.chm

CHM_DEPS=
endif

$(builddir)/vice.inf:	$(srcdir)/vice.texi
if BUILD_IPF
	cp $(srcdir)/vice.texi tmp.texi
	$(srcdir)/fixdox.sh ipf
	$(TEXI2IPF) $(srcdir)/vicetmp.texi >temp.ipf
	$(IPFC) -i temp.ipf
	mv temp.inf $(builddir)/vice.inf
	rm -f temp.ipf vicetmp.texi

IPF_DEPS=$(builddir)/vice.inf
else
	touch $(builddir)/vice.inf

IPF_DEPS=
endif

$(builddir)/vice.info:	$(srcdir)/vice.texi
	touch $(builddir)/vice.info
	$(MAKEINFO) -D$(PLATFORM_DOX_FLAGS) -o $(builddir)/vice.info --no-split $(srcdir)/vice.texi

INFO_DEPS = $(builddir)/vice.txt $(builddir)/vice.info $(PDF_DEPS) $(GUIDE_DEPS) $(HLP_DEPS) $(CHM_DEPS) $(IPF_DEPS)

.texi.pdf:
