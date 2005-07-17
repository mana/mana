AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = tmw
tmw_SOURCES = graphic/spriteset.cpp \
	      graphic/spriteset.h \
	      gui/browserbox.cpp \
	      gui/browserbox.h \
	      gui/buddywindow.cpp \
	      gui/buddywindow.h \
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
	      gui/chatinput.cpp \
	      gui/chatinput.h \
	      gui/checkbox.cpp \
	      gui/checkbox.h \
	      gui/confirm_dialog.cpp \
	      gui/confirm_dialog.h \
	      gui/equipment.cpp \
	      gui/equipment.h \
	      gui/focushandler.cpp \
	      gui/focushandler.h \
	      gui/gui.cpp \
	      gui/gui.h \
	      gui/help.cpp \
	      gui/help.h \
	      gui/inventory.cpp \
	      gui/inventory.h \
	      gui/itemcontainer.cpp \
	      gui/itemcontainer.h \
	      gui/item_amount.cpp \
	      gui/item_amount.h \
	      gui/linkhandler.h \
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
	      gui/popupmenu.cpp \
	      gui/popupmenu.h \
	      gui/progressbar.cpp \
	      gui/progressbar.h \
	      gui/radiobutton.cpp \
	      gui/radiobutton.h \
	      gui/requesttrade.cpp \
	      gui/requesttrade.h \
	      gui/scrollarea.cpp \
	      gui/scrollarea.h \
	      gui/sell.cpp \
	      gui/sell.h \
	      gui/setup.cpp \
	      gui/setup.h \
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
	      gui/trade.cpp \
	      gui/trade.h \
	      gui/window.cpp \
	      gui/window.h \
	      gui/windowcontainer.cpp \
	      gui/windowcontainer.h \
	      gui/inttextbox.h \
	      gui/inttextbox.cpp \
	      gui/box.h \
	      gui/box.cpp \
	      gui/vbox.h \
	      gui/vbox.cpp \
	      gui/hbox.h \
	      gui/hbox.cpp \
	      gui/updatewindow.h \
	      gui/updatewindow.cpp \
	      net/network.cpp \
	      net/network.h \
	      net/protocol.cpp \
	      net/protocol.h \
	      net/win2linux.h \
	      resources/image.cpp \
	      resources/image.h \
	      resources/iteminfo.cpp \
	      resources/iteminfo.h \
	      resources/itemmanager.cpp \
	      resources/itemmanager.h \
	      resources/mapreader.cpp \
	      resources/mapreader.h \
	      resources/music.h \
	      resources/music.cpp \
	      resources/resource.cpp \
	      resources/resource.h \
	      resources/resourcemanager.cpp \
	      resources/resourcemanager.h \
	      resources/soundeffect.h \
	      resources/soundeffect.cpp \
	      resources/buddylist.h \
	      resources/buddylist.cpp \
	      base64.cpp \
	      base64.h \
	      being.cpp \
	      being.h \
	      configuration.cpp \
	      configuration.h \
	      engine.cpp \
	      engine.h \
	      equipment.cpp \
	      equipment.h \
	      floor_item.cpp \
	      floor_item.h \
	      game.cpp \
	      game.h \
	      graphics.cpp \
	      graphics.h \
	      item.cpp \
	      item.h \
	      log.cpp \
	      log.h \
	      main.cpp \
	      main.h \
	      map.cpp\
	      map.h \
	      playerinfo.h \
	      serverinfo.h \
	      sound.cpp \
	      sound.h

# set the include path found by configure
INCLUDES =					\
	$(all_includes)				\
	-DTMW_DATADIR=\""$(pkgdatadir)/"\"

# the library search path.
tmw_LDFLAGS = $(all_libraries) $(LIBSDL_RPATH) `pkg-config --libs libxml-2.0`
tmw_CXXFLAGS = -Wall $(OPENGL_CFLAGS) $(LIBSDL_CFLAGS) `pkg-config --cflags libxml-2.0` $(CURL_CFLAGS)
tmw_LDADD = $(LIBSDL_LIBS) -lguichan_sdl -lguichan $(OPENGL_LIBS) -lphysfs $(CURL_LIBS)
tmw_TARGET = tmw
