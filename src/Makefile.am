bin_PROGRAMS = tmw
tmw_SOURCES = sound/sound.cpp \
	graphic/2xsai.cpp \
	graphic/graphic.cpp \
	gui/button.cpp \
	gui/chat.cpp \
	gui/checkbox.cpp \
	gui/skill.cpp \
	gui/shop.cpp \
	gui/stats.cpp \
	gui/npc.cpp \
	gui/setup.cpp \
	gui/gui.cpp \
	gui/login.cpp \
	gui/char_server.cpp \
	gui/char_select.cpp \
	gui/inventory.cpp \
	gui/window.cpp \
	net/network.cpp \
	net/protocol.cpp \
	being.cpp \
	game.cpp \
	main.cpp \
	map.cpp \
	log.cpp \
	astar.cpp

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
tmw_LDFLAGS = $(all_libraries) $(LIBSDL_RPATH) $(LIBALLEGRO_RPATH)
tmw_CXXFLAGS = $(LIBSDL_CFLAGS) $(LIBALLEGRO_CFLAGS) -Wno-deprecated
tmw_LDADD = $(LIBSDL_LIBS) $(LIBALLEGRO_LIBS)
tmw_TARGET = ../tmw
