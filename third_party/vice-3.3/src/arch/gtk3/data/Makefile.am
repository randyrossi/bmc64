gtk3datadir = $(VICEDIR)/gui

GRESOURCE_TARGET = $(builddir)/vice.gresource
GRESOURCE_PROTO = $(srcdir)//vice.gresource.xml.proto
GRESOURCE_XML = $(builddir)/vice.gresource.xml

all: $(GRESOURCE_TARGET)

$(GRESOURCE_XML): $(GRESOURCE_PROTO)
	cat $(GRESOURCE_PROTO) | sed -e "s@VICE_SRC_DIR@$(srcdir)@g" \
		> $(GRESOURCE_XML)

$(GRESOURCE_TARGET): $(GRESOURCE_XML)
	glib-compile-resources $< --target $@

EXTRA_DIST = $(GRESOURCE_PROTO)

gtk3data_DATA = $(GRESOURCE_TARGET)

.PHONY: clean
clean:
	rm -f $(GRESOURCE_TARGET)
	rm -f $(GRESOURCE_XML)

.PHONY: distclean
distclean: clean

# vim: set noet ts=8 sts=8 sw=8:
