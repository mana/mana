
iconsdir = $(pkgdatadir)/data/icons

icons_DATA = aethyra.png

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps
	$(INSTALL) $(srcdir)/aethyra.png $(DESTDIR)$(datadir)/pixmaps/aethyra.png

uninstall-local:
	rm -f $(DESTDIR)$(datadir)/pixmaps/aethyra.png

EXTRA_DIST =		\
	$(icons_DATA)
