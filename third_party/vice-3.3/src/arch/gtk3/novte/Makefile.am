
GLIB_GENMARSHAL = glib-genmarshal
GLIB_COMPILE_RESOURCES = glib-compile-resources
GLIB_MKENUMS = glib-mkenums

AM_CPPFLAGS = \
	-DHAVE_ROUND \
	\
	@GTK_CFLAGS@ \
	@ARCH_INCLUDES@ \
	@GLIB_CFLAGS@ \
	$(extra_includes) \
	-I$(top_builddir)/src \
	-I$(top_builddir)/src/arch/gtk3/novte \
	-I$(top_srcdir)/src/arch/gtk3/novte \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libnovte.a

libnovte_a_SOURCES = \
	debug.cc \
	iso2022.cc \
	keymap.cc \
	matcher.cc \
	ring.cc \
	table.cc \
	vte.cc \
	vteaccess.cc \
	vteconv.cc \
	vtedraw.cc \
	vtegtk.cc \
	vterowdata.cc \
	vteseq.cc \
	vtestream.cc \
	vtetypes.cc \
	vteunistr.cc \
	vteutils.cc

EXTRA_DIST = \
	novte.h \
	vtedeprecated.h \
	vteenums.h \
	vteglobals.h \
	vtemacros.h \
	vteterminal.h \
	attr.hh \
	buffer.h \
	caps.hh \
	caps-list.hh \
	color-triple.hh \
	debug.h \
	iso2022.h \
	keymap.h \
	matcher.hh \
	ring.h \
	table.hh \
	vteaccess.h \
	vteconv.h \
	vtedefines.hh \
	vtedraw.hh \
	vtegtk.hh \
	vteinternal.hh \
	vterowdata.h \
	vteseq-list.hh \
	vtestream.h \
	vtestream-base.h \
	vtestream-file.h \
	vtetypes.hh \
	vteunistr.h \
	vteutils.h\
	box_drawing.txt \
	iso2022.txt \
	box_drawing_generate.sh \
	marshal.list \
	vtetypebuiltins.cc.template \
	vtetypebuiltins.h.template \
	vtetypebuiltins.h

BUILT_SOURCES = \
	box_drawing.h \
	marshal.cc \
	marshal.h \
	vtetypebuiltins.cc \
	vtetypebuiltins.h

box_drawing.h: box_drawing.txt box_drawing_generate.sh
	$(AM_V_GEN) $(srcdir)/box_drawing_generate.sh < $< > $@

marshal.cc: marshal.list
	$(AM_V_GEN) echo '#include "marshal.h"' > $@ \
	&& $(GLIB_GENMARSHAL) --prefix=_vte_marshal --body --internal $< >> $@

marshal.h: marshal.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=_vte_marshal --header --internal $< > $@

vtetypebuiltins.h: vtetypebuiltins.h.template vteenums.h vtedeprecated.h
	$(AM_V_GEN) $(GLIB_MKENUMS) --template $< $(filter %.h,$^) > xgen-vtbh \
	&& (cmp -s xgen-vtbh vtetypebuiltins.h || cp xgen-vtbh vtetypebuiltins.h ) \
	&& rm -f xgen-vtbh
	
vtetypebuiltins.cc: vtetypebuiltins.cc.template vteenums.h vtedeprecated.h
	$(AM_V_GEN) $(GLIB_MKENUMS) --template $< $(filter %.h,$^) > xgen-vtbc \
	&& (cmp -s xgen-vtbc vtetypebuiltins.cc || cp xgen-vtbc vtetypebuiltins.cc ) \
	&& rm -f xgen-vtbc
