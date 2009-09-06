
iconsdir = $(pkgdatadir)/data/icons

icons_DATA = tmw.png \
	     tmw.xpm

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datarootdir)/pixmaps
	$(INSTALL) $(srcdir)/tmw.png $(DESTDIR)$(datarootdir)/pixmaps/tmw.png

uninstall-local:
	rm -f $(DESTDIR)$(datarootdir)/pixmaps/tmw.png

EXTRA_DIST =		\
	$(icons_DATA)
