/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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
#include "log.h"
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
#include "net/offlinehandler.h"
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
#include "utils/path.h"
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
#include <cstdio>
#include <cstdlib>
#include <filesystem>

#include <guichan/exception.hpp>

// TODO: Get rid of these globals
std::string errorMessage;
LoginData loginData;

Config config;                /**< Global settings (config.xml) */
Branding branding;            /**< Branding settings (*.mana file) */
Paths paths;                  /**< Default paths */
ChatLogger *chatLogger;       /**< Chat log object */
KeyboardConfig keyboard;

UserPalette *userPalette;
Graphics *graphics;

ItemDB *itemDb;               /**< Items info database */
HairDB hairDB;                /**< Hair styles and colors info database */

Sound sound;

volatile int fps = 0;         /**< Frames counted in the last second */
volatile int frame_count = 0; /**< Counts the frames during one second */

#ifdef _WIN32
static bool isDirWritable(const std::string &dir)
{
    const auto path = utils::joinPaths(dir, ".__mana_write_test.tmp");

    FILE *file = std::fopen(path.c_str(), "wb");
    if (!file)
        return false;

    std::fclose(file);
    std::remove(path.c_str());
    return true;
}
#endif

/**
 * Updates fps.
 * Called every seconds by SDL_AddTimer()
 */
static Uint32 nextSecond(Uint32 interval, void *param)
{
    fps = frame_count;
    frame_count = 0;

    return interval;
}

/**
 * Returns whether an Alt key is held down.
 *
 * The modifier state of the event is not enough, since SDL can lose track of
 * the modifiers while the window changes to or from fullscreen (seen on X11).
 * The key state stays correct in that case.
 */
static bool isAltDown(uint16_t eventModifiers)
{
    if (eventModifiers & KMOD_ALT)
        return true;

    const uint8_t *keyState = SDL_GetKeyboardState(nullptr);
    return keyState[SDL_SCANCODE_LALT] || keyState[SDL_SCANCODE_RALT];
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

    // Load branding information
    if (!options.brandingPath.empty())
    {
        XML::Document doc(options.brandingPath, false);
        XML::Node rootNode = doc.rootNode();

        if (rootNode && rootNode.name() == "configuration")
        {
            auto brandingOptions = readOptions(rootNode);
            deserialize(brandingOptions, branding);
            warnUnknownOptions(options.brandingPath, brandingOptions);
        }
        else
        {
            Log::info("Couldn't read branding file: %s",
                      options.brandingPath.c_str());
        }
    }

    initRootDir();
    initHomeDir();
    initConfiguration();

    // Configure logger
    Log::init();
    Log::setLogToStandardOut(config.logToStandardOut);
    if (options.logFile == "-")
        Log::setLogToStandardOut(true);
    else if (!options.logFile.empty())
        Log::setLogFile(options.logFile);
    else
        Log::setLogFile(mLocalDataDir + "/mana.log");
    Log::info("%s", FULL_VERSION);

    chatLogger = new ChatLogger;
    if (options.chatLogDir.empty())
        chatLogger->setLogDir(mLocalDataDir + "/logs/");
    else
        chatLogger->setLogDir(options.chatLogDir);

    initScreenshotDir();

#if SDL_VERSION_ATLEAST(2, 24, 0)
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#endif

    // Initialize SDL
    Log::info("Initializing SDL...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        Log::critical(strprintf("Could not initialize SDL: %s",
                      SDL_GetError()));
    }
    atexit(SDL_Quit);

    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    if (!FS::setWriteDir(mLocalDataDir))
    {
        Log::critical(strprintf("%s couldn't be set as write directory! "
                                "Exiting.", mLocalDataDir.c_str()));
    }

#if defined __APPLE__
    mPackageDir = getResourcesLocation() + "/data";
#elif defined __linux__
    // When running from an AppImage the compiled-in absolute path needs to
    // be resolved against the mount point, which the runtime sets as APPDIR.
    if (const char *appDir = getenv("APPDIR"))
        mPackageDir = std::string(appDir) + PKG_DATADIR "data";
    else
        mPackageDir = PKG_DATADIR "data";
#else
    mPackageDir = PKG_DATADIR "data";
#endif
    ResourceManager::addToSearchPath(mPackageDir, false);
    ResourceManager::addToSearchPath("data", false);

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
            ResourceManager::addToSearchPath(path.substr(0, loc + 1) + "data", false);
    }

    // Add the main data directories to our PhysicsFS search path
    if (!options.dataPath.empty())
        ResourceManager::addToSearchPath(options.dataPath, false);

    // Add the local data directory to PhysicsFS search path
    ResourceManager::addToSearchPath(mLocalDataDir, false);

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

    SDL_SetWindowTitle(mVideo.window(), branding.name().c_str());

    std::string iconFile = branding.appIcon;
#ifdef _WIN32
    iconFile += ".ico";
#else
    iconFile += ".png";
#endif
    iconFile = ResourceManager::getPath(iconFile);
    Log::info("Loading icon from file: %s", iconFile.c_str());
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
        mState = State::Error;
        errorMessage = err;
        Log::warn("%s", err);
    }

    // Initialize keyboard
    keyboard.init();

    // Initialise player relations
    player_relations.init();

    userPalette = new UserPalette;
    setupWindow = new Setup;

    sound.playMusic(branding.loginMusic);

    // Initialize default server
    mCurrentServer.hostname = options.serverName;
    mCurrentServer.port = options.serverPort;
    mCurrentServer.type = options.serverType;
    loginData.username = options.username;
    loginData.password = options.password;

    if (mCurrentServer.type == ServerType::Unknown && mCurrentServer.port != 0)
        mCurrentServer.type = ServerInfo::defaultServerTypeForPort(mCurrentServer.port);

    if (mCurrentServer.type == ServerType::Unknown)
        mCurrentServer.type = ServerInfo::parseType(branding.defaultServerType);

    if (mCurrentServer.port == 0)
    {
        const uint16_t defaultPort = ServerInfo::defaultPortForServerType(mCurrentServer.type);
        mCurrentServer.port = branding.defaultPort.value_or(defaultPort);
    }

    const bool noServerList = branding.onlineServerList.empty();
    if (mCurrentServer.hostname.empty() && noServerList)
        mCurrentServer.hostname = branding.defaultServer;

    if (chatLogger)
        chatLogger->setServerName(mCurrentServer.hostname);

    if (loginData.username.empty() && config.remember)
        loginData.username = config.username;

    if (mState != State::Error)
    {
        // If a server was passed on the command line, or branding
        // provides a server and a blank server list, we skip the
        // server selection dialog.
        mState = mCurrentServer.isValid() ? State::ConnectServer
                                          : State::ChooseServer;

        if (mCurrentServer.type == ServerType::Offline)
        {
            Net::connectToServer(mCurrentServer);

            auto *offlineHandler = static_cast<OfflineHandler*>(Net::getGameHandler());

            // Always load the saved state, so that the inventory is kept even
            // when starting on a different map.
            const bool loadedState = offlineHandler->loadSavedState();

            if (!mOptions.loadMap.empty())
                offlineHandler->setMap(mOptions.loadMap);
            else if (!loadedState)
            {
                errorMessage = _("No saved offline state found. "
                                 "Use --load-map to specify a map.");
                mState = State::Error;
            }

            if (mState != State::Error)
                mState = State::LoadData;
        }
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

    Log::info("Quitting");
    delete userPalette;

    XML::Writer writer(mConfigDir + "/client.xml");
    if (writer.isValid())
        serialize(writer, config);

    mInstance = nullptr;
}

int Client::exec()
{
    Time::beginFrame();     // Prevent startup lag influencing the first frame

    while (mState != State::Exit)
    {
        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                mState = State::Exit;
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    handleWindowSizeChanged(event.window.data1,
                                            event.window.data2);
                    break;
                }
                break;

            case SDL_KEYDOWN:
                if ((event.key.keysym.sym == SDLK_RETURN ||
                     event.key.keysym.sym == SDLK_KP_ENTER) &&
                        isAltDown(event.key.keysym.mod) &&
                        !event.key.repeat)
                {
                    toggleFullscreen();
                    continue;
                }

                if (keyboard.isEnabled())
                {
                    const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);
                    if (tKey == KeyboardConfig::KEY_WINDOW_SETUP)
                    {
                        setupWindow->setVisible(!setupWindow->isVisible());
                        if (setupWindow->isVisible())
                            setupWindow->requestMoveToTop();
                        continue;
                    }
                }

                if (setupWindow->isVisible() &&
                    keyboard.getNewKeyIndex() > KeyboardConfig::KEY_NO_VALUE)
                {
                    keyboard.setNewKey(event.key.keysym.sym);
                    keyboard.callbackNewKey();
                    keyboard.setNewKeyIndex(KeyboardConfig::KEY_NO_VALUE);
                    continue;
                }

                // Check whether the game will handle the event
                if (mGame && mGame->keyDownEvent(event.key))
                    continue;

                break;
            }

            // Push input to GUI when not used
            try
            {
                guiInput->pushInput(event);
            }
            catch (gcn::Exception e)
            {
                const char *err = e.getMessage().c_str();
                Log::warn("Guichan input exception: %s", err);
            }
        }

        update();
    }

    Net::unload();

    return 0;
}

