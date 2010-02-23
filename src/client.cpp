/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "configuration.h"
#include "emoteshortcut.h"
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
#include "gui/skin.h"
#include "gui/login.h"
#include "gui/okdialog.h"
#include "gui/palette.h"
#include "gui/quitdialog.h"
#include "gui/register.h"
#include "gui/sdlinput.h"
#include "gui/serverdialog.h"
#include "gui/setup.h"
#include "gui/unregisterdialog.h"
#include "gui/updatewindow.h"
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

#include "resources/colordb.h"
#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/monsterdb.h"
#include "resources/npcdb.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>
#endif

#include <physfs.h>
#include <SDL_image.h>

#ifdef WIN32
#include <SDL_syswm.h>
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

static const int defaultSfxVolume = 100;
static const int defaultMusicVolume = 60;

// TODO: Get rid fo these globals
std::string errorMessage;
ErrorListener errorListener;
LoginData loginData;

Configuration config;         /**< XML file configuration reader */
Configuration branding;       /**< XML branding information reader */
Logger *logger;               /**< Log object */
KeyboardConfig keyboard;

Palette *guiPalette;
Graphics *graphics;

Sound sound;

void ErrorListener::action(const gcn::ActionEvent &)
{
    Client::setState(STATE_CHOOSE_SERVER);
}

volatile int tick_time;       /**< Tick counter */
volatile int fps = 0;         /**< Frames counted in the last second */
volatile int frame_count = 0; /**< Counts the frames during one second */

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

/**
 * @return the elapsed time in milliseconds
 * between two tick values.
 */
int get_elapsed_time(int start_time)
{
    if (start_time <= tick_time)
        return (tick_time - start_time) * MILLISECONDS_IN_A_TICK;
    else
        return (tick_time + (MAX_TICK_VALUE - start_time))
                * MILLISECONDS_IN_A_TICK;
}


// This anonymous namespace hides whatever is inside from other modules.
namespace {

class AccountListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent &)
    {
        Client::setState(STATE_CHAR_SELECT);
    }
} accountListener;

class LoginListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent &)
    {
        Client::setState(STATE_LOGIN);
    }
} loginListener;

} // anonymous namespace


Client *Client::mInstance = 0;

Client::Client(const Options &options):
    options(options),
    currentDialog(0),
    quitDialog(0),
    desktop(0),
    setupButton(0),
    state(STATE_CHOOSE_SERVER),
    oldstate(STATE_START),
    mLogicCounterId(0),
    mSecondsCounterId(0),
    mLimitFps(false)
{
    assert(!mInstance);
    mInstance = this;

    // Load branding information
    branding.init("data/branding.xml");

    initHomeDir(options);
    initScreenshotDir(options.screenshotDir);

    // Configure logger
    logger = new Logger;
    logger->setLogFile(homeDir + std::string("/mana.log"));

    // Log the mana version
    logger->log("Mana %s", FULL_VERSION);

    initConfiguration(options);
    logger->setLogToStandardOut(config.getValue("logToStandardOut", 0));

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

    if (!resman->setWriteDir(homeDir))
    {
        logger->error(strprintf("%s couldn't be set as home directory! "
                                "Exiting.", homeDir.c_str()));
    }

    // Add the user's homedir to PhysicsFS search path
    resman->addToSearchPath(homeDir, false);

    // Add the main data directories to our PhysicsFS search path
    if (!options.dataPath.empty())
        resman->addToSearchPath(options.dataPath, true);
    resman->addToSearchPath("data", true);
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
    resman->addToSearchPath(path, true);
#else
    resman->addToSearchPath(PKG_DATADIR "data", true);
#endif

#ifdef WIN32
    static SDL_SysWMinfo pInfo;
    SDL_GetWMInfo(&pInfo);
    HICON icon = LoadIcon(GetModuleHandle(NULL), "A");
    if (icon)
    {
        SetClassLong(pInfo.window, GCL_HICON, (LONG) icon);
    }
#else
    icon = IMG_Load(resman->getPath(
            branding.getValue("appIcon", "data/icons/mana.png")).c_str());
    if (icon)
    {
        SDL_SetAlpha(icon, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
        SDL_WM_SetIcon(icon, NULL);
    }
#endif

#ifdef USE_OPENGL
    bool useOpenGL = !options.noOpenGL && (config.getValue("opengl", 0) == 1);

    // Setup image loading for the right image format
    Image::setLoadAsOpenGL(useOpenGL);

    // Create the graphics context
    graphics = useOpenGL ? new OpenGLGraphics : new Graphics;
#else
    // Create the graphics context
    graphics = new Graphics;
#endif

    const int width = (int) config.getValue("screenwidth", defaultScreenWidth);
    const int height = (int) config.getValue("screenheight", defaultScreenHeight);
    const int bpp = 0;
    const bool fullscreen = ((int) config.getValue("screen", 0) == 1);
    const bool hwaccel = ((int) config.getValue("hwaccel", 0) == 1);

    // Try to set the desired video mode
    if (!graphics->setVideoMode(width, height, bpp, fullscreen, hwaccel))
    {
        logger->error(strprintf("Couldn't set %dx%dx%d video mode: %s",
            width, height, bpp, SDL_GetError()));
    }

    // Initialize for drawing
    graphics->_beginDraw();

    // Initialize the item shortcuts.
    itemShortcut = new ItemShortcut;

    // Initialize the emote shortcuts.
    emoteShortcut = new EmoteShortcut;

    gui = new Gui(graphics);

    // Initialize sound engine
    try
    {
        if (config.getValue("sound", 0) == 1)
            sound.init();

        sound.setSfxVolume((int) config.getValue("sfxVolume",
                    defaultSfxVolume));
        sound.setMusicVolume((int) config.getValue("musicVolume",
                    defaultMusicVolume));
    }
    catch (const char *err)
    {
        state = STATE_ERROR;
        errorMessage = err;
        logger->log("Warning: %s", err);
    }

    // Initialize keyboard
    keyboard.init();

    // Initialise player relations
    player_relations.init();

    guiPalette = new Palette;
    setupWindow = new Setup;

    sound.playMusic(branding.getValue("loginMusic", "Magick - Real.ogg"));

    // Initialize default server
    currentServer.hostname = options.serverName;
    currentServer.port = options.serverPort;
    loginData.username = options.username;
    loginData.password = options.password;
    loginData.remember = config.getValue("remember", 0);
    loginData.registerLogin = false;

    if (currentServer.hostname.empty())
    {
        currentServer.hostname = branding.getValue("defaultServer",
                                            "server.themanaworld.org").c_str();
    }
    if (options.serverPort == 0)
    {
        currentServer.port = (short) branding.getValue("defaultPort",
                                                       DEFAULT_PORT);
    }
    if (loginData.username.empty() && loginData.remember)
        loginData.username = config.getValue("username", "");

    if (state != STATE_ERROR)
        state = STATE_CHOOSE_SERVER;

    // Initialize logic and seconds counters
    tick_time = 0;
    mLogicCounterId = SDL_AddTimer(MILLISECONDS_IN_A_TICK, nextTick, NULL);
    mSecondsCounterId = SDL_AddTimer(1000, nextSecond, NULL);

    // Initialize frame limiting
    SDL_initFramerate(&mFpsManager);
    config.addListener("fpslimit", this);
    optionChanged("fpslimit");
}

Client::~Client()
{
    SDL_RemoveTimer(mLogicCounterId);
    SDL_RemoveTimer(mSecondsCounterId);

    // Before config.write() since it writes the shortcuts to the config
    delete itemShortcut;
    delete emoteShortcut;

    config.write();

    delete gui;
    delete graphics;

    // Shutdown libxml
    xmlCleanupParser();

    // Shutdown sound
    sound.close();

    // Unload XML databases
    ColorDB::unload();
    EmoteDB::unload();
    ItemDB::unload();
    MonsterDB::unload();
    NPCDB::unload();
    StatusEffect::unload();

    ResourceManager::deleteInstance();

    SDL_FreeSurface(icon);

    logger->log("Quitting");
    delete guiPalette;
    delete logger;

    mInstance = 0;
}

int Client::exec()
{
    int lastTickTime = tick_time;

    Game *game = 0;
    SDL_Event event;

    while (state != STATE_EXIT)
    {
        bool handledEvents = false;

        if (game)
        {
            // Let the game handle the events while it is active
            game->handleInput();
        }
        else
        {
            // Handle SDL events
            while (SDL_PollEvent(&event))
            {
                handledEvents = true;

                switch (event.type)
                {
                    case SDL_QUIT:
                        state = STATE_EXIT;
                        break;

                    case SDL_KEYDOWN:
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
            if (game)
                game->logic();

            ++lastTickTime;
        }

        // This is done because at some point tick_time will wrap.
        lastTickTime = tick_time;

        // Update the screen when application is active, delay otherwise.
        if (SDL_GetAppState() & SDL_APPACTIVE)
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
        if (state == STATE_CONNECT_GAME &&
                 Net::getGameHandler()->isConnected())
        {
            Net::getLoginHandler()->disconnect();
        }
        else if (state == STATE_CONNECT_SERVER && oldstate == STATE_CHOOSE_SERVER)
        {
            Net::connectToServer(currentServer);
        }
        else if (state == STATE_CONNECT_SERVER &&
                 oldstate != STATE_CHOOSE_SERVER &&
                 Net::getLoginHandler()->isConnected())
        {
            state = STATE_LOGIN;
        }
        else if (state == STATE_WORLD_SELECT && oldstate == STATE_UPDATE)
        {
            if (Net::getLoginHandler()->getWorlds().size() < 2)
            {
                state = STATE_LOGIN;
            }
        }
        else if (oldstate == STATE_START ||
                 (oldstate == STATE_GAME && state != STATE_GAME))
        {
            gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());

            desktop = new Desktop;
            top->add(desktop);
            setupButton = new Button(_("Setup"), "Setup", this);
            setupButton->setPosition(top->getWidth() - setupButton->getWidth()
                                     - 3, 3);
            top->add(setupButton);

            int screenWidth = (int) config.getValue("screenwidth",
                                                    defaultScreenWidth);
            int screenHeight = (int) config.getValue("screenheight",
                                                     defaultScreenHeight);

            desktop->setSize(screenWidth, screenHeight);
        }

        if (state == STATE_SWITCH_LOGIN && oldstate == STATE_GAME)
        {
            Net::getGameHandler()->disconnect();
        }

        if (state != oldstate)
        {
            if (oldstate == STATE_GAME)
            {
                delete game;
                game = 0;
            }

            oldstate = state;

            // Get rid of the dialog of the previous state
            if (currentDialog)
            {
                delete currentDialog;
                currentDialog = NULL;
            }
            // State has changed, while the quitDialog was active, it might
            // not be correct anymore
            if (quitDialog)
                quitDialog->scheduleDelete();

            switch (state)
            {
                case STATE_CHOOSE_SERVER:
                    logger->log("State: CHOOSE SERVER");

                    // Allow changing this using a server choice dialog
                    // We show the dialog box only if the command-line
                    // options weren't set.
                    if (options.serverName.empty() && options.serverPort == 0)
                    {
                        // Don't allow an alpha opacity
                        // lower than the default value
                        SkinLoader::instance()->setMinimumOpacity(0.8f);

                        currentDialog = new ServerDialog(&currentServer,
                                                         homeDir);
                    }
                    else
                    {
                        state = STATE_CONNECT_SERVER;

                        // Reset options so that cancelling or connect
                        // timeout will show the server dialog.
                        options.serverName.clear();
                        options.serverPort = 0;
                    }
                    break;

                case STATE_CONNECT_SERVER:
                    logger->log("State: CONNECT SERVER");
                    currentDialog = new ConnectionDialog(
                            _("Connecting to server"), STATE_SWITCH_SERVER);
                    break;

                case STATE_LOGIN:
                    logger->log("State: LOGIN");
                    // Don't allow an alpha opacity
                    // lower than the default value
                    SkinLoader::instance()->setMinimumOpacity(0.8f);

                    if (options.username.empty()
                            || options.password.empty())
                    {
                        currentDialog = new LoginDialog(&loginData);
                    }
                    else
                    {
                        state = STATE_LOGIN_ATTEMPT;
                        // Clear the password so that when login fails, the
                        // dialog will show up next time.
                        options.password.clear();
                    }
                    break;

                case STATE_LOGIN_ATTEMPT:
                    logger->log("State: LOGIN ATTEMPT");
                    accountLogin(&loginData);
                    currentDialog = new ConnectionDialog(
                            _("Logging in"), STATE_SWITCH_SERVER);
                    break;

                case STATE_WORLD_SELECT:
                    logger->log("State: WORLD SELECT");
                    {
                        Worlds worlds = Net::getLoginHandler()->getWorlds();

                        if (worlds.size() == 0)
                        {
                            // Trust that the netcode knows what it's doing
                            state = STATE_UPDATE;
                        }
                        else if (worlds.size() == 1)
                        {
                            Net::getLoginHandler()->chooseServer(0);
                            state = STATE_UPDATE;
                        }
                        else
                        {
                            currentDialog = new WorldSelectDialog(worlds);
                            if (options.chooseDefault)
                            {
                                ((WorldSelectDialog*) currentDialog)->action(
                                    gcn::ActionEvent(NULL, "ok"));
                            }
                        }
                    }
                    break;

                case STATE_WORLD_SELECT_ATTEMPT:
                    logger->log("State: WORLD SELECT ATTEMPT");
                    currentDialog = new ConnectionDialog(
                            _("Entering game world"), STATE_WORLD_SELECT);
                    break;

                case STATE_UPDATE:
                    // Determine which source to use for the update host
                    if (!options.updateHost.empty())
                        updateHost = options.updateHost;
                    else
                        updateHost = loginData.updateHost;
                    initUpdatesDir();

                    if (options.skipUpdate)
                    {
                        state = STATE_LOAD_DATA;
                    }
                    else
                    {
                        logger->log("State: UPDATE");
                        currentDialog = new UpdaterWindow(updateHost,
                                homeDir + "/" + updatesDir,options.dataPath.empty());
                    }
                    break;

                case STATE_LOAD_DATA:
                    logger->log("State: LOAD DATA");

                    // If another data path has been set,
                    // we don't load any other files...
                    if (options.dataPath.empty())
                    {
                        // Add customdata directory
                        ResourceManager::getInstance()->searchAndAddArchives(
                            "customdata/",
                            "zip",
                            false);
                    }

                    // Load XML databases
                    ColorDB::load();
                    ItemDB::load();
                    Being::load(); // Hairstyles
                    MonsterDB::load();
                    NPCDB::load();
                    EmoteDB::load();
                    StatusEffect::load();
                    Units::loadUnits();

                    desktop->reloadWallpaper();

                    state = STATE_GET_CHARACTERS;
                    break;

                case STATE_GET_CHARACTERS:
                    logger->log("State: GET CHARACTERS");
                    Net::getCharHandler()->requestCharacters();
                    currentDialog = new ConnectionDialog(
                            _("Requesting characters"),
                            STATE_SWITCH_SERVER);
                    break;

                case STATE_CHAR_SELECT:
                    logger->log("State: CHAR SELECT");
                    // Don't allow an alpha opacity
                    // lower than the default value
                    SkinLoader::instance()->setMinimumOpacity(0.8f);

                    currentDialog = new CharSelectDialog(&loginData);

                    if (!((CharSelectDialog*) currentDialog)->selectByName(
                            options.character, CharSelectDialog::Choose))
                    {
                        ((CharSelectDialog*) currentDialog)->selectByName(
                                config.getValue("lastCharacter", ""),
                                options.chooseDefault ?
                                    CharSelectDialog::Choose :
                                    CharSelectDialog::Focus);
                    }

                    break;

                case STATE_CONNECT_GAME:
                    logger->log("State: CONNECT GAME");

                    Net::getGameHandler()->connect();
                    currentDialog = new ConnectionDialog(
                            _("Connecting to the game server"),
                            STATE_SWITCH_CHARACTER);
                    break;

                case STATE_GAME:
                    logger->log("Memorizing selected character %s",
                            player_node->getName().c_str());
                    config.setValue("lastCharacter", player_node->getName());

                    Net::getGameHandler()->inGame();

                    // Fade out logon-music here too to give the desired effect
                    // of "flowing" into the game.
                    sound.fadeOutMusic(1000);

                    // Allow any alpha opacity
                    SkinLoader::instance()->setMinimumOpacity(-1.0f);

                    delete setupButton;
                    delete desktop;
                    setupButton = NULL;
                    desktop = NULL;

                    currentDialog = NULL;

                    logger->log("State: GAME");
                    game = new Game;
                    break;

                case STATE_LOGIN_ERROR:
                    logger->log("State: LOGIN ERROR");
                    currentDialog = new OkDialog(_("Error"), errorMessage);
                    currentDialog->addActionListener(&loginListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    break;

                case STATE_ACCOUNTCHANGE_ERROR:
                    logger->log("State: ACCOUNT CHANGE ERROR");
                    currentDialog = new OkDialog(_("Error"), errorMessage);
                    currentDialog->addActionListener(&accountListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    break;

                case STATE_REGISTER_PREP:
                    logger->log("State: REGISTER_PREP");
                    Net::getLoginHandler()->getRegistrationDetails();
                    currentDialog = new ConnectionDialog(
                            _("Requesting registration details"), STATE_LOGIN);
                    break;

                case STATE_REGISTER:
                    logger->log("State: REGISTER");
                    currentDialog = new RegisterDialog(&loginData);
                    break;

                case STATE_REGISTER_ATTEMPT:
                    logger->log("Username is %s", loginData.username.c_str());
                    Net::getLoginHandler()->registerAccount(&loginData);
                    break;

                case STATE_CHANGEPASSWORD:
                    logger->log("State: CHANGE PASSWORD");
                    currentDialog = new ChangePasswordDialog(&loginData);
                    break;

                case STATE_CHANGEPASSWORD_ATTEMPT:
                    logger->log("State: CHANGE PASSWORD ATTEMPT");
                    Net::getLoginHandler()->changePassword(loginData.username,
                                                loginData.password,
                                                loginData.newPassword);
                    break;

                case STATE_CHANGEPASSWORD_SUCCESS:
                    logger->log("State: CHANGE PASSWORD SUCCESS");
                    currentDialog = new OkDialog(_("Password Change"),
                            _("Password changed successfully!"));
                    currentDialog->addActionListener(&accountListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    loginData.password = loginData.newPassword;
                    loginData.newPassword = "";
                    break;

                case STATE_CHANGEEMAIL:
                    logger->log("State: CHANGE EMAIL");
                    currentDialog = new ChangeEmailDialog(&loginData);
                    break;

                case STATE_CHANGEEMAIL_ATTEMPT:
                    logger->log("State: CHANGE EMAIL ATTEMPT");
                    Net::getLoginHandler()->changeEmail(loginData.email);
                    break;

                case STATE_CHANGEEMAIL_SUCCESS:
                    logger->log("State: CHANGE EMAIL SUCCESS");
                    currentDialog = new OkDialog(_("Email Change"),
                            _("Email changed successfully!"));
                    currentDialog->addActionListener(&accountListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    break;

                case STATE_UNREGISTER:
                    logger->log("State: UNREGISTER");
                    currentDialog = new UnRegisterDialog(&loginData);
                    break;

                case STATE_UNREGISTER_ATTEMPT:
                    logger->log("State: UNREGISTER ATTEMPT");
                    Net::getLoginHandler()->unregisterAccount(
                            loginData.username, loginData.password);
                    break;

                case STATE_UNREGISTER_SUCCESS:
                    logger->log("State: UNREGISTER SUCCESS");
                    Net::getLoginHandler()->disconnect();

                    currentDialog = new OkDialog(_("Unregister Successful"),
                            _("Farewell, come back any time..."));
                    loginData.clear();
                    //The errorlistener sets the state to STATE_CHOOSE_SERVER
                    currentDialog->addActionListener(&errorListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    break;

                case STATE_SWITCH_SERVER:
                    logger->log("State: SWITCH SERVER");

                    Net::getLoginHandler()->disconnect();
                    Net::getGameHandler()->disconnect();

                    state = STATE_CHOOSE_SERVER;
                    break;

                case STATE_SWITCH_LOGIN:
                    logger->log("State: SWITCH LOGIN");

                    Net::getLoginHandler()->logout();

                    state = STATE_LOGIN;
                    break;

                case STATE_SWITCH_CHARACTER:
                    logger->log("State: SWITCH CHARACTER");

                    // Done with game
                    Net::getGameHandler()->disconnect();

                    Net::getCharHandler()->requestCharacters();
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
                    state = STATE_EXIT;
                  break;

                case STATE_ERROR:
                    logger->log("State: ERROR");
                    currentDialog = new OkDialog(_("Error"), errorMessage);
                    currentDialog->addActionListener(&errorListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    Net::getGameHandler()->disconnect();
                    break;

                default:
                    state = STATE_FORCE_QUIT;
                    break;
            }
        }
    }

    return 0;
}

void Client::optionChanged(const std::string &name)
{
    const int fpsLimit = (int) config.getValue("fpslimit", 60);
    mLimitFps = fpsLimit > 0;
    if (mLimitFps)
        SDL_setFramerate(&mFpsManager, fpsLimit);
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
}

/**
 * Initializes the home directory. On UNIX and FreeBSD, ~/.mana is used. On
 * Windows and other systems we use the current working directory.
 */
void Client::initHomeDir(const Options &options)
{
    homeDir = options.homeDir;

    if (homeDir.empty())
    {
#ifdef __APPLE__
        // Use Application Directory instead of .mana
        homeDir = std::string(PHYSFS_getUserDir()) +
            "/Library/Application Support/" +
            branding.getValue("appName", "Mana");
#else
        homeDir = std::string(PHYSFS_getUserDir()) +
            "/." + branding.getValue("appShort", "mana");
#endif
    }
#if defined WIN32
    if (!CreateDirectory(homeDir.c_str(), 0) &&
            GetLastError() != ERROR_ALREADY_EXISTS)
#else
    // Create home directory if it doesn't exist already
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
#endif
    {
        logger->error(strprintf(_("%s doesn't exist and can't be created! "
                                  "Exiting."), homeDir.c_str()));
    }
}

/**
 * Initialize configuration.
 */
void Client::initConfiguration(const Options &options)
{
    // Fill configuration with defaults
    logger->log("Initializing configuration...");
    std::string defaultHost = branding.getValue("defaultServer",
        "server.themanaworld.org");
    int defaultPort = (int) branding.getValue("defaultPort", DEFAULT_PORT);
    config.setValue("port", defaultPort);
    config.setValue("hwaccel", false);
#if (defined __APPLE__ || defined WIN32) && defined USE_OPENGL
    config.setValue("opengl", true);
#else
    config.setValue("opengl", false);
#endif
    config.setValue("screen", false);
    config.setValue("sound", true);
    config.setValue("guialpha", 0.8f);
    config.setValue("remember", true);
    config.setValue("sfxVolume", 100);
    config.setValue("musicVolume", 60);
    config.setValue("fpslimit", 60);
    std::string defaultUpdateHost = branding.getValue("defaultUpdateHost",
        "http://updates.themanaworld.org");
    config.setValue("updatehost", defaultUpdateHost);
    config.setValue("customcursor", true);
    config.setValue("ChatLogLength", 128);

    // Checking if the configuration file exists... otherwise create it with
    // default options.
    FILE *configFile = 0;
    std::string configPath = options.configPath;

    if (configPath.empty())
        configPath = homeDir + "/config.xml";

    configFile = fopen(configPath.c_str(), "r");

    // If we can't read it, it doesn't exist !
    if (configFile == NULL)
    {
        // We reopen the file in write mode and we create it
        configFile = fopen(configPath.c_str(), "wt");
    }
    if (configFile == NULL)
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
    std::stringstream updates;

    // If updatesHost is currently empty, fill it from config file
    if (updateHost.empty())
    {
        updateHost =
            config.getValue("updatehost", "http://updates.themanaworld.org/");
    }

    // Remove any trailing slash at the end of the update host
    if (updateHost.at(updateHost.size() - 1) == '/')
        updateHost.resize(updateHost.size() - 1);

    // Parse out any "http://" or "ftp://", and set the updates directory
    size_t pos;
    pos = updateHost.find("://");
    if (pos != updateHost.npos)
    {
        if (pos + 3 < updateHost.length())
        {
            updates << "updates/" << updateHost.substr(pos + 3);
            updatesDir = updates.str();
        }
        else
        {
            logger->log("Error: Invalid update host: %s", updateHost.c_str());
            errorMessage = strprintf(_("Invalid update host: %s"),
                                     updateHost.c_str());
            state = STATE_ERROR;
        }
    }
    else
    {
        logger->log("Warning: no protocol was specified for the update host");
        updates << "updates/" << updateHost;
        updatesDir = updates.str();
    }

    ResourceManager *resman = ResourceManager::getInstance();

    // Verify that the updates directory exists. Create if necessary.
    if (!resman->isDirectory("/" + updatesDir))
    {
        if (!resman->mkdir("/" + updatesDir))
        {
#if defined WIN32
            std::string newDir = homeDir + "\\" + updatesDir;
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
                errorMessage = _("Error creating updates directory!");
                state = STATE_ERROR;
            }
#else
            logger->log("Error: %s/%s can't be made, but doesn't exist!",
                        homeDir.c_str(), updatesDir.c_str());
            errorMessage = _("Error creating updates directory!");
            state = STATE_ERROR;
#endif
        }
    }
}

void Client::initScreenshotDir(const std::string &dir)
{
    if (dir.empty())
    {
        screenshotDir = std::string(PHYSFS_getUserDir()) + "Desktop";
        // If ~/Desktop does not exist, we save screenshots in the user's home.
        struct stat statbuf;
        if (stat(screenshotDir.c_str(), &statbuf))
            screenshotDir = std::string(PHYSFS_getUserDir());
    }
    else
        screenshotDir = dir;
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
