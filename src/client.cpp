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
#include "playerrelations.h"
#include "sound.h"

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

#include "resources/chardb.h"
#include "resources/hairdb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"
#include "resources/userpalette.h"
#include "resources/settingsmanager.h"

#include "utils/filesystem.h"
#include "utils/gettext.h"
#include "utils/mkdir.h"
#if defined(_WIN32) || defined(__APPLE__)
#include "utils/specialfolder.h"
#endif
#include "utils/stringutils.h"
#include "utils/time.h"

#include <SDL_image.h>

#ifdef _WIN32
#include <SDL_syswm.h>
#include <winuser.h>
#endif

#include <sys/stat.h>
#include <cassert>

// TODO: Get rid of these globals
std::string errorMessage;
LoginData loginData;

Config config;                /**< Global settings (config.xml) */
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

volatile int fps = 0;         /**< Frames counted in the last second */
volatile int frame_count = 0; /**< Counts the frames during one second */

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

bool isDoubleClick(int selected)
{
    static Timer timer;
    static int lastSelected = -1;

    if (selected == lastSelected && !timer.passed())
        return true;

    timer.set(500);
    lastSelected = selected;
    return false;
}

/**
 * A simple but effective frame rate limiter.
 * Based on FPSmanager in SDL2_gfx.
 */
void FpsManager::limitFps(int fpsLimit)
{
    if (fpsLimit <= 0)
        return;

    const auto ticks = SDL_GetTicks();

    // Need to reset if the FPS limit changes
    if (mBaseTicks > 0 && mFpsLimit == fpsLimit)
    {
        ++mFrameCount;
        const auto targetTicks = mBaseTicks + mFrameCount * 1000 / mFpsLimit;

        // Make sure ticks hasn't wrapped
        if (ticks > mBaseTicks && targetTicks > ticks)
        {
            SDL_Delay(targetTicks - ticks);
            return;
        }
    }

    mFpsLimit = fpsLimit;
    mFrameCount = 0;
    mBaseTicks = ticks;
}


Client *Client::mInstance = nullptr;

Client::Client(const Options &options):
    mOptions(options),
    mStateAfterOkDialog(mState)
{
    assert(!mInstance);
    mInstance = this;

    logger = new Logger;

    // Set default values for configuration files
    branding.setDefaultValues(getBrandingDefaults());
    paths.setDefaultValues(getPathsDefaults());

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
        chatLogger->setLogDir(mLocalDataDir + "/logs/");
    else
        chatLogger->setLogDir(options.chatLogDir);

    // Configure logger
    logger->setLogFile(mLocalDataDir + "/mana.log");
    logger->setLogToStandardOut(config.logToStandardOut);

    // Log the mana version
    logger->log("%s", FULL_VERSION);

    initScreenshotDir();

#if SDL_VERSION_ATLEAST(2, 24, 0)
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#endif

    // Initialize SDL
    logger->log("Initializing SDL...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        logger->error(strprintf("Could not initialize SDL: %s",
                      SDL_GetError()));
    }
    atexit(SDL_Quit);

    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    ResourceManager *resman = ResourceManager::getInstance();

    if (!FS::setWriteDir(mLocalDataDir))
    {
        logger->error(strprintf("%s couldn't be set as write directory! "
                                "Exiting.", mLocalDataDir.c_str()));
    }

#if defined __APPLE__
    mPackageDir = getResourcesLocation() + "/data";
#else
    mPackageDir = PKG_DATADIR "data";
#endif
    resman->addToSearchPath(mPackageDir, false);

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

    bool useOpenGL = !mOptions.noOpenGL && config.opengl;

    // Set up the transparency option for low CPU when not using OpenGL.
    if (!useOpenGL && config.disableTransparency)
        Image::SDLdisableTransparency();

    VideoSettings videoSettings;
    videoSettings.windowMode = config.windowMode;
    videoSettings.width = config.screenWidth;
    videoSettings.height = config.screenHeight;
    videoSettings.userScale = config.scale;
    videoSettings.vsync = config.vsync;
    videoSettings.openGL = useOpenGL;

    // Try to set the desired video mode and create the graphics context
    graphics = mVideo.initialize(videoSettings);

    SDL_SetWindowTitle(mVideo.window(),
                       branding.getValue("appName", "Mana").c_str());

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
    SDL_GetWindowWMInfo(mVideo.window(), &pInfo);
    // Attempt to load icon from .ico file
    HICON icon = (HICON) LoadImage(NULL,
                                   iconFile.c_str(),
                                   IMAGE_ICON, 64, 64, LR_LOADFROMFILE);
    // If it's failing, we load the default resource file.
    if (!icon)
        icon = LoadIcon(GetModuleHandle(NULL), "A");

    if (icon)
        SetClassLongPtr(pInfo.info.win.window, GCLP_HICON, (LONG_PTR) icon);
