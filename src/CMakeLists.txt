include(FindPkgConfig)
pkg_check_modules(SDL2 REQUIRED
  sdl2>=2.0.10
  SDL2_image
  SDL2_mixer
  SDL2_net
  SDL2_ttf)
find_package(PhysFS REQUIRED)
find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(PNG REQUIRED)
find_package(Gettext REQUIRED)
find_package(Intl REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
  # Help getting compilation warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  if(WIN32)
    # This includes enough debug information to get something useful from Dr.
    # Mingw while keeping binary size down. Almost useless with gdb, though.
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb0 -gstabs2")
  endif()
endif()

add_compile_definitions(PACKAGE_VERSION="${PROJECT_VERSION}"
                        $<$<CONFIG:Debug>:DEBUG>)

if(WIN32)
  add_compile_definitions(PKG_DATADIR="${CMAKE_INSTALL_DATADIR}/"
                          LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}/")
elseif(NOT APPLE)
  add_compile_definitions(PKG_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}/"
                          LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}/")
endif()

if(ENABLE_MANASERV)
  if(NOT USE_SYSTEM_ENET)
    message(STATUS "Using ENet submodule")
    set(ENET_INCLUDE_DIR "../libs/enet/include")
    set(ENET_LIBRARIES enet)
    link_directories("../libs/enet")
  else()
    message(STATUS "Using system ENet")
    find_package(ENet REQUIRED)
  endif()

  add_compile_definitions(MANASERV_SUPPORT=1)
  message(STATUS "Manaserv support enabled")
endif()

if(APPLE OR NOT USE_SYSTEM_GUICHAN)
  message(STATUS "Using Guichan submodule")
  set(GUICHAN_INCLUDE_DIR "../libs/guichan/include")
  set(GUICHAN_LIBRARIES guichan)
  link_directories("../libs/guichan")
else()
  message(STATUS "Using system Guichan")
  find_package(Guichan REQUIRED)
endif()

if(ENABLE_NLS)
  add_compile_definitions(ENABLE_NLS=1)
  message(STATUS "i18n support enabled")
endif()

if(WITH_OPENGL)
  find_package(OpenGL REQUIRED)
  include_directories(${OPENGL_INCLUDE_DIR})
  add_compile_definitions(USE_OPENGL)
  message(STATUS "OpenGL support enabled")
endif(WITH_OPENGL)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SDL2_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIR}
  ${PHYSFS_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${ENET_INCLUDE_DIR}
  ${GUICHAN_INCLUDE_DIR}
  ${Intl_INCLUDE_DIRS})

