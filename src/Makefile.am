bin_PROGRAMS = tmw
tmw_SOURCES = graphic/graphic.cpp \
	      graphic/graphic.h \
	      graphic/spriteset.cpp \
	      graphic/spriteset.h \
	      gui/button.cpp \
	      gui/button.h \
	      gui/buy.cpp \
	      gui/buy.h \
	      gui/buysell.cpp \
	      gui/buysell.h \
	      gui/chargedialog.cpp \
	      gui/chargedialog.h \
	      gui/char_server.cpp \
	      gui/char_server.h \
	      gui/char_select.cpp \
	      gui/char_select.h \
	      gui/chat.cpp \
	      gui/chat.h \
	      gui/checkbox.cpp \
	      gui/checkbox.h \
	      gui/confirm_dialog.cpp \
	      gui/confirm_dialog.h \
	      gui/equipment.cpp \
	      gui/equipment.h \
	      gui/gui.cpp \
	      gui/gui.h \
	      gui/inventory.cpp \
	      gui/inventory.h \
	      gui/itemcontainer.cpp \
	      gui/itemcontainer.h \
	      gui/listbox.cpp \
	      gui/listbox.h \
	      gui/login.cpp \
	      gui/login.h \
	      gui/minimap.cpp \
	      gui/minimap.h \
	      gui/newskill.cpp \
	      gui/newskill.h \
	      gui/npc.cpp \
	      gui/npc.h \
	      gui/npc_text.cpp \
	      gui/npc_text.h \
	      gui/ok_dialog.cpp \
	      gui/ok_dialog.h \
	      gui/passwordfield.cpp \
	      gui/passwordfield.h \
	      gui/playerbox.cpp \
	      gui/playerbox.h \
	      gui/progressbar.cpp \
	      gui/progressbar.h \
	      gui/radiobutton.cpp \
	      gui/radiobutton.h \
	      gui/scrollarea.cpp \
	      gui/scrollarea.h \
	      gui/sell.cpp \
	      gui/sell.h \
	      gui/setup.cpp \
	      gui/setup.h \
	      gui/shop.cpp \
	      gui/shop.h \
	      gui/skill.cpp \
	      gui/skill.h \
	      gui/slider.cpp \
	      gui/slider.h \
	      gui/status.cpp \
	      gui/status.h \
	      gui/stats.cpp \
	      gui/stats.h \
	      gui/textbox.cpp \
	      gui/textbox.h \
	      gui/textfield.cpp \
	      gui/textfield.h \
	      gui/window.cpp \
	      gui/window.h \
	      gui/windowcontainer.cpp \
	      gui/windowcontainer.h \
	      net/network.cpp \
	      net/network.h \
	      net/protocol.cpp \
	      net/protocol.h \
	      net/win2linux.h \
	      resources/image.cpp \
	      resources/image.h \
	      resources/mapreader.cpp \
	      resources/mapreader.h \
	      resources/resource.cpp \
	      resources/resource.h \
	      resources/resourcemanager.cpp \
	      resources/resourcemanager.h \
	      base64.cpp \
	      base64.h \
	      being.cpp \
	      being.h \
	      configuration.cpp \
	      configuration.h \
	      game.cpp \
	      game.h \
	      log.cpp \
	      log.h \
	      main.cpp \
	      main.h \
	      map.cpp\
	      map.h \
	      sound.cpp \
	      sound.h

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
tmw_LDFLAGS = $(all_libraries) $(LIBSDL_RPATH) -lguichan_sdl -lguichan `pkg-config --libs libxml-2.0`
tmw_CXXFLAGS = -Wall $(LIBSDL_CFLAGS) `pkg-config --cflags libxml-2.0`
tmw_LDADD = $(LIBSDL_LIBS) -lguichan_opengl -lguichan -lGL -lphysfs
tmw_TARGET = ../tmw
