/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "client.h"
#include "main.h"

#include "chatlogger.h"
#include "configuration.h"
#include "emoteshortcut.h"
#include "event.h"
#include "game.h"
#include "itemshortcut.h"
#include "keyboardconfig.h"
#ifdef USE_OPENGL
#include "openglgraphics.h"
#endif
#include "playerrelations.h"
#include "sound.h"
#include "statuseffect.h"
#include "units.h"

#include "gui/changeemaildialog.h"
#include "gui/changepassworddialog.h"
#include "gui/charselectdialog.h"
#include "gui/connectiondialog.h"
#include "gui/gui.h"
#include "gui/logindialog.h"
#include "gui/okdialog.h"
#include "gui/quitdialog.h"
#include "gui/register.h"
#include "gui/sdlinput.h"
#include "gui/serverdialog.h"
#include "gui/setup.h"
#include "gui/unregisterdialog.h"
#include "gui/updaterwindow.h"
#include "gui/worldselectdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/desktop.h"

#include "net/charhandler.h"
#include "net/gamehandler.h"
#include "net/generalhandler.h"
#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/net.h"
#include "net/worldinfo.h"

#include "resources/hairdb.h"
#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/monsterdb.h"
#include "resources/specialdb.h"
#include "resources/npcdb.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"
#include "resources/userpalette.h"

#include "utils/gettext.h"
#include "utils/mkdir.h"
#include "utils/stringutils.h"

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>
#endif

#include <physfs.h>
#include <SDL_image.h>

#ifdef _WIN32
#include <SDL_syswm.h>
#include "utils/specialfolder.h"
#else
#include <cerrno>
#endif

#include <sys/stat.h>
#include <cassert>

/**
 * Tells the max tick value,
 * setting it back to zero (and start again).
 */
static const int MAX_TICK_VALUE = 10000;

// TODO: Get rid fo these globals
std::string errorMessage;
LoginData loginData;

Configuration config;         /**< XML file configuration reader */
Configuration branding;       /**< XML branding information reader */
Configuration paths;          /**< XML default paths information reader */
Logger *logger;               /**< Log object */
ChatLogger *chatLogger;       /**< Chat log object */
KeyboardConfig keyboard;

UserPalette *userPalette;
Graphics *graphics;

ItemDB *itemDb;               /**< Items info database */
HairDB hairDB;                /**< Hair styles and colors info database */

Sound sound;

volatile int tick_time;       /**< Tick counter */
volatile int fps = 0;         /**< Frames counted in the last second */
volatile int frame_count = 0; /**< Counts the frames during one second */
volatile int cur_time;

/**
 * Advances game logic counter.
 * Called every 10 milliseconds by SDL_AddTimer()
 * @see MILLISECONDS_IN_A_TICK value
 */
Uint32 nextTick(Uint32 interval, void *param)
{
    tick_time++;
    if (tick_time == MAX_TICK_VALUE)
        tick_time = 0;
    return interval;
}

/**
 * Updates fps.
 * Called every seconds by SDL_AddTimer()
 */
Uint32 nextSecond(Uint32 interval, void *param)
{
    fps = frame_count;
    frame_count = 0;

    return interval;
}

int get_elapsed_time(int startTime)
{
    if (startTime <= tick_time)
        return (tick_time - startTime) * MILLISECONDS_IN_A_TICK;
    else
        return (tick_time + (MAX_TICK_VALUE - startTime))
                * MILLISECONDS_IN_A_TICK;
}

bool isDoubleClick(int selected)
{
    const Uint32 maximumDelay = 500;
    static Uint32 lastTime = 0;
    static int lastSelected = -1;

    if (selected == lastSelected && lastTime + maximumDelay >= SDL_GetTicks())
    {
        lastTime = 0;
        return true;
    }

    lastTime = SDL_GetTicks();
    lastSelected = selected;
    return false;
}


Client *Client::mInstance = 0;

Client::Client(const Options &options):
    mOptions(options),
    mGame(0),
    mCurrentDialog(0),
    mQuitDialog(0),
    mDesktop(0),
    mSetupButton(0),
    mState(STATE_CHOOSE_SERVER),
    mOldState(STATE_START),
    mStateAfterOkDialog(mState),
    mIcon(0),
    mLogicCounterId(0),
    mSecondsCounterId(0),
    mLimitFps(false)
{
    assert(!mInstance);
    mInstance = this;

    logger = new Logger;

    // Set default values for configuration files
    branding.setDefaultValues(getBrandingDefaults());
    paths.setDefaultValues(getPathsDefaults());
    config.setDefaultValues(getConfigDefaults());

    // Load branding information
    if (!options.brandingPath.empty())
    {
        branding.init(options.brandingPath);
    }

    initRootDir();
    initHomeDir();
    initConfiguration();

    chatLogger = new ChatLogger;
    if (options.chatLogDir.empty())
        chatLogger->setLogDir(mLocalDataDir + std::string("/logs/"));
    else
        chatLogger->setLogDir(options.chatLogDir);

    // Configure logger
    logger->setLogFile(mLocalDataDir + std::string("/mana.log"));
    logger->setLogToStandardOut(config.getBoolValue("logToStandardOut"));

    // Log the mana version
    logger->log("Mana %s", FULL_VERSION);

    initScreenshotDir();

    // Initialize SDL
    logger->log("Initializing SDL...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        logger->error(strprintf("Could not initialize SDL: %s",
                      SDL_GetError()));
    }
    atexit(SDL_Quit);

    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    SDL_WM_SetCaption(branding.getValue("appName", "Mana").c_str(), NULL);

    ResourceManager *resman = ResourceManager::getInstance();

    if (!resman->setWriteDir(mLocalDataDir))
    {
        logger->error(strprintf("%s couldn't be set as home directory! "
                                "Exiting.", mLocalDataDir.c_str()));
    }

    Image::SDLsetEnableAlphaCache(config.getValue("alphaCache", true));

