
iconsdir = $(pkgdatadir)/data/icons

icons_DATA = tmw.png \
	     tmw.xpm

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps
	$(INSTALL) $(srcdir)/tmw.png $(DESTDIR)$(datadir)/pixmaps/tmw.png

uninstall-local:
	rm -f $(DESTDIR)$(datadir)/pixmaps/tmw.png

EXTRA_DIST =		\
	$(icons_DATA)