set(SRCS
    gui/widgets/avatarlistbox.cpp
    gui/widgets/avatarlistbox.h
    gui/widgets/browserbox.cpp
    gui/widgets/browserbox.h
    gui/widgets/button.cpp
    gui/widgets/button.h
    gui/widgets/channeltab.cpp
    gui/widgets/channeltab.h
    gui/widgets/chattab.cpp
    gui/widgets/chattab.h
    gui/widgets/checkbox.cpp
    gui/widgets/checkbox.h
    gui/widgets/container.cpp
    gui/widgets/container.h
    gui/widgets/desktop.cpp
    gui/widgets/desktop.h
    gui/widgets/dropdown.cpp
    gui/widgets/dropdown.h
    gui/widgets/emoteshortcutcontainer.cpp
    gui/widgets/emoteshortcutcontainer.h
    gui/widgets/flowcontainer.cpp
    gui/widgets/flowcontainer.h
    gui/widgets/icon.cpp
    gui/widgets/icon.h
    gui/widgets/inttextfield.cpp
    gui/widgets/inttextfield.h
    gui/widgets/itemcontainer.cpp
    gui/widgets/itemcontainer.h
    gui/widgets/itemlinkhandler.cpp
    gui/widgets/itemlinkhandler.h
    gui/widgets/itemshortcutcontainer.cpp
    gui/widgets/itemshortcutcontainer.h
    gui/widgets/label.cpp
    gui/widgets/label.h
    gui/widgets/layout.cpp
    gui/widgets/layout.h
    gui/widgets/layouthelper.cpp
    gui/widgets/layouthelper.h
    gui/widgets/linkhandler.h
    gui/widgets/listbox.cpp
    gui/widgets/listbox.h
    gui/widgets/passwordfield.cpp
    gui/widgets/passwordfield.h
    gui/widgets/playerbox.cpp
    gui/widgets/playerbox.h
    gui/widgets/popup.cpp
    gui/widgets/popup.h
    gui/widgets/progressbar.cpp
    gui/widgets/progressbar.h
    gui/widgets/progressindicator.cpp
    gui/widgets/progressindicator.h
    gui/widgets/radiobutton.cpp
    gui/widgets/radiobutton.h
    gui/widgets/resizegrip.cpp
    gui/widgets/resizegrip.h
    gui/widgets/scrollarea.cpp
    gui/widgets/scrollarea.h
    gui/widgets/setuptab.cpp
    gui/widgets/setuptab.h
    gui/widgets/shopitems.cpp
    gui/widgets/shopitems.h
    gui/widgets/shoplistbox.cpp
    gui/widgets/shoplistbox.h
    gui/widgets/shortcutcontainer.cpp
    gui/widgets/shortcutcontainer.h
    gui/widgets/slider.cpp
    gui/widgets/slider.h
    gui/widgets/spacer.cpp
    gui/widgets/spacer.h
    gui/widgets/tab.cpp
    gui/widgets/tab.h
    gui/widgets/tabbedarea.cpp
    gui/widgets/tabbedarea.h
    gui/widgets/table.cpp
    gui/widgets/table.h
    gui/widgets/tablemodel.cpp
    gui/widgets/tablemodel.h
    gui/widgets/textbox.cpp
    gui/widgets/textbox.h
    gui/widgets/textfield.cpp
    gui/widgets/textfield.h
    gui/widgets/textpreview.cpp
    gui/widgets/textpreview.h
    gui/widgets/vertcontainer.cpp
    gui/widgets/vertcontainer.h
    gui/widgets/whispertab.cpp
    gui/widgets/whispertab.h
    gui/widgets/window.cpp
    gui/widgets/window.h
    gui/widgets/windowcontainer.cpp
    gui/widgets/windowcontainer.h
    gui/beingpopup.cpp
    gui/beingpopup.h
    gui/buydialog.cpp
    gui/buydialog.h
    gui/buyselldialog.cpp
    gui/buyselldialog.h
    gui/changeemaildialog.cpp
    gui/changeemaildialog.h
    gui/changepassworddialog.cpp
    gui/changepassworddialog.h
    gui/charselectdialog.cpp
    gui/charselectdialog.h
    gui/charcreatedialog.cpp
    gui/charcreatedialog.h
    gui/chatwindow.cpp
    gui/chatwindow.h
    gui/confirmdialog.cpp
    gui/confirmdialog.h
    gui/connectiondialog.cpp
    gui/connectiondialog.h
    gui/customserverdialog.cpp
    gui/customserverdialog.h
    gui/debugwindow.cpp
    gui/debugwindow.h
    gui/emotepopup.cpp
    gui/emotepopup.h
    gui/equipmentwindow.cpp
    gui/equipmentwindow.h
    gui/focushandler.cpp
    gui/focushandler.h
    gui/gui.cpp
    gui/gui.h
    gui/helpwindow.cpp
    gui/helpwindow.h
    gui/inventorywindow.cpp
    gui/inventorywindow.h
    gui/itempopup.cpp
    gui/itempopup.h
    gui/itemamountwindow.cpp
    gui/itemamountwindow.h
    gui/logindialog.cpp
    gui/logindialog.h
    gui/minimap.cpp
    gui/minimap.h
    gui/ministatuswindow.cpp
    gui/ministatuswindow.h
    gui/npcdialog.cpp
    gui/npcdialog.h
    gui/npcpostdialog.cpp
    gui/npcpostdialog.h
    gui/okdialog.cpp
    gui/okdialog.h
    gui/outfitwindow.cpp
    gui/outfitwindow.h
    gui/palette.cpp
    gui/palette.h
    gui/popupmenu.cpp
    gui/popupmenu.h
    gui/quitdialog.cpp
    gui/quitdialog.h
    gui/recorder.cpp
    gui/recorder.h
    gui/register.cpp
    gui/register.h
    gui/sdlinput.cpp
    gui/sdlinput.h
    gui/selldialog.cpp
    gui/selldialog.h
    gui/serverdialog.cpp
    gui/serverdialog.h
    gui/setup.cpp
    gui/setup.h
    gui/setup_audio.cpp
    gui/setup_audio.h
    gui/setup_colors.cpp
    gui/setup_colors.h
    gui/setup_interface.cpp
    gui/setup_interface.h
    gui/setup_joystick.cpp
    gui/setup_joystick.h
    gui/setup_keyboard.cpp
    gui/setup_keyboard.h
    gui/setup_players.cpp
    gui/setup_players.h
    gui/setup_video.cpp
    gui/setup_video.h
    gui/shortcutwindow.cpp
    gui/shortcutwindow.h
    gui/skilldialog.cpp
    gui/skilldialog.h
    gui/socialwindow.cpp
    gui/socialwindow.h
    gui/speechbubble.cpp
    gui/speechbubble.h
    gui/specialswindow.cpp
    gui/specialswindow.h
    gui/statuswindow.cpp
    gui/statuswindow.h
    gui/textdialog.cpp
    gui/textdialog.h
    gui/textpopup.cpp
    gui/textpopup.h
    gui/tradewindow.cpp
    gui/tradewindow.h
    gui/truetypefont.cpp
    gui/truetypefont.h
    gui/unregisterdialog.cpp
    gui/unregisterdialog.h
    gui/updaterwindow.cpp
    gui/updaterwindow.h
    gui/viewport.cpp
    gui/viewport.h
    gui/windowmenu.cpp
    gui/windowmenu.h
    gui/worldselectdialog.cpp
    gui/worldselectdialog.h
    net/adminhandler.h
    net/charhandler.cpp
    net/charhandler.h
    net/chathandler.h
    net/download.cpp
    net/download.h
    net/gamehandler.h
    net/generalhandler.h
    net/guildhandler.h
    net/inventoryhandler.h
    net/logindata.h
    net/loginhandler.h
    net/messagehandler.h
    net/npchandler.h
    net/net.cpp
    net/net.h
    net/partyhandler.h
    net/playerhandler.h
    net/serverinfo.h
    net/specialhandler.h
    net/tradehandler.h
    net/worldinfo.h
    resources/action.cpp
    resources/action.h
    resources/ambientlayer.cpp
    resources/ambientlayer.h
    resources/animation.cpp
    resources/animation.h
    resources/attributes.cpp
    resources/attributes.h
    resources/beinginfo.cpp
    resources/beinginfo.h
    resources/chardb.cpp
    resources/chardb.h
    resources/dye.cpp
    resources/dye.h
    resources/emotedb.cpp
    resources/emotedb.h
    resources/hairdb.cpp
    resources/hairdb.h
    resources/image.cpp
    resources/image.h
    resources/imageset.h
    resources/imageset.cpp
    resources/imagewriter.cpp
    resources/imagewriter.h
    resources/itemdb.cpp
    resources/itemdb.h
    resources/iteminfo.h
    resources/iteminfo.cpp
    resources/mapreader.cpp
    resources/mapreader.h
    resources/monsterdb.cpp
    resources/monsterdb.h
    resources/music.cpp
    resources/music.h
    resources/npcdb.cpp
    resources/npcdb.h
    resources/resource.cpp
    resources/resource.h
    resources/resourcemanager.cpp
    resources/resourcemanager.h
    resources/settingsmanager.cpp
    resources/settingsmanager.h
    resources/soundeffect.h
    resources/soundeffect.cpp
    resources/specialdb.cpp
    resources/specialdb.h
    resources/spritedef.h
    resources/spritedef.cpp
    resources/theme.cpp
    resources/theme.h
    resources/userpalette.cpp
    resources/userpalette.h
    resources/wallpaper.cpp
    resources/wallpaper.h
    utils/base64.cpp
    utils/base64.h
    utils/bufferedrwops.c
    utils/bufferedrwops.h
    utils/copynpaste.cpp
    utils/copynpaste.h
    utils/dtor.h
    utils/filesystem.h
    utils/gettext.h
    utils/mathutils.h
    utils/mkdir.cpp
    utils/mkdir.h
    utils/mutex.h
    utils/path.cpp
    utils/path.h
    utils/physfsrwops.c
    utils/physfsrwops.h
    utils/sha256.cpp
    utils/sha256.h
    utils/specialfolder.cpp
    utils/specialfolder.h
    utils/stringutils.cpp
    utils/stringutils.h
    utils/time.cpp
    utils/time.h
    utils/xml.cpp
    utils/xml.h
    utils/zlib.cpp
    utils/zlib.h
    actor.cpp
    actor.h
    actorsprite.cpp
    actorsprite.h
    actorspritemanager.cpp
    actorspritemanager.h
    animationparticle.cpp
    animationparticle.h
    avatar.cpp
    avatar.h
    being.cpp
    being.h
    channel.cpp
    channel.h
    channelmanager.cpp
    channelmanager.h
    chatlogger.cpp
    chatlogger.h
    client.cpp
    client.h
    commandhandler.cpp
    commandhandler.h
    compoundsprite.cpp
    compoundsprite.h
    configuration.cpp
    configuration.h
    defaults.cpp
    defaults.h
    effectmanager.cpp
    effectmanager.h
    emoteshortcut.cpp
    emoteshortcut.h
    equipment.h
    event.cpp
    event.h
    eventlistener.cpp
    eventlistener.h
    flooritem.cpp
    flooritem.h
    game.cpp
    game.h
    graphics.cpp
    graphics.h
    guichanfwd.h
    guild.cpp
    guild.h
    imageparticle.cpp
    imageparticle.h
    inventory.cpp
    inventory.h
    item.cpp
    item.h
    itemshortcut.cpp
    itemshortcut.h
    joystick.cpp
    joystick.h
    keyboardconfig.cpp
    keyboardconfig.h
    localplayer.cpp
    localplayer.h
    log.cpp
    log.h
    main.cpp
    main.h
    map.cpp
    map.h
    openglgraphics.cpp
    openglgraphics.h
    particle.cpp
    particle.h
    particlecontainer.cpp
    particlecontainer.h
    particleemitter.cpp
    particleemitter.h
    particleemitterprop.h
    party.cpp
    party.h
    playerinfo.cpp
    playerinfo.h
    playerrelations.cpp
    playerrelations.h
    position.cpp
    position.h
    properties.h
    rotationalparticle.cpp
    rotationalparticle.h
    sdlgraphics.cpp
    sdlgraphics.h
    shopitem.cpp
    shopitem.h
    simpleanimation.cpp
    simpleanimation.h
    sound.cpp
    sound.h
    sprite.cpp
    sprite.h
    statuseffect.cpp
    statuseffect.h
    text.cpp
    text.h
    textmanager.cpp
    textmanager.h
    textparticle.cpp
    textparticle.h
    textrenderer.h
    tileset.h
    units.cpp
    units.h
    variabledata.h
    vector.cpp
    vector.h
    video.cpp
    video.h)