#else
    mIcon = IMG_Load(iconFile.c_str());
    if (mIcon)
    {
        SDL_SetWindowIcon(mVideo.window(), mIcon);
    }
#endif

    // Initialize the item and emote shortcuts.
    itemShortcut = new ItemShortcut;
    emoteShortcut = new EmoteShortcut;

    gui = new Gui(graphics, Theme::prepareThemePath());

    // Initialize sound engine
    try
    {
        if (config.sound)
            sound.init();

        sound.setSfxVolume(config.sfxVolume);
        sound.setNotificationsVolume(config.notificationsVolume);
        sound.setMusicVolume(config.musicVolume);
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
    mCurrentServer.type = options.serverType;
    loginData.username = options.username;
    loginData.password = options.password;
    loginData.remember = config.remember;
    loginData.registerLogin = false;

    if (mCurrentServer.type == ServerType::UNKNOWN && mCurrentServer.port != 0)
    {
        mCurrentServer.type = ServerInfo::defaultServerTypeForPort(mCurrentServer.port);
    }

    if (mCurrentServer.type == ServerType::UNKNOWN)
    {
        mCurrentServer.type = ServerInfo::parseType(
                    branding.getValue("defaultServerType", "tmwathena"));
    }

    if (mCurrentServer.port == 0)
    {
        const uint16_t defaultPort = ServerInfo::defaultPortForServerType(mCurrentServer.type);
        mCurrentServer.port = static_cast<uint16_t>(
                    branding.getValue("defaultPort", defaultPort));
    }

    const bool noServerList = branding.getValue("onlineServerList", std::string()).empty();
    if (mCurrentServer.hostname.empty() && noServerList)
    {
        mCurrentServer.hostname = branding.getValue("defaultServer", std::string());
    }

    if (chatLogger)
        chatLogger->setServerName(mCurrentServer.hostname);

    if (loginData.username.empty() && loginData.remember)
        loginData.username = config.username;

    if (mState != STATE_ERROR)
    {
        // If a server was passed on the command line, or branding
        // provides a server and a blank server list, we skip the
        // server selection dialog.
        mState = mCurrentServer.isValid() ? STATE_CONNECT_SERVER
                                          : STATE_CHOOSE_SERVER;
    }

    // Initialize seconds counter
    mSecondsCounterId = SDL_AddTimer(1000, nextSecond, nullptr);

    // Initialize PlayerInfo
    PlayerInfo::init();
}

Client::~Client()
{
    SDL_RemoveTimer(mSecondsCounterId);

    // Unload XML databases
    SettingsManager::unload();
    CharDB::unload();
    delete itemDb;

    ActorSprite::unload();

    // Before config.write() since it writes the shortcuts to the config
    delete itemShortcut;
    delete emoteShortcut;

    delete gui;

    // Shutdown libxml
    xmlCleanupParser();

    // Shutdown sound
    sound.close();

    ResourceManager::deleteInstance();

    SDL_FreeSurface(mIcon);

    logger->log("Quitting");
    delete userPalette;

    XML::Writer writer(mConfigDir + "/config.xml");
    if (writer.isValid())
        serialize(writer, config);

    delete logger;

    mInstance = nullptr;
}