#if defined __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path,
                                          PATH_MAX))
    {
        fprintf(stderr, "Can't find Resources directory\n");
    }
    CFRelease(resourcesURL);
    strncat(path, "/data", PATH_MAX - 1);
    resman->addToSearchPath(path, false);
    mPackageDir = path;
#else
    resman->addToSearchPath(PKG_DATADIR "data", false);
    mPackageDir = PKG_DATADIR "data";
#endif

    resman->addToSearchPath("data", false);

    // Add branding/data to PhysFS search path
    if (!options.brandingPath.empty())
    {
        std::string path = options.brandingPath;

        // Strip blah.mana from the path
#ifdef _WIN32
        int loc1 = path.find_last_of('/');
        int loc2 = path.find_last_of('\\');
        int loc = std::max(loc1, loc2);
#else
        int loc = path.find_last_of('/');
#endif
        if (loc > 0)
            resman->addToSearchPath(path.substr(0, loc + 1) + "data", false);
    }

    // Add the main data directories to our PhysicsFS search path
    if (!options.dataPath.empty())
        resman->addToSearchPath(options.dataPath, false);

    // Add the local data directory to PhysicsFS search path
    resman->addToSearchPath(mLocalDataDir, false);

    std::string iconFile = branding.getValue("appIcon", "icons/mana");
#ifdef _WIN32
    iconFile += ".ico";
#else
    iconFile += ".png";
#endif
    iconFile = resman->getPath(iconFile);
    logger->log("Loading icon from file: %s", iconFile.c_str());
#ifdef _WIN32
    static SDL_SysWMinfo pInfo;
    SDL_GetWMInfo(&pInfo);
    // Attempt to load icon from .ico file
    HICON icon = (HICON) LoadImage(NULL,
                                   iconFile.c_str(),
                                   IMAGE_ICON, 64, 64, LR_LOADFROMFILE);
    // If it's failing, we load the default resource file.
    if (!icon)
        icon = LoadIcon(GetModuleHandle(NULL), "A");

    if (icon)
        SetClassLong(pInfo.window, GCL_HICON, (LONG) icon);
#else
    mIcon = IMG_Load(iconFile.c_str());
    if (mIcon)
    {
        SDL_SetAlpha(mIcon, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
        SDL_WM_SetIcon(mIcon, NULL);
    }
#endif

    bool useOpenGL = !mOptions.noOpenGL && (config.getValue("opengl", 1) == 1);

    // Set up the transparency option for low CPU when not using OpenGL.
    if (!useOpenGL && (config.getValue("disableTransparency", 0) == 1))
        Image::SDLdisableTransparency();

#ifdef USE_OPENGL

    // Setup image loading for the right image format
    Image::setLoadAsOpenGL(useOpenGL);

    // Create the graphics context
    graphics = useOpenGL ? new OpenGLGraphics : new Graphics;
#else
    // Create the graphics context
    graphics = new Graphics;
#endif

    const int width = config.getIntValue("screenwidth");
    const int height = config.getIntValue("screenheight");
    const int bpp = 0;
    const bool fullscreen = config.getBoolValue("screen");
    const bool hwaccel = config.getBoolValue("hwaccel");

    // Try to set the desired video mode
    if (!graphics->setVideoMode(width, height, bpp, fullscreen, hwaccel))
    {
        logger->error(strprintf("Couldn't set %dx%dx%d video mode: %s",
            width, height, bpp, SDL_GetError()));
    }

    // Initialize for drawing
    graphics->_beginDraw();

    Theme::prepareThemePath();

    // Initialize the item and emote shortcuts.
    itemShortcut = new ItemShortcut;
    emoteShortcut = new EmoteShortcut;

    gui = new Gui(graphics);

    // Initialize sound engine
    try
    {
        if (config.getBoolValue("sound"))
            sound.init();

        sound.setSfxVolume(config.getIntValue("sfxVolume"));
        sound.setNotificationsVolume(config.getIntValue("notificationsVolume"));
        sound.setMusicVolume(config.getIntValue("musicVolume"));
    }
    catch (const char *err)
    {
        mState = STATE_ERROR;
        errorMessage = err;
        logger->log("Warning: %s", err);
    }

    // Initialize keyboard
    keyboard.init();

    // Initialise player relations
    player_relations.init();

    userPalette = new UserPalette;
    setupWindow = new Setup;

    sound.playMusic(branding.getStringValue("loginMusic"));

    // Initialize default server
    mCurrentServer.hostname = options.serverName;
    mCurrentServer.port = options.serverPort;
    loginData.username = options.username;
    loginData.password = options.password;
    loginData.remember = config.getBoolValue("remember");
    loginData.registerLogin = false;

    if (mCurrentServer.hostname.empty())
        mCurrentServer.hostname = branding.getValue("defaultServer","").c_str();

    if (mCurrentServer.port == 0)
    {
        mCurrentServer.port = (short) branding.getValue("defaultPort",
                                                                  DEFAULT_PORT);
        mCurrentServer.type = ServerInfo::parseType(
                           branding.getValue("defaultServerType", "tmwathena"));
    }

    if (chatLogger)
        chatLogger->setServerName(mCurrentServer.hostname);

    if (loginData.username.empty() && loginData.remember)
        loginData.username = config.getStringValue("username");

    if (mState != STATE_ERROR)
        mState = STATE_CHOOSE_SERVER;

    // Initialize logic and seconds counters
    tick_time = 0;
    mLogicCounterId = SDL_AddTimer(MILLISECONDS_IN_A_TICK, nextTick, NULL);
    mSecondsCounterId = SDL_AddTimer(1000, nextSecond, NULL);

    // Initialize frame limiting
    SDL_initFramerate(&mFpsManager);

    listen(Event::ConfigChannel);

    //TODO: fix having to fake a option changed event
    Event fakeevent(Event::ConfigOptionChanged);
    fakeevent.setString("option", "fpslimit");
    event(Event::ConfigChannel, fakeevent);

    // Initialize PlayerInfo
    PlayerInfo::init();
}

Client::~Client()
{
    SDL_RemoveTimer(mLogicCounterId);
    SDL_RemoveTimer(mSecondsCounterId);

    // Unload XML databases
    hairDB.unload();
    EmoteDB::unload();
    delete itemDb;
    MonsterDB::unload();
    NPCDB::unload();
    StatusEffect::unload();

    // Before config.write() since it writes the shortcuts to the config
    delete itemShortcut;
    delete emoteShortcut;

    delete gui;
    delete graphics;

    // Shutdown libxml
    xmlCleanupParser();

    // Shutdown sound
    sound.close();

    ResourceManager::deleteInstance();

    SDL_FreeSurface(mIcon);

    logger->log("Quitting");
    delete userPalette;

    config.write();

    delete logger;

    mInstance = 0;
}

int Client::exec()
{
    int lastTickTime = tick_time;

    SDL_Event event;

    while (mState != STATE_EXIT)
    {
        if (mGame)
        {
            // Let the game handle the events while it is active
            mGame->handleInput();
        }
        else
        {
            // Handle SDL events
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                        mState = STATE_EXIT;
                        break;

                    case SDL_KEYDOWN:
                        break;

                    case SDL_VIDEORESIZE:
                        handleVideoResize(event.resize.w, event.resize.h);
                        break;
                }

                guiInput->pushInput(event);
            }
        }

        if (Net::getGeneralHandler())
            Net::getGeneralHandler()->flushNetwork();

        while (get_elapsed_time(lastTickTime) > 0)
        {
            gui->logic();
            if (mGame)
                mGame->logic();

            sound.logic();

            ++lastTickTime;
        }

        // This is done because at some point tick_time will wrap.
        lastTickTime = tick_time;

        // Update the screen when application is active, delay otherwise.
        if (isActive())
        {
            frame_count++;
            gui->draw();
            graphics->updateScreen();
        }
        else
        {
            SDL_Delay(10);
        }

        if (mLimitFps)
            SDL_framerateDelay(&mFpsManager);

        // TODO: Add connect timeouts
        if (mState == STATE_CONNECT_GAME &&
                 Net::getGameHandler()->isConnected())
        {
            Net::getLoginHandler()->disconnect();
        }
        else if (mState == STATE_CONNECT_SERVER &&
                 mOldState == STATE_CONNECT_SERVER &&
                 Net::getLoginHandler()->isConnected())
        {
            mState = STATE_LOGIN;
        }
        else if (mState == STATE_WORLD_SELECT && mOldState == STATE_UPDATE)
        {
            if (Net::getLoginHandler()->getWorlds().size() < 2)
            {
                mState = STATE_LOGIN;
            }
        }
        else if (mOldState == STATE_START ||
                 (mOldState == STATE_GAME && mState != STATE_GAME))
        {
            gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());

            mDesktop = new Desktop;
            top->add(mDesktop);
            mSetupButton = new Button("", "Setup", this);
            mSetupButton->setButtonPopupText(_("Setup"));
            mSetupButton->setButtonIcon("button-icon-setup.png");
            mSetupButton->setPosition(top->getWidth() - mSetupButton->getWidth()
                                     - 3, 3);
            top->add(mSetupButton);

            mDesktop->setSize(graphics->getWidth(), graphics->getHeight());
        }

        if (mState == STATE_SWITCH_LOGIN && mOldState == STATE_GAME)
        {
            Net::getGameHandler()->disconnect();
        }

        if (mState != mOldState)
        {
            {
                Event event(Event::StateChange);
                event.setInt("oldState", mOldState);
                event.setInt("newState", mState);
                event.trigger(Event::ClientChannel);
            }

            if (mOldState == STATE_GAME)
            {
                delete mGame;
                mGame = 0;
            }

            mOldState = mState;

            // Get rid of the dialog of the previous state
            if (mCurrentDialog)
            {
                delete mCurrentDialog;
                mCurrentDialog = NULL;
            }
            // State has changed, while the quitDialog was active, it might
            // not be correct anymore
            if (mQuitDialog)
                mQuitDialog->scheduleDelete();

            switch (mState)
            {
                case STATE_CHOOSE_SERVER:
                    logger->log("State: CHOOSE SERVER");

                    // If a server was passed on the command line, or branding
                    // provides a server and a blank server list, we skip the
                    // server selection dialog.
                    if ((!mOptions.serverName.empty() && mOptions.serverPort)
                        || (!branding.getValue("defaultServer","").empty() &&
                            branding.getValue("defaultPort",0) &&
                            branding.getValue("onlineServerList", "").empty()))
                    {
                        mState = STATE_CONNECT_SERVER;

                        // Reset options so that cancelling or connect
                        // timeout will show the server dialog.
                        mOptions.serverName.clear();
                        mOptions.serverPort = 0;
                    }
                    else
                    {
                        // Don't allow an alpha opacity
                        // lower than the default value
                        Theme::instance()->setMinimumOpacity(0.8f);

                        mCurrentDialog = new ServerDialog(&mCurrentServer,
                                                          mConfigDir);
                    }
                    break;

                case STATE_CONNECT_SERVER:
                    logger->log("State: CONNECT SERVER");

                    Net::connectToServer(mCurrentServer);

                    mCurrentDialog = new ConnectionDialog(
                            _("Connecting to server"), STATE_SWITCH_SERVER);
                    break;

                case STATE_LOGIN:
                    logger->log("State: LOGIN");
                    // Don't allow an alpha opacity
                    // lower than the default value
                    Theme::instance()->setMinimumOpacity(0.8f);

                    if (mOptions.username.empty()
                            || mOptions.password.empty())
                    {
                        mCurrentDialog = new LoginDialog(&loginData);
                    }
                    else
                    {
                        mState = STATE_LOGIN_ATTEMPT;
                        // Clear the password so that when login fails, the
                        // dialog will show up next time.
                        mOptions.password.clear();
                    }
                    break;

                case STATE_LOGIN_ATTEMPT:
                    logger->log("State: LOGIN ATTEMPT");
                    accountLogin(&loginData);
                    mCurrentDialog = new ConnectionDialog(
                            _("Logging in"), STATE_SWITCH_SERVER);
                    break;

                case STATE_WORLD_SELECT:
                    logger->log("State: WORLD SELECT");
                    {
                        Worlds worlds = Net::getLoginHandler()->getWorlds();

                        if (worlds.size() == 0)
                        {
                            // Trust that the netcode knows what it's doing
                            mState = STATE_UPDATE;
                        }
                        else if (worlds.size() == 1)
                        {
                            Net::getLoginHandler()->chooseServer(0);
                            mState = STATE_UPDATE;
                        }
                        else
                        {
                            mCurrentDialog = new WorldSelectDialog(worlds);
                            if (mOptions.chooseDefault)
                            {
                                ((WorldSelectDialog*) mCurrentDialog)->action(
                                    gcn::ActionEvent(NULL, "ok"));
                            }
                        }
                    }
                    break;

                case STATE_WORLD_SELECT_ATTEMPT:
                    logger->log("State: WORLD SELECT ATTEMPT");
                    mCurrentDialog = new ConnectionDialog(
                            _("Entering game world"), STATE_WORLD_SELECT);
                    break;

                case STATE_UPDATE:
                    // Determine which source to use for the update host
                    if (!mOptions.updateHost.empty())
                        mUpdateHost = mOptions.updateHost;
                    else
                        mUpdateHost = loginData.updateHost;
                    initUpdatesDir();

                    if (mOptions.skipUpdate)
                    {
                        mState = STATE_LOAD_DATA;
                    }
                    else
                    {
                        logger->log("State: UPDATE");
                        mCurrentDialog = new UpdaterWindow(mUpdateHost,
                                mLocalDataDir + "/" + mUpdatesDir,mOptions.dataPath.empty());
                    }
                    break;

                case STATE_LOAD_DATA:
                    logger->log("State: LOAD DATA");

                    // If another data path has been set,
                    // we don't load any other files...
                    if (mOptions.dataPath.empty())
                    {
                        // Add customdata directory
                        ResourceManager::getInstance()->searchAndAddArchives(
                            "customdata/",
                            "zip",
                            false);
                    }

                    // Read default paths file 'data/paths.xml'
                    paths.init("paths.xml", true);

                    Event::trigger(Event::ClientChannel, Event::LoadingDatabases);

                    // Load XML databases
                    hairDB.load();
                    switch (Net::getNetworkType())
                    {
                      case ServerInfo::TMWATHENA:
                        itemDb = new TmwAthena::TaItemDB;
                      break;
                      case ServerInfo::MANASERV:
                        itemDb = new ManaServ::ManaServItemDB;
                      break;
                      default:
                        // Nothing
                        itemDb = 0;
                      break;
                    }
                    if (!itemDb || !itemDb->isLoaded())
                    {
                        // Warn and return to login screen
                        errorMessage =
                            _("This server is missing needed world data. "
                              "Please contact the administrator(s).");
                        showOkDialog(_("ItemDB: Error while loading "
                                       ITEMS_DB_FILE "!"), errorMessage,
                                     STATE_CHOOSE_SERVER);
                        break;
                    }
                    MonsterDB::load();
                    SpecialDB::load();
                    NPCDB::load();
                    EmoteDB::load();
                    StatusEffect::load();
                    Units::loadUnits();

                    ActorSprite::load();

                    mDesktop->reloadWallpaper();

                    mState = STATE_GET_CHARACTERS;
                    break;

                case STATE_GET_CHARACTERS:
                    logger->log("State: GET CHARACTERS");
                    Net::getCharHandler()->requestCharacters();
                    mCurrentDialog = new ConnectionDialog(
                            _("Requesting characters"),
                            STATE_SWITCH_SERVER);
                    break;

                case STATE_CHAR_SELECT:
                    logger->log("State: CHAR SELECT");
                    // Don't allow an alpha opacity
                    // lower than the default value
                    Theme::instance()->setMinimumOpacity(0.8f);

                    mCurrentDialog = new CharSelectDialog(&loginData);

                    if (!((CharSelectDialog*) mCurrentDialog)->selectByName(
                            mOptions.character, CharSelectDialog::Choose))
                    {
                        ((CharSelectDialog*) mCurrentDialog)->selectByName(
                                config.getStringValue("lastCharacter"),
                                mOptions.chooseDefault ?
                                    CharSelectDialog::Choose :
                                    CharSelectDialog::Focus);
                    }

                    // Choosing character on the command line should work only
                    // once, clear it so that 'switch character' works.
                    mOptions.character.clear();

                    break;

                case STATE_CONNECT_GAME:
                    logger->log("State: CONNECT GAME");

                    Net::getGameHandler()->connect();
                    mCurrentDialog = new ConnectionDialog(
                            _("Connecting to the game server"),
                            Net::getNetworkType() == ServerInfo::TMWATHENA ?
                            STATE_CHOOSE_SERVER : STATE_SWITCH_CHARACTER);
                    break;

                case STATE_CHANGE_MAP:
                    logger->log("State: CHANGE_MAP");

                    Net::getGameHandler()->connect();
                    mCurrentDialog = new ConnectionDialog(
                            _("Changing game servers"),
                            STATE_SWITCH_CHARACTER);
                    break;

                case STATE_GAME:
                    logger->log("Memorizing selected character %s",
                            local_player->getName().c_str());
                    config.setValue("lastCharacter", local_player->getName());

                    // Fade out logon-music here too to give the desired effect
                    // of "flowing" into the game.
                    sound.fadeOutMusic(1000);

                    // Allow any alpha opacity
                    Theme::instance()->setMinimumOpacity(-1.0f);

                    delete mSetupButton;
                    delete mDesktop;
                    mSetupButton = NULL;
                    mDesktop = NULL;

                    mCurrentDialog = NULL;

                    logger->log("State: GAME");
                    mGame = new Game;
                    break;

                case STATE_LOGIN_ERROR:
                    logger->log("State: LOGIN ERROR");
                    showErrorDialog(errorMessage, STATE_LOGIN);
                    break;

                case STATE_ACCOUNTCHANGE_ERROR:
                    logger->log("State: ACCOUNT CHANGE ERROR");
                    showErrorDialog(errorMessage, STATE_CHAR_SELECT);
                    break;

                case STATE_REGISTER_PREP:
                    logger->log("State: REGISTER_PREP");
                    Net::getLoginHandler()->getRegistrationDetails();
                    mCurrentDialog = new ConnectionDialog(
                            _("Requesting registration details"), STATE_LOGIN);
                    break;

                case STATE_REGISTER:
                    logger->log("State: REGISTER");
                    mCurrentDialog = new RegisterDialog(&loginData);
                    break;

                case STATE_REGISTER_ATTEMPT:
                    logger->log("Username is %s", loginData.username.c_str());
                    Net::getLoginHandler()->registerAccount(&loginData);
                    loginData.password = "";
                    break;

                case STATE_CHANGEPASSWORD:
                    logger->log("State: CHANGE PASSWORD");
                    mCurrentDialog = new ChangePasswordDialog(&loginData);
                    break;

                case STATE_CHANGEPASSWORD_ATTEMPT:
                    logger->log("State: CHANGE PASSWORD ATTEMPT");
                    Net::getLoginHandler()->changePassword(loginData.username,
                                                loginData.password,
                                                loginData.newPassword);
                    break;

                case STATE_CHANGEPASSWORD_SUCCESS:
                    logger->log("State: CHANGE PASSWORD SUCCESS");
                    showOkDialog(_("Password Change"),
                                 _("Password changed successfully!"),
                                 STATE_CHAR_SELECT);
                    loginData.password = "";
                    loginData.newPassword = "";
                    break;

                case STATE_CHANGEEMAIL:
                    logger->log("State: CHANGE EMAIL");
                    mCurrentDialog = new ChangeEmailDialog(&loginData);
                    break;

                case STATE_CHANGEEMAIL_ATTEMPT:
                    logger->log("State: CHANGE EMAIL ATTEMPT");
                    Net::getLoginHandler()->changeEmail(loginData.email);
                    break;

                case STATE_CHANGEEMAIL_SUCCESS:
                    logger->log("State: CHANGE EMAIL SUCCESS");
                    showOkDialog(_("Email Change"),
                                 _("Email changed successfully!"),
                                 STATE_CHAR_SELECT);
                    break;

                case STATE_UNREGISTER:
                    logger->log("State: UNREGISTER");
                    mCurrentDialog = new UnRegisterDialog(&loginData);
                    break;

                case STATE_UNREGISTER_ATTEMPT:
                    logger->log("State: UNREGISTER ATTEMPT");
                    Net::getLoginHandler()->unregisterAccount(
                            loginData.username, loginData.password);
                    break;

                case STATE_UNREGISTER_SUCCESS:
                    logger->log("State: UNREGISTER SUCCESS");
                    Net::getLoginHandler()->disconnect();

                    showOkDialog(_("Unregister Successful"),
                                 _("Farewell, come back any time..."),
                                 STATE_CHOOSE_SERVER);
                    loginData.clear();
                    break;

                case STATE_SWITCH_SERVER:
                    logger->log("State: SWITCH SERVER");

                    Net::getLoginHandler()->disconnect();
                    Net::getGameHandler()->disconnect();

                    mState = STATE_CHOOSE_SERVER;
                    break;

                case STATE_SWITCH_LOGIN:
                    logger->log("State: SWITCH LOGIN");

                    Net::getLoginHandler()->disconnect();

                    mState = STATE_CONNECT_SERVER;
                    break;

                case STATE_SWITCH_CHARACTER:
                    logger->log("State: SWITCH CHARACTER");

                    // Done with game
                    Net::getGameHandler()->disconnect();

                    mState = STATE_GET_CHARACTERS;
                    break;

                case STATE_LOGOUT_ATTEMPT:
                    logger->log("State: LOGOUT ATTEMPT");
                    // TODO
                    break;

                case STATE_WAIT:
                    logger->log("State: WAIT");
                    break;

                case STATE_EXIT:
                    logger->log("State: EXIT");
                    Net::unload();
                    break;

                case STATE_FORCE_QUIT:
                    logger->log("State: FORCE QUIT");
                    if (Net::getGeneralHandler())
                        Net::getGeneralHandler()->unload();
                    mState = STATE_EXIT;
                  break;

                case STATE_ERROR:
                    logger->log("State: ERROR");
                    logger->log("Error: %s", errorMessage.c_str());
                    showErrorDialog(errorMessage, STATE_CHOOSE_SERVER);
                    Net::getGameHandler()->disconnect();
                    break;

                default:
                    mState = STATE_FORCE_QUIT;
                    break;
            }
        }
    }

    return 0;
}

void Client::showOkDialog(const std::string &title,
                          const std::string &message,
                          State state)
{
    OkDialog *okDialog = new OkDialog(title, message);
    okDialog->addActionListener(this);
    mStateAfterOkDialog = state;
}

void Client::showErrorDialog(const std::string &message, State state)
{
    showOkDialog(_("Error"), message, state);
}

void Client::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::ConfigChannel &&
        event.getType() == Event::ConfigOptionChanged &&
        event.getString("option") == "fpslimit")
    {
        const int fpsLimit = config.getIntValue("fpslimit");
        mLimitFps = fpsLimit > 0;
        if (mLimitFps)
            SDL_setFramerate(&mFpsManager, fpsLimit);
    }

}

void Client::action(const gcn::ActionEvent &event)
{
    Window *window = 0;

    if (event.getId() == "Setup")
        window = setupWindow;

    if (window)
    {
        window->setVisible(!window->isVisible());
        if (window->isVisible())
            window->requestMoveToTop();
    }

    // If this came from the OkDialog used by showOkDialog
    if (event.getId() == "ok")
        mState = mStateAfterOkDialog;
}

void Client::initRootDir()
{
    mRootDir = PHYSFS_getBaseDir();
#ifdef _WIN32
    std::string portableName = mRootDir + "portable.xml";
    struct stat statbuf;

    if (!stat(portableName.c_str(), &statbuf) && S_ISREG(statbuf.st_mode))
    {
        std::string dir;
        Configuration portable;
        portable.init(portableName);

        logger->log("Portable file: %s", portableName.c_str());

        if (mOptions.localDataDir.empty())
        {
            dir = portable.getValue("dataDir", "");
            if (!dir.empty())
            {
                mOptions.localDataDir = mRootDir + dir;
                logger->log("Portable data dir: %s",
                    mOptions.localDataDir.c_str());
            }
        }

        if (mOptions.configDir.empty())
        {
            dir = portable.getValue("configDir", "");
            if (!dir.empty())
            {
                mOptions.configDir = mRootDir + dir;
                logger->log("Portable config dir: %s",
                    mOptions.configDir.c_str());
            }
        }

        if (mOptions.screenshotDir.empty())
        {
            dir = portable.getValue("screenshotDir", "");
            if (!dir.empty())
            {
                mOptions.screenshotDir = mRootDir + dir;
                logger->log("Portable screenshot dir: %s",
                    mOptions.screenshotDir.c_str());
            }
        }
    }
#endif
}

/**
 * Initializes the home directory. On UNIX and FreeBSD, ~/.mana is used. On
 * Windows and other systems we use the current working directory.
 */
void Client::initHomeDir()
{
    mLocalDataDir = mOptions.localDataDir;

    if (mLocalDataDir.empty())
    {
#ifdef __APPLE__
        // Use Application Directory instead of .mana
        mLocalDataDir = std::string(PHYSFS_getUserDir()) +
            "/Library/Application Support/" +
            branding.getValue("appName", "Mana");
#elif defined __HAIKU__
        mLocalDataDir = std::string(PHYSFS_getUserDir()) +
           "/config/data/Mana";
#elif defined _WIN32
        mLocalDataDir = getSpecialFolderLocation(CSIDL_LOCAL_APPDATA);
        if (mLocalDataDir.empty())
            mLocalDataDir = std::string(PHYSFS_getUserDir());
        mLocalDataDir += "/Mana";
#else
        mLocalDataDir = std::string(PHYSFS_getUserDir()) +
            "/.local/share/mana";
#endif
    }

    if (mkdir_r(mLocalDataDir.c_str()))
    {
        logger->error(strprintf(_("%s doesn't exist and can't be created! "
                                  "Exiting."), mLocalDataDir.c_str()));
    }

    mConfigDir = mOptions.configDir;

    if (mConfigDir.empty())
    {
        const std::string app = branding.getValue("appShort", "manasource");
#ifdef __APPLE__
        mConfigDir = mLocalDataDir + "/" + app;
#elif defined __HAIKU__
        mConfigDir = std::string(PHYSFS_getUserDir()) +
           "/config/settings/Mana" +
           branding.getValue("appName", "manasource");
#elif defined _WIN32
        mConfigDir = getSpecialFolderLocation(CSIDL_APPDATA);
        if (mConfigDir.empty())
            mConfigDir = mLocalDataDir;
        else
            mConfigDir += "/mana/" + app;
#else
        mConfigDir = std::string(PHYSFS_getUserDir()) + "/.config/mana/" + app;
#endif
    }

    if (mkdir_r(mConfigDir.c_str()))
    {
        logger->error(strprintf(_("%s doesn't exist and can't be created! "
                                  "Exiting."), mConfigDir.c_str()));
    }

    struct stat statbuf;
    std::string newConfigFile = mConfigDir + "/config.xml";
    if (stat(newConfigFile.c_str(), &statbuf))
    {
        std::string oldConfigFile = std::string(PHYSFS_getUserDir()) +
            "/.tmw/config.xml";
        if (mRootDir.empty() && !stat(oldConfigFile.c_str(), &statbuf)
            && S_ISREG(statbuf.st_mode))
        {
            std::ifstream oldConfig;
            std::ofstream newConfig;
            logger->log("Copying old TMW settings.");

            oldConfig.open(oldConfigFile.c_str(), std::ios::binary);
            newConfig.open(newConfigFile.c_str(), std::ios::binary);

            if (!oldConfig.is_open() || !newConfig.is_open())
                logger->log("Unable to copy old settings.");
            else
            {
                newConfig << oldConfig.rdbuf();
                newConfig.close();
                oldConfig.close();
            }
        }
    }
}

/**
 * Initialize configuration.
 */
void Client::initConfiguration()
{
    // Fill configuration with defaults
    config.setValue("hwaccel", false);
    config.setValue("opengl", true);
    config.setValue("screen", false);
    config.setValue("sound", true);
    config.setValue("guialpha", 0.8f);
    config.setValue("remember", true);
    config.setValue("sfxVolume", 100);
    config.setValue("musicVolume", 60);
    config.setValue("fpslimit", 60);
    std::string defaultUpdateHost = branding.getValue("defaultUpdateHost", "");
    config.setValue("updatehost", defaultUpdateHost);
    config.setValue("customcursor", true);
    config.setValue("useScreenshotDirectorySuffix", true);
    config.setValue("ChatLogLength", 128);
    config.setValue("disableTransparency", false);

    // Checking if the configuration file exists... otherwise create it with
    // default options.
    FILE *configFile = 0;
    std::string configPath;

    configPath = mConfigDir + "/config.xml";

    configFile = fopen(configPath.c_str(), "r");

    // If we can't read it, it doesn't exist !
    if (!configFile)
    {
        // We reopen the file in write mode and we create it
        configFile = fopen(configPath.c_str(), "wt");
    }
    if (!configFile)
    {
       logger->log("Can't create %s. Using defaults.", configPath.c_str());
    }
    else
    {
        fclose(configFile);
        config.init(configPath);
    }
}