set(SRCS_TMWA
    net/tmwa/gui/guildtab.cpp
    net/tmwa/gui/guildtab.h
    net/tmwa/gui/partytab.cpp
    net/tmwa/gui/partytab.h
    net/tmwa/adminhandler.cpp
    net/tmwa/adminhandler.h
    net/tmwa/beinghandler.cpp
    net/tmwa/beinghandler.h
    net/tmwa/buysellhandler.cpp
    net/tmwa/buysellhandler.h
    net/tmwa/charserverhandler.cpp
    net/tmwa/charserverhandler.h
    net/tmwa/chathandler.cpp
    net/tmwa/chathandler.h
    net/tmwa/gamehandler.cpp
    net/tmwa/gamehandler.h
    net/tmwa/generalhandler.cpp
    net/tmwa/generalhandler.h
    net/tmwa/guildhandler.cpp
    net/tmwa/guildhandler.h
    net/tmwa/inventoryhandler.cpp
    net/tmwa/inventoryhandler.h
    net/tmwa/itemhandler.cpp
    net/tmwa/itemhandler.h
    net/tmwa/loginhandler.cpp
    net/tmwa/loginhandler.h
    net/tmwa/messagehandler.cpp
    net/tmwa/messagehandler.h
    net/tmwa/messagein.cpp
    net/tmwa/messagein.h
    net/tmwa/messageout.cpp
    net/tmwa/messageout.h
    net/tmwa/network.cpp
    net/tmwa/network.h
    net/tmwa/npchandler.cpp
    net/tmwa/npchandler.h
    net/tmwa/partyhandler.cpp
    net/tmwa/partyhandler.h
    net/tmwa/playerhandler.cpp
    net/tmwa/playerhandler.h
    net/tmwa/protocol.h
    net/tmwa/specialhandler.cpp
    net/tmwa/specialhandler.h
    net/tmwa/token.h
    net/tmwa/tradehandler.cpp
    net/tmwa/tradehandler.h)

