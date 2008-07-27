
iconsdir = $(pkgdatadir)/data/icons

icons_DATA = tme.png

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps
	$(INSTALL) $(srcdir)/tme.png $(DESTDIR)$(datadir)/pixmaps/tme.png

uninstall-local:
	rm -f $(DESTDIR)$(datadir)/pixmaps/tme.png

EXTRA_DIST =		\
	$(icons_DATA)