/**
 * Parse the update host and determine the updates directory
 * Then verify that the directory exists (creating if needed).
 */
void Client::initUpdatesDir()
{
    // If updatesHost is currently empty, fill it from config file
    if (mUpdateHost.empty())
        mUpdateHost = config.getStringValue("updatehost");

    // Exit on empty update host.
    if (mUpdateHost.empty())
        return;

    logger->log("Setting update host: %s", mUpdateHost.c_str());

    std::string updateHost = getHostNameFromURL(mUpdateHost);

    // Exit on a wrong update host.
    if (updateHost.length() < 2)
    {
        // Show the original updateHostname in the error message.
        errorMessage = strprintf(_("Invalid update host: %s"),
                                 mUpdateHost.c_str());
        mState = STATE_ERROR;
        return;
    }

    mUpdateHost = updateHost;
    mUpdatesDir = "updates/" + mUpdateHost;

    ResourceManager *resman = ResourceManager::getInstance();

    // Verify that the updates directory exists. Create if necessary.
    if (!resman->isDirectory("/" + mUpdatesDir))
    {
        if (!resman->mkdir("/" + mUpdatesDir))
        {
#if defined _WIN32
            std::string newDir = mLocalDataDir + "\\" + mUpdatesDir;
            std::string::size_type loc = newDir.find("/", 0);

            while (loc != std::string::npos)
            {
                newDir.replace(loc, 1, "\\");
                loc = newDir.find("/", loc);
            }

            if (!CreateDirectory(newDir.c_str(), 0) &&
                GetLastError() != ERROR_ALREADY_EXISTS)
            {
                logger->log("Error: %s can't be made, but doesn't exist!",
                            newDir.c_str());
                errorMessage =
                    strprintf(_("Error creating updates directory!\n(%s)"),
                                newDir.c_str());
                mState = STATE_ERROR;
            }
#else
            logger->log("Error: %s/%s can't be made, but doesn't exist!",
                        mLocalDataDir.c_str(), mUpdatesDir.c_str());
            errorMessage =
                strprintf(_("Error creating updates directory!\n(%s/%s)"),
                            mLocalDataDir.c_str(), mUpdatesDir.c_str());
            mState = STATE_ERROR;
#endif
        }
    }
}