set(SRCS_MANA
    net/manaserv/adminhandler.cpp
    net/manaserv/adminhandler.h
    net/manaserv/beinghandler.cpp
    net/manaserv/beinghandler.h
    net/manaserv/buysellhandler.cpp
    net/manaserv/buysellhandler.h
    net/manaserv/charhandler.cpp
    net/manaserv/charhandler.h
    net/manaserv/chathandler.cpp
    net/manaserv/chathandler.h
    net/manaserv/connection.cpp
    net/manaserv/connection.h
    net/manaserv/defines.h
    net/manaserv/effecthandler.cpp
    net/manaserv/effecthandler.h
    net/manaserv/gamehandler.cpp
    net/manaserv/gamehandler.h
    net/manaserv/generalhandler.cpp
    net/manaserv/generalhandler.h
    net/manaserv/guildhandler.cpp
    net/manaserv/guildhandler.h
    net/manaserv/internal.cpp
    net/manaserv/internal.h
    net/manaserv/inventoryhandler.cpp
    net/manaserv/inventoryhandler.h
    net/manaserv/itemhandler.h
    net/manaserv/itemhandler.cpp
    net/manaserv/loginhandler.cpp
    net/manaserv/loginhandler.h
    net/manaserv/messagehandler.cpp
    net/manaserv/messagehandler.h
    net/manaserv/messagein.cpp
    net/manaserv/messagein.h
    net/manaserv/messageout.cpp
    net/manaserv/messageout.h
    net/manaserv/network.cpp
    net/manaserv/network.h
    net/manaserv/npchandler.cpp
    net/manaserv/npchandler.h
    net/manaserv/partyhandler.cpp
    net/manaserv/partyhandler.h
    net/manaserv/playerhandler.cpp
    net/manaserv/playerhandler.h
    net/manaserv/manaserv_protocol.h
    net/manaserv/specialhandler.cpp
    net/manaserv/specialhandler.h
    net/manaserv/tradehandler.cpp
    net/manaserv/tradehandler.h)

