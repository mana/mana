
iconsdir = $(pkgdatadir)/data/icons

icons_DATA = mana.png \
	     mana.xpm

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datarootdir)/pixmaps
	$(INSTALL) $(srcdir)/mana.png $(DESTDIR)$(datarootdir)/pixmaps/mana.png

uninstall-local:
	rm -f $(DESTDIR)$(datarootdir)/pixmaps/mana.png

EXTRA_DIST =		\
	$(icons_DATA)