void Client::update()
{
    Time::beginFrame();

    mVideo.updateWindowSize();
    checkGraphicsSize();

    // Let the game handle continuous input while it is active
    if (mGame)
        mGame->handleInput();

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
        mVideo.present();
        mFpsManager.limitFps(config.fpsLimit);
    }
    else
    {
        mFpsManager.limitFps(10);
    }

    // TODO: Add connect timeouts
    if (mState == State::ConnectGame &&
             Net::getGameHandler()->isConnected())
    {
        Net::getLoginHandler()->disconnect();
    }
    else if (mState == State::ConnectServer &&
             mOldState == State::ConnectServer &&
             Net::getLoginHandler()->isConnected())
    {
        mState = State::Login;
    }
    else if (mState == State::WorldSelect && mOldState == State::Update)
    {
        if (Net::getLoginHandler()->getWorlds().size() < 2)
        {
            mState = State::Login;
        }
    }
    else if (mOldState == State::Start ||
             (mOldState == State::Game && mState != State::Game))
    {
        auto *top = static_cast<gcn::Container*>(gui->getTop());

        mDesktop = new Desktop;
        top->add(mDesktop);
        mSetupButton = new Button("", "Setup", this);
        mSetupButton->setToggle(true);
        mSetupButton->setButtonPopupText(_("Setup"));
        mSetupButton->setButtonIcon("button-icon-setup.png");
        setupWindow->addWidgetListener(this);
        mSetupButton->setPosition(top->getWidth() - mSetupButton->getWidth()
                                 - 3, 3);
        top->add(mSetupButton);

        mDesktop->setSize(graphics->getWidth(), graphics->getHeight());
    }

    if (mState == State::SwitchLogin && mOldState == State::Game)
    {
        Net::getGameHandler()->disconnect();
    }

    if (mState != mOldState)
    {
        {
            Event event(Event::StateChange);
            event.setInt("oldState", static_cast<int>(mOldState));
            event.setInt("newState", static_cast<int>(mState));
            event.trigger(Event::ClientChannel);
        }

        if (mOldState == State::Game)
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
            case State::ChooseServer:
                Log::info("State: CHOOSE SERVER");

                // Don't allow an alpha opacity
                // lower than the default value
                gui->getTheme()->setMinimumOpacity(0.8f);

                mCurrentDialog = new ServerDialog(&mCurrentServer,
                                                  mConfigDir);
                break;

            case State::ConnectServer:
                Log::info("State: CONNECT SERVER");

                Net::connectToServer(mCurrentServer);

                mCurrentDialog = new ConnectionDialog(
                        _("Connecting to server"), State::SwitchServer);
                break;

            case State::Login:
                Log::info("State: LOGIN");
                // Don't allow an alpha opacity
                // lower than the default value
                gui->getTheme()->setMinimumOpacity(0.8f);

                if (mOptions.username.empty() || mOptions.password.empty())
                {
                    mCurrentDialog = new LoginDialog(&loginData);
                }
                else
                {
                    mState = State::LoginAttempt;
                    // Clear the password so that when login fails, the
                    // dialog will show up next time.
                    mOptions.password.clear();
                }
                break;

            case State::LoginAttempt:
                Log::info("State: LOGIN ATTEMPT");
                accountLogin(&loginData);
                mCurrentDialog = new ConnectionDialog(
                        _("Logging in"), State::SwitchServer);
                break;

            case State::WorldSelect:
                Log::info("State: WORLD SELECT");
                {
                    Worlds worlds = Net::getLoginHandler()->getWorlds();

                    if (worlds.empty())
                    {
                        // Trust that the netcode knows what it's doing
                        mState = State::Update;
                    }
                    else if (worlds.size() == 1 || mOptions.chooseDefault)
                    {
                        Net::getLoginHandler()->chooseServer(0);
                        mState = State::Update;
                    }
                    else
                    {
                        mCurrentDialog = new WorldSelectDialog(std::move(worlds));
                    }
                }
                break;

            case State::WorldSelectAttempt:
                Log::info("State: WORLD SELECT ATTEMPT");
                mCurrentDialog = new ConnectionDialog(
                        _("Entering game world"), State::WorldSelect);
                break;

            case State::Update:
                Log::info("State: UPDATE");

                if (mOptions.skipUpdate)
                {
                    mState = State::LoadData;
                }
                else if (initUpdatesDir())
                {
                    mCurrentDialog = new UpdaterWindow(mUpdateHost,
                                                       mLocalDataDir + "/" + mUpdatesDir,
                                                       mOptions.dataPath.empty());
                }
                break;

            case State::LoadData:
                Log::info("State: LOAD DATA");

                // If another data path has been set,
                // we don't load any other files...
                if (mOptions.dataPath.empty())
                {
                    // Add customdata directory
                    ResourceManager::searchAndAddArchives(
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
                  case ServerType::TmwAthena:
                  case ServerType::Offline:
                    itemDb = new TmwAthena::TaItemDB;
                  break;
                  case ServerType::ManaServ:
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

                mState = State::GetCharacters;
                break;

            case State::GetCharacters:
                Log::info("State: GET CHARACTERS");
                Net::getCharHandler()->requestCharacters();
                mCurrentDialog = new ConnectionDialog(
                        _("Requesting characters"),
                        State::SwitchServer);
                break;

            case State::CharSelect:
                Log::info("State: CHAR SELECT");
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

            case State::ConnectGame:
                Log::info("State: CONNECT GAME");

                Net::getGameHandler()->connect();
                mCurrentDialog = new ConnectionDialog(
                        _("Connecting to the game server"),
                        Net::getNetworkType() == ServerType::TmwAthena ?
                        State::ChooseServer : State::SwitchCharacter);
                break;

            case State::ChangeMap:
                Log::info("State: CHANGE_MAP");

                Net::getGameHandler()->connect();
                mCurrentDialog = new ConnectionDialog(
                        _("Changing game servers"),
                        State::SwitchCharacter);
                break;

            case State::Game:
                if (Net::getNetworkType() != ServerType::Offline)
                {
                    Log::info("Memorizing selected character %s",
                            local_player->getName().c_str());
                    config.lastCharacter = local_player->getName();
                }

                // Fade out logon-music here too to give the desired effect
                // of "flowing" into the game.
                sound.fadeOutMusic(1000);

                // Allow any alpha opacity
                gui->getTheme()->setMinimumOpacity(0.0f);

                setupWindow->removeWidgetListener(this);
                delete mSetupButton;
                delete mDesktop;
                mSetupButton = nullptr;
                mDesktop = nullptr;

                mCurrentDialog = nullptr;

                Log::info("State: GAME");
                mGame = new Game;
                break;

            case State::LoginError:
                Log::info("State: LOGIN ERROR");
                showErrorDialog(errorMessage, State::Login);
                break;

            case State::AccountChangeError:
                Log::info("State: ACCOUNT CHANGE ERROR");
                showErrorDialog(errorMessage, State::CharSelect);
                break;

            case State::RegisterPrep:
                Log::info("State: REGISTER_PREP");
                Net::getLoginHandler()->getRegistrationDetails();
                mCurrentDialog = new ConnectionDialog(
                        _("Requesting registration details"), State::Login);
                break;

            case State::Register:
                Log::info("State: REGISTER");
                mCurrentDialog = new RegisterDialog(&loginData);
                break;

            case State::RegisterAttempt:
                Log::info("Username is %s", loginData.username.c_str());
                Net::getLoginHandler()->registerAccount(&loginData);
                loginData.password.clear();
                break;

            case State::ChangePassword:
                Log::info("State: CHANGE PASSWORD");
                mCurrentDialog = new ChangePasswordDialog(&loginData);
                break;

            case State::ChangePasswordAttempt:
                Log::info("State: CHANGE PASSWORD ATTEMPT");
                Net::getLoginHandler()->changePassword(loginData.username,
                                            loginData.password,
                                            loginData.newPassword);
                break;

            case State::ChangePasswordSuccess:
                Log::info("State: CHANGE PASSWORD SUCCESS");
                showOkDialog(_("Password Change"),
                             _("Password changed successfully!"),
                             State::CharSelect);
                loginData.password.clear();
                loginData.newPassword.clear();
                break;

            case State::ChangeEmail:
                Log::info("State: CHANGE EMAIL");
                mCurrentDialog = new ChangeEmailDialog(&loginData);
                break;

            case State::ChangeEmailAttempt:
                Log::info("State: CHANGE EMAIL ATTEMPT");
                Net::getLoginHandler()->changeEmail(loginData.email);
                break;

            case State::ChangeEmailSuccess:
                Log::info("State: CHANGE EMAIL SUCCESS");
                showOkDialog(_("Email Change"),
                             _("Email changed successfully!"),
                             State::CharSelect);
                break;

            case State::Unregister:
                Log::info("State: UNREGISTER");
                mCurrentDialog = new UnRegisterDialog(&loginData);
                break;

            case State::UnregisterAttempt:
                Log::info("State: UNREGISTER ATTEMPT");
                Net::getLoginHandler()->unregisterAccount(
                        loginData.username, loginData.password);
                break;

            case State::UnregisterSuccess:
                Log::info("State: UNREGISTER SUCCESS");
                Net::getLoginHandler()->disconnect();

                showOkDialog(_("Unregister Successful"),
                             _("Farewell, come back any time..."),
                             State::ChooseServer);
                loginData.clear();
                break;

            case State::SwitchServer:
                Log::info("State: SWITCH SERVER");

                Net::getLoginHandler()->disconnect();
                Net::getGameHandler()->disconnect();

                mCurrentServer.hostname.clear();
                mState = State::ChooseServer;
                break;

            case State::SwitchLogin:
                Log::info("State: SWITCH LOGIN");

                Net::getLoginHandler()->disconnect();

                mState = State::ConnectServer;
                break;

            case State::SwitchCharacter:
                Log::info("State: SWITCH CHARACTER");

                // Done with game
                Net::getGameHandler()->disconnect();

                mState = State::GetCharacters;
                break;

            case State::LogoutAttempt:
                Log::info("State: LOGOUT ATTEMPT");
                // TODO
                break;

            case State::Exit:
                Log::info("State: EXIT");
                break;

            case State::Error:
                Log::info("State: ERROR");
                Log::error("%s", errorMessage.c_str());
                showErrorDialog(errorMessage, State::ChooseServer);
                Net::getGameHandler()->disconnect();
                break;

            case State::Start:
                mState = State::Exit;
                break;
        }
    }
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

void Client::widgetHidden(const gcn::Event &event)
{
    if (mSetupButton && event.getSource() == setupWindow)
        mSetupButton->setSelected(false);
}

void Client::widgetShown(const gcn::Event &event)
{
    if (mSetupButton && event.getSource() == setupWindow)
        mSetupButton->setSelected(true);
}

void Client::initRootDir()
{
    mRootDir = FS::getBaseDir();
#ifdef _WIN32
    std::string portableName = mRootDir + "portable.xml";
    struct stat statbuf;

    if (!stat(portableName.c_str(), &statbuf) && S_ISREG(statbuf.st_mode))
    {
        if (!isDirWritable(mRootDir))
        {
            Log::warn("Ignoring portable mode because install directory is not writable: %s",
                      mRootDir.c_str());
            return;
        }

        Log::info("Portable file: %s", portableName.c_str());

        XML::Document doc(portableName, false);
        auto options = readOptions(doc.rootNode());

        Portable portable;
        deserialize(options, portable);
        warnUnknownOptions(portableName, options);

        if (mOptions.localDataDir.empty() && !portable.dataDir.empty())
        {
            mOptions.localDataDir = mRootDir + portable.dataDir;
            Log::info("Portable data dir: %s", mOptions.localDataDir.c_str());
        }

        if (mOptions.configDir.empty() && !portable.configDir.empty())
        {
            mOptions.configDir = mRootDir + portable.configDir;
            Log::info("Portable config dir: %s", mOptions.configDir.c_str());
        }

        if (mOptions.screenshotDir.empty() && !portable.screenshotDir.empty())
        {
            mOptions.screenshotDir = mRootDir + portable.screenshotDir;
            Log::info("Portable screenshot dir: %s",
                      mOptions.screenshotDir.c_str());
        }
    }
#endif
}

/**
 * Returns the configuration directory used by Mana 0.6 and earlier for the
 * given application name.
 */
static std::string getLegacyConfigDir(const std::string &app)
{
#if defined _WIN32
    std::string appData = getSpecialFolderLocation(FOLDERID_RoamingAppData);
    if (appData.empty())
        appData = FS::getUserDir();
    return appData + "/Mana/" + app;
#elif defined __APPLE__
    return std::string(FS::getUserDir()) + "Library/Application Support/mana/" + app;
#elif defined __HAIKU__
    // PhysFS puts preferences in "config/settings", and "Mana" was passed as
    // the application name
    return std::string(FS::getUserDir()) + "config/settings/Mana/" + app;
#else
    return std::string(FS::getUserDir()) + ".config/mana/" + app;
#endif
}

/**
 * Copies over the configuration file from the location used by Mana 0.6 and
 * earlier, unless there already is one at the current location.
 */
static void migrateLegacyConfig(const std::string &configPath)
{
    std::error_code error;

    if (std::filesystem::exists(configPath, error))
        return;

    // The application name defaulted to "manasource" here
    const std::string app = branding.appShort.empty() ? "manasource"
                                                      : branding.appShort;
    const std::string legacyPath = getLegacyConfigDir(app) + "/config.xml";

    if (!std::filesystem::exists(legacyPath, error))
        return;

    if (std::filesystem::copy_file(legacyPath, configPath, error))
        Log::info("Migrated configuration from %s", legacyPath.c_str());
    else
        Log::warn("Failed to migrate configuration from %s: %s",
                  legacyPath.c_str(), error.message().c_str());
}

/**
 * Initializes the directory in which the client stores its configuration,
 * downloaded updates and log file.
 */
void Client::initHomeDir()
{
    mLocalDataDir = mOptions.localDataDir;

    if (mLocalDataDir.empty())
    {
        if (const char *prefDir = FS::getPrefDir("manasource",
                                                 branding.shortName().c_str()))
            mLocalDataDir = prefDir;
        else
            Log::critical(_("Failed to determine the directory for storing "
                            "settings and downloads! Exiting."));
    }

    if (mkdir_r(mLocalDataDir.c_str()))
    {
        Log::critical(strprintf(_("%s doesn't exist and can't be created! "
                                  "Exiting."), mLocalDataDir.c_str()));
    }

    mConfigDir = mOptions.configDir;

    if (mConfigDir.empty())
        mConfigDir = mLocalDataDir;

    if (mkdir_r(mConfigDir.c_str()))
    {
        Log::critical(strprintf(_("%s doesn't exist and can't be created! "
                                  "Exiting."), mConfigDir.c_str()));
    }
}

/**
 * Initialize configuration.
 */
void Client::initConfiguration()
{
    // Fill configuration with defaults
    config.updatehost = branding.defaultUpdateHost;

    const std::string configPath = mConfigDir + "/client.xml";

    // Don't migrate into a directory that was given on the command line
    if (mOptions.configDir.empty() && mOptions.localDataDir.empty())
        migrateLegacyConfig(configPath);

    XML::Document doc(configPath, false);

    if (doc.rootNode() && doc.rootNode().name() == "configuration")
        deserialize(doc.rootNode(), config);
    else
        Log::info("Couldn't read configuration file: %s", configPath.c_str());
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
        Log::info("No update host provided");
        mUpdatesDir.clear();
        mState = State::LoadData;
        return false;
    }

    mUpdatesDir = "updates/" + getDirectoryFromURL(mUpdateHost);

    Log::info("Update host: %s", mUpdateHost.c_str());
    Log::info("Updates dir: %s", mUpdatesDir.c_str());

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
                Log::error("%s can't be made, but doesn't exist!",
                           newDir.c_str());
                errorMessage =
                    strprintf(_("Error creating updates directory!\n(%s)"),
                                newDir.c_str());
                mState = State::Error;
            }
