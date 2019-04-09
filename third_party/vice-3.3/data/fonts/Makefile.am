fontsdir = $(VICEDIR)/fonts

fonts_DATA = \
	vice-cbm.bdf \
	CBM.ttf

EXTRA_DIST = $(fonts_DATA) cbm-directory-charset.fon 

install: install-data-am
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	@echo "Preparing fontdir, please wait..." ; \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	if [ ! `fc-cache --list > /dev/null 2>&1` ] ; then \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	  if test "x${HOME}" != "x" ; then \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	    mkdir ${HOME}/.fonts > /dev/null 2>&1 ; \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	    cp $(fontsdir)/CBM.ttf ${HOME}/.fonts ; \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	    fc-cache ; \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	  else \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	    echo '$$HOME not set... giving up.' ; \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	  fi \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	else \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	  echo 'fc-cache not available... giving up.' ; \
@HAVE_FC_CACHE_TRUE@@SDL_COMPILE_FALSE@	fi
@SDL_COMPILE_FALSE@	@echo "Preparing fontdir..." ; \
@SDL_COMPILE_FALSE@	cd $(DESTDIR)$(fontsdir) ; \
@SDL_COMPILE_FALSE@	bdftopcf -o vice-cbm.pcf vice-cbm.bdf ; \
@SDL_COMPILE_FALSE@	${MKFONTDIR_BIN} .
