@ALTERNATE_DOCDIR_TRUE@realdocdir = $(prefix)/share/doc/vice
@ALTERNATE_DOCDIR_FALSE@realdocdir = $(VICEDIR)/doc

docdir = $(realdocdir)

doc_DATA = \
	amigaos.html \
	android.html \
	beos.html \
	darwin.html \
	images/CIPS2008.png \
	images/new.gif \
	images/vice-logo.svg \
	fonts/Lato-Bold.eot \
	fonts/Lato-BoldItalic.eot \
	fonts/Lato-BoldItalic.ttf \
	fonts/Lato-BoldItalic.woff \
	fonts/Lato-BoldItalic.woff2 \
	fonts/Lato-Bold.ttf \
	fonts/Lato-Bold.woff \
	fonts/Lato-Bold.woff2 \
	fonts/Lato-Italic.eot \
	fonts/Lato-Italic.ttf \
	fonts/Lato-Italic.woff \
	fonts/Lato-Italic.woff2 \
	fonts/Lato-Regular.eot \
	fonts/Lato-Regular.ttf \
	fonts/Lato-Regular.woff \
	fonts/Lato-Regular.woff2 \
	fonts/OFL.txt \
	index.html \
	macosx.html \
	COPYING \
	NEWS \
	nsosrh.html \
	qnx.html \
	robots.txt \
	sco.html \
	sitemap.xml \
	solaris.html \
	sortix.html \
	unix.html \
	vice.css \
	vice_1.html \
	vice_2.html \
	vice_3.html \
	vice_4.html \
	vice_5.html \
	vice_6.html \
	vice_7.html \
	vice_8.html \
	vice_9.html \
	vice_10.html \
	vice_11.html \
	vice_12.html \
	vice_13.html \
	vice_14.html \
	vice_15.html \
	vice_16.html \
	vice_17.html \
	vice_18.html \
	vice_19.html \
	vice_20.html \
	vice_21.html \
	vice_22.html \
	vice_toc.html \
	windows.html

EXTRA_DIST = $(doc_DATA) \
	texi2html

CLEAN_STUFF = \
	vice_1.html \
	vice_2.html \
	vice_3.html \
	vice_4.html \
	vice_5.html \
	vice_6.html \
	vice_7.html \
	vice_8.html \
	vice_9.html \
	vice_10.html \
	vice_11.html \
	vice_12.html \
	vice_13.html \
	vice_14.html \
	vice_15.html \
	vice_16.html \
	vice_17.html \
	vice_18.html \
	vice_19.html \
	vice_20.html \
	vice_21.html \
	vice_22.html \
	vice_toc.html

vice_1.html vice_2.html vice_3.html vice_4.html vice_5.html vice_6.html vice_7.html vice_8.html vice_9.html vice_10.html vice_11.html vice_12.html vice_13.html vice_14.html vice_15.html vice_16.html vice_17.html vice_18.html vice_19.html vice_20.html vice_21.html vice_22.html: vice_toc.html

vice_toc.html: $(top_srcdir)/doc/vice.texi
	(cd $(builddir); \
	$(PERL) $(srcdir)/texi2html -number -split_chapter -D$(PLATFORM_DOX_FLAGS) $(top_srcdir)/doc/vice.texi)

COPYING: $(top_srcdir)/COPYING
	cp $< $@

NEWS: $(top_srcdir)/NEWS
	cp $< $@

clean:	clean-am
	rm -f $(CLEAN_STUFF)