if(WIN32)
  configure_file(mana.rc.in mana.rc)
  set(SRCS ${SRCS} ${CMAKE_CURRENT_BINARY_DIR}/mana.rc)
endif(WIN32)

set(APP_ICON_NAME "mana.icns")
set(APP_ICON_FILE "${CMAKE_SOURCE_DIR}/data/icons/${APP_ICON_NAME}")
set_source_files_properties(${APP_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION
                                                        "Resources")

if(ENABLE_MANASERV)
  add_executable(mana WIN32 ${SRCS} ${SRCS_MANA} ${SRCS_TMWA} ${APP_ICON_FILE})
else(ENABLE_MANASERV)
  add_executable(mana WIN32 ${SRCS} ${SRCS_TMWA} ${APP_ICON_FILE})
endif(ENABLE_MANASERV)

target_link_libraries(
  mana
  PRIVATE ${ENET_LIBRARIES}
          ${SDL2_LINK_LIBRARIES}
          ${PNG_LIBRARIES}
          ${PHYSFS_LIBRARY}
	  CURL::libcurl
          ${LIBXML2_LIBRARIES}
          ${GUICHAN_LIBRARIES}
          ${OPENGL_LIBRARIES}
          ${Intl_LIBRARIES})

set_target_properties(
  mana
  PROPERTIES MACOSX_BUNDLE TRUE
             MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
             MACOSX_BUNDLE_GUI_IDENTIFIER org.manasource.Mana
             MACOSX_BUNDLE_ICON_FILE ${APP_ICON_NAME}
             MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
             MACOSX_BUNDLE_COPYRIGHT "(C) 2004-2024 ${CPACK_PACKAGE_VENDOR}"
             MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
             MACOSX_BUNDLE_SHORT_VERSION_STRING
             ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

install(
  TARGETS mana
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(APPLE)
  target_link_libraries(mana PRIVATE "-framework Foundation")
  set_target_properties(mana PROPERTIES OUTPUT_NAME Mana)

  set(CPACK_GENERATOR "DRAGNDROP")

  install(
    CODE "include(BundleUtilities)
      fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/Mana.app\" \"\" \"/usr/local/lib\")"
  )
endif()