int Client::exec()
{
    Time::beginFrame();     // Prevent startup lag influencing the first frame

    while (mState != STATE_EXIT)
    {
        Time::beginFrame();

        if (mGame)
        {
            // Let the game handle the events while it is active
            mGame->handleInput();
        }
        else
        {
            // Handle SDL events
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    mState = STATE_EXIT;
                    break;

                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        handleWindowSizeChanged(event.window.data1,
                                                event.window.data2);
                        break;
                    }
                    break;
                }

                guiInput->pushInput(event);
            }
        }

        if (Net::getGeneralHandler())
            Net::getGeneralHandler()->flushNetwork();

        gui->logic();
        if (mGame)
            mGame->logic();

        sound.logic();

        // Update the screen when application is active, delay otherwise.
        if (isActive())
        {
            frame_count++;
            gui->draw();
            graphics->updateScreen();
            mFpsManager.limitFps(config.fpsLimit);
        }
        else
        {
            mFpsManager.limitFps(10);
        }

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
            auto *top = static_cast<gcn::Container*>(gui->getTop());

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
                mGame = nullptr;
            }

            mOldState = mState;

            // Get rid of the dialog of the previous state
            if (mCurrentDialog)
            {
                delete mCurrentDialog;
                mCurrentDialog = nullptr;
            }
            // State has changed, while the quitDialog was active, it might
            // not be correct anymore
            if (mQuitDialog)
                mQuitDialog->scheduleDelete();

            switch (mState)
            {
                case STATE_CHOOSE_SERVER:
                    logger->log("State: CHOOSE SERVER");

                    // Don't allow an alpha opacity
                    // lower than the default value
                    gui->getTheme()->setMinimumOpacity(0.8f);

                    mCurrentDialog = new ServerDialog(&mCurrentServer,
                                                      mConfigDir);
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
                    gui->getTheme()->setMinimumOpacity(0.8f);

                    if (mOptions.username.empty() || mOptions.password.empty())
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

                        if (worlds.empty())
                        {
                            // Trust that the netcode knows what it's doing
                            mState = STATE_UPDATE;
                        }
                        else if (worlds.size() == 1 || mOptions.chooseDefault)
                        {
                            Net::getLoginHandler()->chooseServer(0);
                            mState = STATE_UPDATE;
                        }
                        else
                        {
                            mCurrentDialog = new WorldSelectDialog(std::move(worlds));
                        }
                    }
                    break;

                case STATE_WORLD_SELECT_ATTEMPT:
                    logger->log("State: WORLD SELECT ATTEMPT");
                    mCurrentDialog = new ConnectionDialog(
                            _("Entering game world"), STATE_WORLD_SELECT);
                    break;

                case STATE_UPDATE:
                    logger->log("State: UPDATE");

                    if (mOptions.skipUpdate)
                    {
                        mState = STATE_LOAD_DATA;
                    }
                    else if (initUpdatesDir())
                    {
                        mCurrentDialog = new UpdaterWindow(mUpdateHost,
                                                           mLocalDataDir + "/" + mUpdatesDir,
                                                           mOptions.dataPath.empty());
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

                    // TODO remove this as soon as inventoryhandler stops using this event
                    Event::trigger(Event::ClientChannel, Event::LoadingDatabases);

                    // Load XML databases
                    CharDB::load();

                    delete itemDb;

                    switch (Net::getNetworkType())
                    {
                      case ServerType::TMWATHENA:
                        itemDb = new TmwAthena::TaItemDB;
                      break;
                      case ServerType::MANASERV:
                        itemDb = new ManaServ::ManaServItemDB;
                      break;
                      default:
                        // Nothing
                        itemDb = nullptr;
                      break;
                    }
                    assert(itemDb);

                    // load settings.xml
                    SettingsManager::load();

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
                    gui->getTheme()->setMinimumOpacity(0.8f);

                    mCurrentDialog = new CharSelectDialog(&loginData);

                    if (!((CharSelectDialog*) mCurrentDialog)->selectByName(
                            mOptions.character, CharSelectDialog::Choose))
                    {
                        ((CharSelectDialog*) mCurrentDialog)->selectByName(
                                config.lastCharacter,
                                mOptions.chooseDefault ?
                                    CharSelectDialog::Choose :
                                    CharSelectDialog::Focus);
                    }

                    // Choosing character on the command line should work only
                    // once, clear it so that 'switch character' works.
                    mOptions.character.clear();
                    mOptions.chooseDefault = false;

                    break;

                case STATE_CONNECT_GAME:
                    logger->log("State: CONNECT GAME");

                    Net::getGameHandler()->connect();
                    mCurrentDialog = new ConnectionDialog(
                            _("Connecting to the game server"),
                            Net::getNetworkType() == ServerType::TMWATHENA ?
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
                    config.lastCharacter = local_player->getName();

                    // Fade out logon-music here too to give the desired effect
                    // of "flowing" into the game.
                    sound.fadeOutMusic(1000);

                    // Allow any alpha opacity
                    gui->getTheme()->setMinimumOpacity(0.0f);

                    delete mSetupButton;
                    delete mDesktop;
                    mSetupButton = nullptr;
                    mDesktop = nullptr;

                    mCurrentDialog = nullptr;

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
                    loginData.password.clear();
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
                    loginData.password.clear();
                    loginData.newPassword.clear();
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

                    mCurrentServer.hostname.clear();
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
                    break;

                case STATE_FORCE_QUIT:
                    logger->log("State: FORCE QUIT");
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

    Net::unload();

    return 0;
}

void Client::showOkDialog(const std::string &title,
                          const std::string &message,
                          State state)
{
    auto *okDialog = new OkDialog(title, message);
    okDialog->addActionListener(this);
    mStateAfterOkDialog = state;
}

void Client::showErrorDialog(const std::string &message, State state)
{
    showOkDialog(_("Error"), message, state);
}

void Client::action(const gcn::ActionEvent &event)
{
    Window *window = nullptr;

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
    mRootDir = FS::getBaseDir();
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
#if defined __HAIKU__
        mLocalDataDir = FS::getUserDir();
        mLocalDataDir += "/config/data/Mana";
#elif defined _WIN32
        mLocalDataDir = getSpecialFolderLocation(FOLDERID_LocalAppData);
        if (mLocalDataDir.empty())
            mLocalDataDir = FS::getUserDir();
        mLocalDataDir += "/Mana";
#else
        mLocalDataDir = FS::getPrefDir("manasource.org", "mana");
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
        mConfigDir = FS::getPrefDir("manasource.org", "Mana");
        mConfigDir += app;
#elif defined _WIN32
        mConfigDir = FS::getPrefDir("Mana", app.c_str());
#else
        mConfigDir = std::string(FS::getUserDir()) + ".config/mana/" + app;
#endif
    }

    if (mkdir_r(mConfigDir.c_str()))
    {
        logger->error(strprintf(_("%s doesn't exist and can't be created! "
                                  "Exiting."), mConfigDir.c_str()));
    }
}

