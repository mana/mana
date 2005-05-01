
iconsdir = $(pkgdatadir)/data/icons

icons_DATA =		\
	tmw-icon.ico	\
	tmw-icon.png	\
	tmw-icon.xpm

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps
	$(INSTALL) $(srcdir)/tmw-icon.png $(DESTDIR)$(datadir)/pixmaps/tmw.png

uninstall-local:
	rm -f $(DESTDIR)$(datadir)/pixmaps/tmw.png

EXTRA_DIST =		\
	$(icons_DATA)
