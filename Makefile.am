# Subdirectories to build
SUBDIRS = data docs po src

desktopdir = $(datarootdir)/applications

desktop_DATA = mana.desktop

# Extra files to include
EXTRA_DIST = $(desktop_DATA) \
	     CMakeLists.txt \
	     po/CMakeLists.txt \
	     CMake/Modules/FindENet.cmake \
	     CMake/Modules/FindGuichan.cmake \
	     CMake/Modules/FindLibIntl.cmake \
	     CMake/Modules/FindSDL_gfx.cmake \
	     mana.cbp \
	     mana.menu

# Autopoint m4 stuff
ACLOCAL_AMFLAGS = -I m4