#else
            Log::error("%s/%s can't be made, but doesn't exist!",
                       mLocalDataDir.c_str(), mUpdatesDir.c_str());
            errorMessage =
                strprintf(_("Error creating updates directory!\n(%s/%s)"),
                            mLocalDataDir.c_str(), mUpdatesDir.c_str());
            mState = State::Error;
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
                screenshotSuffix = branding.name();

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
    Log::info("Username is %s", loginData->username.c_str());

    // Send login infos
    if (loginData->registerLogin)
        Net::getLoginHandler()->registerAccount(loginData);
    else
        Net::getLoginHandler()->loginAccount(loginData);

    // Clear the password, avoids auto login when returning to login
    loginData->password.clear();

    // TODO This is not the best place to save the config, but at least better
    // than the login gui window
    if (config.remember)
        config.username = loginData->username;
}

/**
 * Toggles between windowed and windowed fullscreen mode. Exclusive fullscreen
 * turns into windowed mode, so that this shortcut is also a way out when the
 * current display mode turns out not to work.
 */
void Client::toggleFullscreen()
{
    VideoSettings videoSettings = mVideo.settings();
    videoSettings.windowMode = videoSettings.windowMode == WindowMode::Windowed
            ? WindowMode::WindowedFullscreen
            : WindowMode::Windowed;

    if (!mVideo.apply(videoSettings))
    {
        Log::warn("Failed to toggle fullscreen");
        return;
    }

    config.windowMode = videoSettings.windowMode;
}

void Client::handleWindowSizeChanged(int width, int height)
{
    // Store the new size in the configuration. Only the windowed size is worth
    // remembering, since a fullscreen window is as large as the display.
    if (mVideo.settings().windowMode == WindowMode::Windowed)
    {
        config.screenWidth = width;
        config.screenHeight = height;
    }
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
