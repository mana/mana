bin_PROGRAMS = tmw
tmw_SOURCES = sound/sound.cpp \
	graphic/2xsai.cpp \
	graphic/graphic.cpp \
	graphic/image.cpp \
	gui/button.cpp \
	gui/buy.cpp \
	gui/buysell.cpp \
	gui/char_server.cpp \
	gui/char_select.cpp \
	gui/chat.cpp \
	gui/checkbox.cpp \
	gui/gui.cpp \
	gui/inventory.cpp \
	gui/login.cpp \
	gui/npc.cpp \
	gui/ok_dialog.cpp \
	gui/passwordfield.cpp \
	gui/progressbar.cpp \
	gui/radiobutton.cpp \
	gui/scrollarea.cpp \
	gui/setup.cpp \
	gui/shop.cpp \
	gui/skill.cpp \
	gui/slider.cpp \
	gui/status.cpp \
	gui/stats.cpp \
	gui/textfield.cpp \
	gui/window.cpp \
	net/network.cpp \
	net/protocol.cpp \
	astar.cpp \
	being.cpp \
	configuration.cpp \
	game.cpp \
	log.cpp \
	main.cpp \
	map.cpp

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
tmw_LDFLAGS = $(all_libraries) $(LIBSDL_RPATH) $(LIBALLEGRO_RPATH)
tmw_CXXFLAGS = $(LIBSDL_CFLAGS) $(LIBALLEGRO_CFLAGS)
tmw_LDADD = $(LIBSDL_LIBS) $(LIBALLEGRO_LIBS)
tmw_TARGET = ../tmw