void Client::initScreenshotDir()
{
    if (!mOptions.screenshotDir.empty())
    {
        mScreenshotDir = mOptions.screenshotDir;
    }
    else if (mScreenshotDir.empty())
    {
#ifdef _WIN32
        mScreenshotDir = getSpecialFolderLocation(CSIDL_MYPICTURES);
        if (mScreenshotDir.empty())
            mScreenshotDir = getSpecialFolderLocation(CSIDL_DESKTOP);
#else
        mScreenshotDir = std::string(PHYSFS_getUserDir()) + "Desktop";
#endif

        if (config.getBoolValue("useScreenshotDirectorySuffix"))
        {
            std::string configScreenshotSuffix =
                config.getValue("screenshotDirectorySuffix",
                                branding.getValue("appShort", "Mana"));

            if (!configScreenshotSuffix.empty())
            {
                mScreenshotDir += "/" + configScreenshotSuffix;
                config.setValue("screenshotDirectorySuffix",
                                configScreenshotSuffix);
            }
        }
    }
}

void Client::accountLogin(LoginData *loginData)
{
    logger->log("Username is %s", loginData->username.c_str());

    // Send login infos
    if (loginData->registerLogin)
        Net::getLoginHandler()->registerAccount(loginData);
    else
        Net::getLoginHandler()->loginAccount(loginData);

    // Clear the password, avoids auto login when returning to login
    loginData->password = "";

    // TODO This is not the best place to save the config, but at least better
    // than the login gui window
    if (loginData->remember)
        config.setValue("username", loginData->username);
    config.setValue("remember", loginData->remember);
}

void Client::handleVideoResize(int width, int height)
{
    // Keep a minimum size. This isn't adhered to by the actual window, but
    // it keeps some window positions from getting messed up.
    width = std::max(640, width);
    height = std::max(360, height);

    if (graphics->changeVideoMode(width,
                                  height,
                                  graphics->getBpp(),
                                  false,
                                  graphics->getHWAccel()))
    {
        videoResized(graphics->getWidth(),
                     graphics->getHeight());

        // Since everything appears to have worked out, remember to store the
        // new size in the configuration.
        config.setValue("screenwidth", width);
        config.setValue("screenheight", height);
    }
}

void Client::videoResized(int width, int height)
{
    gui->videoResized();

    if (mDesktop)
        mDesktop->setSize(width, height);

    if (mSetupButton)
        mSetupButton->setPosition(width - mSetupButton->getWidth() - 3, 3);

    if (mGame)
        mGame->videoResized(width, height);
}