/**
 * Initialize configuration.
 */
void Client::initConfiguration()
{
    // Fill configuration with defaults
    config.updatehost = branding.getValue("defaultUpdateHost", std::string());

    const std::string configPath = mConfigDir + "/config.xml";
    XML::Document doc(configPath, false);

    if (doc.rootNode() && doc.rootNode().name() == "configuration")
        deserialize(doc.rootNode(), config);
    else
        logger->log("Couldn't read configuration file: %s", configPath.c_str());
}

/**
 * Parse the update host and determine the updates directory
 * Then verify that the directory exists (creating if needed).
 */
bool Client::initUpdatesDir()
{
    // Determine which source to use for the update host
    if (!mOptions.updateHost.empty())
        mUpdateHost = mOptions.updateHost;
    else if (!loginData.updateHost.empty())
        mUpdateHost = loginData.updateHost;
    else
        mUpdateHost = config.updatehost;

    // Remove any trailing slashes at the end of the URL
    while (!mUpdateHost.empty() && mUpdateHost.back() == '/')
        mUpdateHost.pop_back();

    if (mUpdateHost.empty())
    {
        logger->log("No update host provided");
        mUpdatesDir.clear();
        mState = STATE_LOAD_DATA;
        return false;
    }

    mUpdatesDir = "updates/" + getDirectoryFromURL(mUpdateHost);

    logger->log("Update host: %s", mUpdateHost.c_str());
    logger->log("Updates dir: %s", mUpdatesDir.c_str());

    // Verify that the updates directory exists. Create if necessary.
    if (!FS::isDirectory(mUpdatesDir))
    {
        if (!FS::mkdir(mUpdatesDir))
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
            return false;
#endif
        }
    }

    return true;
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
        mScreenshotDir = getSpecialFolderLocation(FOLDERID_Pictures);
        if (mScreenshotDir.empty())
            mScreenshotDir = getSpecialFolderLocation(FOLDERID_Desktop);
#else
        mScreenshotDir = std::string(FS::getUserDir()) + "Desktop";
#endif

        if (config.useScreenshotDirectorySuffix)
        {
            std::string screenshotSuffix = config.screenshotDirectorySuffix;
            if (screenshotSuffix.empty())
                screenshotSuffix = branding.getValue("appShort", "Mana");

            if (!screenshotSuffix.empty())
            {
                mScreenshotDir += "/" + screenshotSuffix;
                config.screenshotDirectorySuffix = screenshotSuffix;
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
    loginData->password.clear();

    // TODO This is not the best place to save the config, but at least better
    // than the login gui window
    if (loginData->remember)
        config.username = loginData->username;
    config.remember = loginData->remember;
}

void Client::handleWindowSizeChanged(int width, int height)
{
    // Store the new size in the configuration.
    config.screenWidth = width;
    config.screenHeight = height;

    mVideo.windowSizeChanged(width, height);

    checkGraphicsSize();
}

void Client::checkGraphicsSize()
{
    const int width = graphics->getWidth();
    const int height = graphics->getHeight();

    if (!gui->videoResized(width, height))
        return;

    if (mDesktop)
        mDesktop->setSize(width, height);

    if (mSetupButton)
        mSetupButton->setPosition(width - mSetupButton->getWidth() - 3, 3);

    if (mGame)
        mGame->videoResized(width, height);
}

bool Client::isActive()
{
    return !(SDL_GetWindowFlags(getVideo().window()) & SDL_WINDOW_MINIMIZED);
}

bool Client::hasInputFocus()
{
    return SDL_GetWindowFlags(getVideo().window()) & SDL_WINDOW_INPUT_FOCUS;
}

bool Client::hasMouseFocus()
{
    return SDL_GetWindowFlags(getVideo().window()) & SDL_WINDOW_MOUSE_FOCUS;
}
