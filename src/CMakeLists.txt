FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(SDL_mixer REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)

FIND_PACKAGE(Libcurl REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(PhysFS REQUIRED)
FIND_PACKAGE(PNG REQUIRED)

SET(FLAGS "-DPACKAGE_VERSION=\\\"${VERSION}\\\"")
SET(FLAGS "${FLAGS} -DTMW_DATADIR=\\\"${PKG_DATADIR}/\\\"")

SET(GUICHAN_COMPONENTS "SDL")
IF (WITH_OPENGL)
    SET(GUICHAN_COMPONENTS ${GUICHAN_COMPONENTS} "OpenGL")
ENDIF (WITH_OPENGL)
FIND_PACKAGE(Guichan REQUIRED ${GUICHAN_COMPONENTS})

IF (WITH_OPENGL)
    FIND_PACKAGE(OpenGL REQUIRED)
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
    SET(FLAGS "${FLAGS} -DUSE_OPENGL")
ENDIF (WITH_OPENGL)

INCLUDE_DIRECTORIES(
    ${CURRENT_SOURCE_DIR}
    ${SDL_INCLUDE_DIR}
    ${SDLIMAGE_INCLUDE_DIR}
    ${SDLMIXER_INCLUDE_DIR}
    ${SDLNET_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${PHYSFS_INCLUDE_DIR}
    ${LIBCURL_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${GUICHAN_INCLUDE_DIR}
    )

# Fix some stuff that gets not hidden by mainline modules
MARK_AS_ADVANCED(PHYSFS_INCLUDE_DIR)
MARK_AS_ADVANCED(PHYSFS_LIBRARY)
MARK_AS_ADVANCED(SDLIMAGE_INCLUDE_DIR)
MARK_AS_ADVANCED(SDLIMAGE_LIBRARY)
MARK_AS_ADVANCED(SDLMAIN_LIBRARY)
MARK_AS_ADVANCED(SDLMIXER_INCLUDE_DIR)
MARK_AS_ADVANCED(SDLMIXER_LIBRARY)
MARK_AS_ADVANCED(SDLNET_INCLUDE_DIR)
MARK_AS_ADVANCED(SDLNET_LIBRARY)
MARK_AS_ADVANCED(SDL_INCLUDE_DIR)
MARK_AS_ADVANCED(SDL_LIBRARY)

SET(SRCS
    gui/widgets/resizegrip.cpp
    gui/widgets/resizegrip.h
    gui/browserbox.cpp
    gui/browserbox.h
    gui/buddywindow.cpp
    gui/buddywindow.h
    gui/button.cpp
    gui/button.h
    gui/buy.cpp
    gui/buy.h
    gui/buysell.cpp
    gui/buysell.h
    gui/chargedialog.cpp
    gui/chargedialog.h
    gui/char_server.cpp
    gui/char_server.h
    gui/char_select.cpp
    gui/char_select.h
    gui/chat.cpp
    gui/chat.h
    gui/chatinput.cpp
    gui/chatinput.h
    gui/checkbox.cpp
    gui/checkbox.h
    gui/confirm_dialog.cpp
    gui/confirm_dialog.h
    gui/connection.cpp
    gui/connection.h
    gui/debugwindow.cpp
    gui/debugwindow.h
    gui/equipmentwindow.cpp
    gui/equipmentwindow.h
    gui/focushandler.cpp
    gui/focushandler.h
    gui/gccontainer.cpp
    gui/gccontainer.h
    gui/gui.cpp
    gui/gui.h
    gui/help.cpp
    gui/help.h
    gui/inventorywindow.cpp
    gui/inventorywindow.h
    gui/itemcontainer.cpp
    gui/itemcontainer.h
    gui/item_amount.cpp
    gui/item_amount.h
    gui/linkhandler.h
    gui/listbox.cpp
    gui/listbox.h
    gui/login.cpp
    gui/login.h
    gui/menuwindow.cpp
    gui/menuwindow.h
    gui/minimap.cpp
    gui/minimap.h
    gui/ministatus.cpp
    gui/ministatus.h
    gui/newskill.cpp
    gui/newskill.h
    gui/npclistdialog.cpp
    gui/npclistdialog.h
    gui/npc_text.cpp
    gui/npc_text.h
    gui/ok_dialog.cpp
    gui/ok_dialog.h
    gui/passwordfield.cpp
    gui/passwordfield.h
    gui/playerbox.cpp
    gui/playerbox.h
    gui/popupmenu.cpp
    gui/popupmenu.h
    gui/progressbar.cpp
    gui/progressbar.h
    gui/radiobutton.cpp
    gui/radiobutton.h
    gui/register.cpp
    gui/register.h
    gui/scrollarea.cpp
    gui/scrollarea.h
    gui/selectionlistener.h
    gui/sell.cpp
    gui/sell.h
    gui/setup_audio.cpp
    gui/setup_audio.h
    gui/setup.cpp
    gui/setup.h
    gui/setup_joystick.cpp
    gui/setup_joystick.h
    gui/setuptab.h
    gui/setup_video.cpp
    gui/setup_video.h
    gui/shop.cpp
    gui/shop.h
    gui/shoplistbox.cpp
    gui/shoplistbox.h
    gui/skill.cpp
    gui/skill.h
    gui/slider.cpp
    gui/slider.h
    gui/status.cpp
    gui/status.h
    gui/tabbedcontainer.cpp
    gui/tabbedcontainer.h
    gui/textbox.cpp
    gui/textbox.h
    gui/textfield.cpp
    gui/textfield.h
    gui/trade.cpp
    gui/trade.h
    gui/viewport.cpp
    gui/viewport.h
    gui/window.cpp
    gui/window.h
    gui/windowcontainer.cpp
    gui/windowcontainer.h
    gui/inttextbox.h
    gui/inttextbox.cpp
    gui/box.h
    gui/box.cpp
    gui/vbox.h
    gui/vbox.cpp
    gui/hbox.h
    gui/hbox.cpp
    gui/updatewindow.h
    gui/updatewindow.cpp
    net/beinghandler.h
    net/beinghandler.cpp
    net/buysellhandler.h
    net/buysellhandler.cpp
    net/charserverhandler.h
    net/charserverhandler.cpp
    net/chathandler.h
    net/chathandler.cpp
    net/equipmenthandler.h
    net/equipmenthandler.cpp
    net/inventoryhandler.h
    net/inventoryhandler.cpp
    net/itemhandler.h
    net/itemhandler.cpp
    net/loginhandler.h
    net/loginhandler.cpp
    net/maploginhandler.cpp
    net/maploginhandler.h
    net/messagehandler.cpp
    net/messagehandler.h
    net/messagein.cpp
    net/messagein.h
    net/messageout.cpp
    net/messageout.h
    net/network.cpp
    net/network.h
    net/npchandler.cpp
    net/npchandler.h
    net/playerhandler.cpp
    net/playerhandler.h
    net/protocol.cpp
    net/protocol.h
    net/skillhandler.cpp
    net/skillhandler.h
    net/tradehandler.cpp
    net/tradehandler.h
    resources/action.cpp
    resources/action.h
    resources/ambientoverlay.cpp
    resources/ambientoverlay.h
    resources/animation.cpp
    resources/animation.h
    resources/equipmentdb.cpp
    resources/equipmentdb.h
    resources/equipmentinfo.h
    resources/image.cpp
    resources/image.h
    resources/imagewriter.cpp
    resources/imagewriter.h
    resources/itemdb.cpp
    resources/itemdb.h
    resources/iteminfo.h
    resources/iteminfo.cpp
    resources/mapreader.cpp
    resources/mapreader.h
    resources/monsterdb.h
    resources/monsterdb.cpp
    resources/monsterinfo.h
    resources/monsterinfo.cpp
    resources/music.h
    resources/music.cpp
    resources/openglsdlimageloader.h
    resources/openglsdlimageloader.cpp
    resources/resource.cpp
    resources/resource.h
    resources/resourcemanager.cpp
    resources/resourcemanager.h
    resources/sdlimageloader.h
    resources/sdlimageloader.cpp
    resources/soundeffect.h
    resources/soundeffect.cpp
    resources/spritedef.h
    resources/spritedef.cpp
    resources/imageset.h
    resources/imageset.cpp
    resources/buddylist.h
    resources/buddylist.cpp
    utils/base64.cpp
    utils/base64.h
    utils/dtor.h
    utils/fastsqrt.h
    utils/minmax.h
    utils/tostring.h
    utils/xml.cpp
    utils/xml.h
    animatedsprite.cpp
    animatedsprite.h
    animationparticle.cpp
    animationparticle.h
    being.cpp
    being.h
    beingmanager.cpp
    beingmanager.h
    configlistener.h
    configuration.cpp
    configuration.h
    engine.cpp
    engine.h
    equipment.cpp
    equipment.h
    floor_item.cpp
    floor_item.h
    flooritemmanager.cpp
    flooritemmanager.h
    game.cpp
    game.h
    graphics.cpp
    graphics.h
    guichanfwd.h
    imageparticle.cpp
    imageparticle.h
    inventory.cpp
    inventory.h
    item.cpp
    item.h
    joystick.cpp
    joystick.h
    localplayer.cpp
    localplayer.h
    lockedarray.h
    log.cpp
    log.h
    logindata.h
    main.cpp
    main.h
    map.cpp
    map.h
    monster.cpp
    monster.h
    npc.cpp
    npc.h
    openglgraphics.cpp
    openglgraphics.h
    particle.cpp
    particle.h
    particleemitter.cpp
    particleemitter.h
    player.cpp
    player.h
    properties.h
    serverinfo.h
    simpleanimation.cpp
    simpleanimation.h
    sound.cpp
    sound.h
    sprite.h
    textparticle.cpp
    textparticle.h
    tileset.h
    )

ADD_EXECUTABLE(tmw ${SRCS})

TARGET_LINK_LIBRARIES(tmw
    ${SDL_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLNET_LIBRARY}
    ${PNG_LIBRARIES}
    ${PHYSFS_LIBRARY}
    ${LIBCURL_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${GUICHAN_LIBRARIES}
    ${OPENGL_LIBRARIES}
    )

INSTALL(TARGETS tmw RUNTIME DESTINATION ${PKG_BINDIR})

SET_TARGET_PROPERTIES(tmw PROPERTIES COMPILE_FLAGS "${FLAGS}")
