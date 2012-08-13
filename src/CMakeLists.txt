FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(SDL_mixer REQUIRED)
FIND_PACKAGE(SDL_net REQUIRED)
FIND_PACKAGE(SDL_ttf REQUIRED)
FIND_PACKAGE(SDL_gfx REQUIRED)
FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(PhysFS REQUIRED)
FIND_PACKAGE(PNG REQUIRED)
FIND_PACKAGE(Gettext REQUIRED)

IF (CMAKE_COMPILER_IS_GNUCXX)
    # Help getting compilation warnings
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    IF (WIN32)
        # This includes enough debug information to get something useful
        # from Dr. Mingw while keeping binary size down. Almost useless
        # with gdb, though.
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb0 -gstabs2")
    ENDIF()
ENDIF()

IF (POLICY CMP0015)
    CMAKE_POLICY(SET CMP0015 OLD)
ENDIF()

IF (ENABLE_MANASERV)
    INCLUDE_DIRECTORIES("../libs/enet/include")
    LINK_DIRECTORIES("../libs/enet")
    SET(INTERNAL_LIBRARIES enet)
ENDIF()

SET(FLAGS "-DPACKAGE_VERSION=\\\"${VERSION}\\\"")
SET(FLAGS "${FLAGS} -DPKG_DATADIR=\\\"${PKG_DATADIR}/\\\"")
SET(FLAGS "${FLAGS} -DLOCALEDIR=\\\"${LOCALEDIR}/\\\"")
IF (ENABLE_CPP0X)
    SET(FLAGS "${FLAGS} -std=c++0x -DENABLE_CPP0X=1")
ELSE()
    INCLUDE_DIRECTORIES("cpp0x_compat")
ENDIF()

IF (ENABLE_NLS)
    SET(FLAGS "${FLAGS} -DENABLE_NLS=1")
    MESSAGE(STATUS "i18n support enabled")
ENDIF()

IF (ENABLE_MANASERV)
    SET(FLAGS "${FLAGS} -DMANASERV_SUPPORT=1")
    MESSAGE(STATUS "Manaserv support enabled")
ENDIF()

IF (CMAKE_BUILD_TYPE)
    STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
    IF(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug OR
       CMAKE_BUILD_TYPE_TOLOWER MATCHES relwithdebinfo)
        SET(FLAGS "${FLAGS} -DDEBUG")
        MESSAGE(STATUS "Set Debug build type")
    ENDIF()
ENDIF()

IF (WIN32)
    SET(EXTRA_LIBRARIES ws2_32 winmm)
    FIND_PACKAGE(LibIntl REQUIRED)
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR BEOS)
    FIND_PACKAGE(LibIntl REQUIRED)
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL SunOS)
    # explicit linking to libintl is required on Solaris
    SET(EXTRA_LIBRARIES intl)
ENDIF()

SET(GUICHAN_COMPONENTS "SDL")
FIND_PACKAGE(Guichan REQUIRED ${GUICHAN_COMPONENTS})

IF (WITH_OPENGL)
    FIND_PACKAGE(OpenGL REQUIRED)
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
    SET(FLAGS "${FLAGS} -DUSE_OPENGL")
    MESSAGE(STATUS "OpenGL support enabled")
ENDIF (WITH_OPENGL)

IF (USE_X11)
    FIND_PACKAGE(X11 REQUIRED)
    INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
    SET(FLAGS "${FLAGS} -DUSE_X11")
ENDIF (USE_X11)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL_INCLUDE_DIR}
    ${SDLIMAGE_INCLUDE_DIR}
    ${SDLMIXER_INCLUDE_DIR}
    ${SDLNET_INCLUDE_DIR}
    ${SDLTTF_INCLUDE_DIR}
    ${SDLGFX_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${PHYSFS_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${GUICHAN_INCLUDE_DIR}
    )

# enable rc-handling with mingw
# most likely this part can be kicked out with some later cmake version
IF (MINGW)
    FIND_PATH(MINGW_INCLUDE_DIR windows.h $ENV{INCLUDE})
    IF (MINGW_INCLUDE_DIR)
        MESSAGE(STATUS "Found mingw headers: ${MINGW_INCLUDE_DIR}")
        INCLUDE_DIRECTORIES(${MINGW_INCLUDE_DIR})
    ELSE()
        MESSAGE(FATAL_ERROR "Unable to find mingw headers. Required for windres")
    ENDIF()
    SET(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT
      "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -O coff -o <OBJECT> <SOURCE>")
ENDIF()

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
    gui/minimapwindow.cpp
    gui/minimapwindow.h
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
    resources/beinginfo.cpp
    resources/beinginfo.h
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
    utils/copynpaste.cpp
    utils/copynpaste.h
    utils/dtor.h
    utils/gettext.h
    utils/mathutils.h
    utils/physfsrwops.c
    utils/physfsrwops.h
    utils/sha256.cpp
    utils/sha256.h
    utils/stringutils.cpp
    utils/stringutils.h
    utils/mutex.h
    utils/mkdir.cpp
    utils/mkdir.h
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
    animatedsprite.cpp
    animatedsprite.h
    animationparticle.cpp
    animationparticle.h
    avatar.cpp
    avatar.h
    being.cpp
    being.h
    chatlogger.cpp
    chatlogger.h
    client.cpp
    client.h
    channel.cpp
    channel.h
    channelmanager.cpp
    channelmanager.h
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
    imagesprite.cpp
    imagesprite.h
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
    shopitem.cpp
    shopitem.h
    simpleanimation.cpp
    simpleanimation.h
    sound.cpp
    sound.h
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
    )

SET(SRCS_TMWA
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
    net/tmwa/tradehandler.h
    )

SET(SRCS_MANA
    net/manaserv/adminhandler.cpp
    net/manaserv/adminhandler.h
    net/manaserv/attributes.cpp
    net/manaserv/attributes.h
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
    net/manaserv/tradehandler.h
    )

IF (WIN32)
    SET(SRCS
        ${SRCS}
        utils/specialfolder.cpp
        utils/specialfolder.h
        mana.rc
    )
ENDIF (WIN32)

IF (APPLE)
    SET(SRCS
        ${SRCS}
        log.mm
        SDLMain.h
        SDLMain.m
    )
ENDIF (APPLE)

SET (PROGRAMS mana)

IF (ENABLE_MANASERV)
    ADD_EXECUTABLE(mana WIN32 ${SRCS} ${SRCS_MANA} ${SRCS_TMWA})
ELSE(ENABLE_MANASERV)
    ADD_EXECUTABLE(mana WIN32 ${SRCS} ${SRCS_TMWA})
ENDIF(ENABLE_MANASERV)

TARGET_LINK_LIBRARIES(mana
    ${INTERNAL_LIBRARIES}
    ${SDLGFX_LIBRARIES}
    ${SDL_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLNET_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${PNG_LIBRARIES}
    ${PHYSFS_LIBRARY}
    ${CURL_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${GUICHAN_LIBRARIES}
    ${X11_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${LIBINTL_LIBRARIES}
    ${EXTRA_LIBRARIES})

INSTALL(TARGETS mana RUNTIME DESTINATION ${PKG_BINDIR})

IF (CMAKE_SYSTEM_NAME STREQUAL SunOS)
    # we expect the SMCgtxt package to be present on Solaris;
    # the Solaris gettext is not API-compatible to GNU gettext
    SET_TARGET_PROPERTIES(mana PROPERTIES LINK_FLAGS "-L/usr/local/lib")
ENDIF()

SET_TARGET_PROPERTIES(mana PROPERTIES COMPILE_FLAGS "${FLAGS}")
