bin_PROGRAMS = tmw
tmw_SOURCES = sound/sound.cpp \
	graphic/graphic.cpp \
	graphic/spriteset.cpp \
	gui/button.cpp \
	gui/buy.cpp \
	gui/buysell.cpp \
	gui/char_server.cpp \
	gui/char_select.cpp \
	gui/chat.cpp \
	gui/checkbox.cpp \
	gui/confirm_dialog.cpp \
	gui/equipment.cpp \
	gui/gui.cpp \
	gui/inventory.cpp \
	gui/itemcontainer.cpp \
	gui/listbox.cpp \
	gui/login.cpp \
	gui/minimap.cpp \
	gui/npc.cpp \
	gui/npc_text.cpp \
	gui/ok_dialog.cpp \
	gui/passwordfield.cpp \
	gui/playerbox.cpp \
	gui/progressbar.cpp \
	gui/radiobutton.cpp \
	gui/scrollarea.cpp \
	gui/sell.cpp \
	gui/setup.cpp \
	gui/shop.cpp \
	gui/skill.cpp \
	gui/slider.cpp \
	gui/status.cpp \
	gui/stats.cpp \
	gui/textbox.cpp \
	gui/textfield.cpp \
	gui/window.cpp \
	gui/windowcontainer.cpp \
	net/network.cpp \
	net/protocol.cpp \
	resources/image.cpp \
	resources/resource.cpp \
	resources/resourcemanager.cpp \
	astar.cpp \
	base64.cpp \
	being.cpp \
	configuration.cpp \
	game.cpp \
	log.cpp \
	main.cpp \
	map.cpp

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
tmw_LDFLAGS = $(all_libraries) $(LIBSDL_RPATH) -lguichan -lguichan_sdl
tmw_CXXFLAGS = $(LIBSDL_CFLAGS)
tmw_LDADD = $(LIBSDL_LIBS)
tmw_TARGET = ../tmw
