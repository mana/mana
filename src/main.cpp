/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "main.h"

#include <getopt.h>
#include <iostream>
#include <physfs.h>
#include <unistd.h>
#include <vector>
#include <SDL_image.h>

#include <guichan/actionlistener.hpp>
#include <guichan/widgets/label.hpp>

#include <libxml/parser.h>

#ifdef WIN32
#include <SDL_syswm.h>
#endif
#ifndef WIN32
#include <cerrno>
#include <sys/stat.h>
#endif
#if defined __APPLE__
#include <CoreFoundation/CFBundle.h>
#endif

#include "configuration.h"
#include "keyboardconfig.h"
#include "game.h"
#include "graphics.h"
#include "itemshortcut.h"
#include "lockedarray.h"
#include "localplayer.h"
#include "log.h"
#include "logindata.h"
#ifdef USE_OPENGL
#include "openglgraphics.h"
#endif
#include "sound.h"

#include "gui/char_select.h"
#include "gui/connection.h"
#include "gui/gui.h"
#include "gui/login.h"
#include "gui/ok_dialog.h"
#include "gui/progressbar.h"
#include "gui/quitdialog.h"
#include "gui/register.h"
#include "gui/sdlinput.h"
#include "gui/serverdialog.h"
#include "gui/textfield.h"
#include "gui/updatewindow.h"

#include "net/charserverhandler.h"
#include "net/connection.h"
#include "net/loginhandler.h"
#include "net/logouthandler.h"
#include "net/network.h"

#include "net/accountserver/accountserver.h"
#include "net/accountserver/account.h"

#include "net/chatserver/chatserver.h"

#include "net/gameserver/gameserver.h"

#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/monsterdb.h"
#include "resources/npcdb.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/tostring.h"

std::string token; //used to store magic_token

Graphics *graphics;

unsigned char state;
std::string errorMessage;
std::string homeDir;
unsigned char screen_mode;

Sound sound;
Music *bgm;

Configuration config;         /**< XML file configuration reader */
Logger *logger;               /**< Log object */
KeyboardConfig keyboard;

Net::Connection *accountServerConnection = 0;
Net::Connection *gameServerConnection = 0;
Net::Connection *chatServerConnection = 0;

/**
 * A structure holding the values of various options that can be passed from
 * the command line.
 */
struct Options
{
    /**
     * Constructor.
     */
    Options():
        printHelp(false),
        printVersion(false),
        skipUpdate(false),
        chooseDefault(false),
        serverPort(0)
    {};

    bool printHelp;
    bool printVersion;
    bool skipUpdate;
    bool chooseDefault;
    std::string playername;
    std::string password;
    std::string configPath;
    std::string dataPath;

    std::string serverName;
    short serverPort;
};

/**
 * Initializes the home directory. On UNIX and FreeBSD, ~/.tmw is used. On
 * Windows and other systems we use the current working directory.
 */
void initHomeDir()
{
    homeDir = std::string(PHYSFS_getUserDir()) + "/.tmw";
#if defined WIN32
    if (!CreateDirectory(homeDir.c_str(), 0) &&
            GetLastError() != ERROR_ALREADY_EXISTS)
#else
    // Checking if /home/user/.tmw folder exists.
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
#endif
    {
        std::cout << homeDir
                  << " can't be created, but it doesn't exist! Exiting."
                  << std::endl;
        exit(1);
    }
}

/**
 * Initialize configuration.
 */
void initConfiguration(const Options &options)
{
    // Fill configuration with defaults
    logger->log("Initializing configuration...");
    config.setValue("host", "server.themanaworld.org");
    config.setValue("port", 9601);
    config.setValue("hwaccel", 0);
#if (defined __APPLE__ || defined WIN32) && defined USE_OPENGL
    config.setValue("opengl", 1);
#else
    config.setValue("opengl", 0);
#endif
    config.setValue("screen", 0);
    config.setValue("sound", 1);
    config.setValue("guialpha", 0.8f);
    config.setValue("remember", 1);
    config.setValue("sfxVolume", 100);
    config.setValue("musicVolume", 60);
    config.setValue("fpslimit", 0);
    config.setValue("updatehost", "http://updates.themanaworld.org");
    config.setValue("customcursor", 1);
    config.setValue("ChatLogLength", 128);

    // Checking if the configuration file exists... otherwise create it with
    // default options.
    FILE *tmwFile = 0;
    std::string configPath = options.configPath;
    if (configPath == "") {
        configPath = homeDir + "/config.xml";
    }
    tmwFile = fopen(configPath.c_str(), "r");

    // If we can't read it, it doesn't exist !
    if (tmwFile == NULL) {
        // We reopen the file in write mode and we create it
        tmwFile = fopen(configPath.c_str(), "wt");
    }
    if (tmwFile == NULL) {
        std::cout << "Can't create " << configPath << ". "
                  << "Using Defaults." << std::endl;
    } else {
        fclose(tmwFile);
        config.init(configPath);
    }
}

/**
 * Do all initialization stuff.
 */
void initEngine(const Options &options)
{
    // Initialize SDL
    logger->log("Initializing SDL...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << "Could not initialize SDL: " <<
            SDL_GetError() << std::endl;
        exit(1);
    }
    atexit(SDL_Quit);

    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    SDL_WM_SetCaption("The Mana World", NULL);
#ifdef WIN32
    static SDL_SysWMinfo pInfo;
    SDL_GetWMInfo(&pInfo);
    HICON icon = LoadIcon(GetModuleHandle(NULL), "A");
    if (icon)
    {
        SetClassLong(pInfo.window, GCL_HICON, (LONG) icon);
    }
#else
    SDL_Surface *icon = IMG_Load(TMW_DATADIR "data/icons/tmw.png");
    if (icon)
    {
        SDL_SetAlpha(icon, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
        SDL_WM_SetIcon(icon, NULL);
    }
#endif

    ResourceManager *resman = ResourceManager::getInstance();

    if (!resman->setWriteDir(homeDir)) {
        std::cout << homeDir
                  << " couldn't be set as home directory! Exitting."
                  << std::endl;
        exit(1);
    }

    // Add the user's homedir to PhysicsFS search path
    resman->addToSearchPath(homeDir, false);
    // Creating and checking the updates folder existence and rights.
    if (!resman->isDirectory("/updates")) {
        if (!resman->mkdir("/updates")) {
            std::cout << homeDir << "/updates "
                      << "can't be made, but it doesn't exist! Exitting."
                      << std::endl;
            exit(1);
        }
    }

    // Add the main data directories to our PhysicsFS search path
    if (!options.dataPath.empty()) {
        resman->addToSearchPath(options.dataPath, true);
    }
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
    resman->addToSearchPath(TMW_DATADIR "data", true);
#endif

#ifdef USE_OPENGL
    bool useOpenGL = (config.getValue("opengl", 0) == 1);

    // Setup image loading for the right image format
    Image::setLoadAsOpenGL(useOpenGL);

    // Create the graphics context
    graphics = useOpenGL ? new OpenGLGraphics() : new Graphics();
#else
    // Create the graphics context
    graphics = new Graphics();
#endif

    int width = (int) config.getValue("screenwidth", defaultScreenWidth);
    int height = (int) config.getValue("screenheight", defaultScreenHeight);
    int bpp = 0;
    bool fullscreen = ((int) config.getValue("screen", 0) == 1);
    bool hwaccel = ((int) config.getValue("hwaccel", 0) == 1);

    // Try to set the desired video mode
    if (!graphics->setVideoMode(width, height, bpp, fullscreen, hwaccel))
    {
        std::cerr << "Couldn't set "
                  << width << "x" << height << "x" << bpp << " video mode: "
                  << SDL_GetError() << std::endl;
        exit(1);
    }

    // Initialize for drawing
    graphics->_beginDraw();

    // Initialize the item shortcuts.
    itemShortcut = new ItemShortcut();

    gui = new Gui(graphics);
    state = STATE_CHOOSE_SERVER; /**< Initial game state */

    // Initialize sound engine
    try {
        if (config.getValue("sound", 0) == 1) {
            sound.init();
        }
        sound.setSfxVolume((int) config.getValue("sfxVolume",
                    defaultSfxVolume));
        sound.setMusicVolume((int) config.getValue("musicVolume",
                    defaultMusicVolume));
    }
    catch (const char *err) {
        state = STATE_ERROR;
        errorMessage = err;
        logger->log("Warning: %s", err);
    }

    // Initialize keyboard
    keyboard.init();
}

/** Clear the engine */
void exit_engine()
{
    // Before config.write() since it writes the shortcuts to the config
    delete itemShortcut;

    config.write();

    delete gui;
    delete graphics;

    // Shutdown libxml
    xmlCleanupParser();

    // Shutdown sound
    sound.close();

    // Unload XML databases
    ItemDB::unload();
    MonsterDB::unload();
    NPCDB::unload();

    ResourceManager::deleteInstance();
}

void printHelp()
{
    std::cout <<
        "tmw\n\n"
        "Options:\n"
        "  -h --help       : Display this help\n"
        "  -v --version    : Display the version\n"
        "  -u --skipupdate : Skip the update process\n"
        "  -d --data       : Directory to load game data from\n"
        "  -U --username   : Login with this username\n"
        "  -P --password   : Login with this password\n"
        "  -D --default    : Bypass the login process with default settings\n"
        "  -s --server     : Login Server name or IP\n"
        "  -o --port       : Login Server Port\n"
        "  -p --playername : Login with this player\n"
        "  -C --configfile : Configuration file to use\n";
}

void printVersion()
{
#ifdef PACKAGE_VERSION
    std::cout << "The Mana World version " << PACKAGE_VERSION << std::endl;
#else
    std::cout << "The Mana World version " <<
             "(local build?, PACKAGE_VERSION is not defined)" << std::endl;
#endif
}

void parseOptions(int argc, char *argv[], Options &options)
{
    const char *optstring = "hvud:U:P:Dp:s:o:C:";

    const struct option long_options[] = {
        { "help",       no_argument,       0, 'h' },
        { "version",    no_argument,       0, 'v' },
        { "skipupdate", no_argument,       0, 'u' },
        { "data",       required_argument, 0, 'd' },
        { "username",   required_argument, 0, 'U' },
        { "password",   required_argument, 0, 'P' },
        { "default",    no_argument,       0, 'D' },
        { "server",     required_argument, 0, 's' },
        { "port",       required_argument, 0, 'o' },
        { "playername", required_argument, 0, 'p' },
        { "configfile", required_argument, 0, 'C' },
        { 0 }
    };

    while (optind < argc) {
        int result = getopt_long(argc, argv, optstring, long_options, NULL);

        if (result == -1)
            break;

        switch (result) {
            default: // Unknown option
            case 'h':
                options.printHelp = true;
                break;
            case 'v':
                options.printVersion = true;
                break;
            case 'u':
                options.skipUpdate = true;
                break;
            case 'd':
                options.dataPath = optarg;
                break;
            case 'U':
                options.playername = optarg;
                break;
            case 'P':
                options.password = optarg;
                break;
            case 'D':
                options.chooseDefault = true;
                break;
            case 's':
                options.serverName = optarg;
                break;
            case 'o':
                options.serverPort = (short)atoi(optarg);
                break;
            case 'p':
                options.playername = optarg;
                break;
            case 'C':
                options.configPath = optarg;
                break;
        }
    }
}

/**
 * Reads the file "updates/resources2.txt" and attempts to load each update
 * mentioned in it.
 */
void loadUpdates()
{
    const std::string updatesFile = "updates/resources2.txt";
    ResourceManager *resman = ResourceManager::getInstance();
    std::vector<std::string> lines = resman->loadTextFile(updatesFile);

    for (unsigned int i = 0; i < lines.size(); ++i)
    {
        std::stringstream line(lines[i]);
        std::string filename;
        line >> filename;
        resman->addToSearchPath(homeDir + "/updates/" + filename, false);
    }
}

CharServerHandler charServerHandler;
LoginData loginData;
LoginHandler loginHandler;
LogoutHandler logoutHandler;
LockedArray<LocalPlayer*> charInfo(maxSlot + 1);

namespace {
    struct ErrorListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            state = STATE_CHOOSE_SERVER;
        }
    } errorListener;

    struct AccountListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            state = STATE_CHAR_SELECT;
        }
    } accountListener;

    struct LoginListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            state = STATE_LOGIN;
        }
    } loginListener;
}

// TODO Find some nice place for these functions
void accountLogin(LoginData *loginData)
{
    logger->log("Username is %s", loginData->username.c_str());

    Net::registerHandler(&loginHandler);

    charServerHandler.setCharInfo(&charInfo);
    Net::registerHandler(&charServerHandler);

    // Send login infos
    Net::AccountServer::login(accountServerConnection, 0,
            loginData->username, loginData->password);

    // Clear the password, avoids auto login when returning to login
    loginData->password = "";

    // TODO This is not the best place to save the config, but at least better
    // than the login gui window
    if (loginData->remember)
    {
        config.setValue("host", loginData->hostname);
        config.setValue("username", loginData->username);
    }
    config.setValue("remember", loginData->remember);
}

void accountRegister(LoginData *loginData)
{
    logger->log("Username is %s", loginData->username.c_str());

    Net::registerHandler(&loginHandler);

    charServerHandler.setCharInfo(&charInfo);
    Net::registerHandler(&charServerHandler);

    Net::AccountServer::registerAccount(accountServerConnection, 0,
            loginData->username, loginData->password, loginData->email);
}

void accountUnRegister(LoginData *loginData)
{
    Net::registerHandler(&logoutHandler);

    Net::AccountServer::Account::unregister(loginData->username,
                                                         loginData->password);

}

void accountChangePassword(LoginData *loginData)
{
    Net::registerHandler(&loginHandler);

    Net::AccountServer::Account::changePassword(loginData->password,
                                                loginData->newPassword);
}

void switchCharacter(std::string* passToken)
{
    Net::registerHandler(&logoutHandler);

    logoutHandler.reset();
    logoutHandler.setScenario(LOGOUT_SWITCH_CHARACTER, passToken);

    Net::GameServer::logout(true);
    Net::ChatServer::logout();
}

void switchAccountServer()
{
    Net::registerHandler(&logoutHandler);

    logoutHandler.reset();
    logoutHandler.setScenario(LOGOUT_SWITCH_ACCOUNTSERVER);

    //Can't logout if we were not logged in ...
    if (accountServerConnection->isConnected())
    {
        Net::AccountServer::logout();
    }
    else
    {
        logoutHandler.setAccountLoggedOut();
    }

    if (gameServerConnection->isConnected())
    {
        Net::GameServer::logout(false);
    }
    else
    {
        logoutHandler.setGameLoggedOut();
    }

    if (chatServerConnection->isConnected())
    {
        Net::ChatServer::logout();
    }
    else
    {
        logoutHandler.setChatLoggedOut();
    }
}

void logoutThenExit()
{
    Net::registerHandler(&logoutHandler);

    logoutHandler.reset();
    logoutHandler.setScenario(LOGOUT_EXIT);

    // Can't logout if we were not logged in ...
    if (accountServerConnection->isConnected())
    {
        Net::AccountServer::logout();
    }
    else
    {
        logoutHandler.setAccountLoggedOut();
    }

    if (gameServerConnection->isConnected())
    {
        Net::GameServer::logout(false);
    }
    else
    {
        logoutHandler.setGameLoggedOut();
    }

    if (chatServerConnection->isConnected())
    {
        Net::ChatServer::logout();
    }
    else
    {
        logoutHandler.setChatLoggedOut();
    }
}

void reconnectAccount(const std::string& passToken)
{
    Net::registerHandler(&loginHandler);

    charServerHandler.setCharInfo(&charInfo);
    Net::registerHandler(&charServerHandler);

    Net::AccountServer::reconnectAccount(accountServerConnection, passToken);
}

void xmlNullLogger(void *ctx, const char *msg, ...)
{
    // Does nothing, that's the whole point of it
}

// Initialize libxml2 and check for potential ABI mismatches between
// compiled version and the shared library actually used.
void initXML()
{
    logger->log("Initializing libxml2...");
    xmlInitParser();
    LIBXML_TEST_VERSION;

    // Suppress libxml2 error messages
    xmlSetGenericErrorFunc(NULL, xmlNullLogger);
}

extern "C" char const *_nl_locale_name_default(void);

/** Main */
int main(int argc, char *argv[])
{
    try
    {
        // Parse command line options
        Options options;
        parseOptions(argc, argv, options);
        if (options.printHelp)
        {
            printHelp();
            return 0;
        }
        else if (options.printVersion)
        {
            printVersion();
            return 0;
        }

#if ENABLE_NLS
#ifdef WIN32
        putenv(("LANG=" + std::string(_nl_locale_name_default())).c_str());
#endif
        setlocale(LC_MESSAGES, "");
        bindtextdomain("tmw", LOCALEDIR);
        bind_textdomain_codeset("tmw", "UTF-8");
        textdomain("tmw");
#endif

        // Initialize PhysicsFS
        PHYSFS_init(argv[0]);

        initHomeDir();
        // Configure logger
        logger = new Logger();
        logger->setLogFile(homeDir + std::string("/tmw.log"));
        logger->setLogToStandardOut(config.getValue("logToStandardOut", 0));

        // Log the tmw version
#ifdef PACKAGE_VERSION
        logger->log("The Mana World v%s", PACKAGE_VERSION);
#else
        logger->log("The Mana World - version not defined");
#endif

        initXML();
        initConfiguration(options);
        initEngine(options);

        Game *game = NULL;
        Window *currentDialog = NULL;
        QuitDialog* quitDialog = NULL;
        Image *login_wallpaper = NULL;

        gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());
#ifdef PACKAGE_VERSION
        gcn::Label *versionLabel = new gcn::Label(PACKAGE_VERSION);
        top->add(versionLabel, 25, 2);
#endif

        sound.playMusic("Magick - Real.ogg");

        // Server choice
        if (options.serverName.empty()) {
            loginData.hostname = config.getValue("MostUsedServerName0",
                                    defaultAccountServerName.c_str());
        }
        else {
            loginData.hostname = options.serverName;
        }
        if (options.serverPort == 0) {
            loginData.port = (short)config.getValue("MostUsedServerPort0",
                                                    defaultAccountServerPort);
        } else {
            loginData.port = options.serverPort;
        }

        loginData.username = options.playername;
        if (loginData.username.empty()) {
            if (config.getValue("remember", 0)) {
                loginData.username = config.getValue("username", "");
            }
        }
        if (!options.password.empty()) {
            loginData.password = options.password;
        }

        loginData.remember = config.getValue("remember", 0);
        loginData.registerLogin = false;

        Net::initialize();
        accountServerConnection = Net::getConnection();
        gameServerConnection = Net::getConnection();
        chatServerConnection = Net::getConnection();

        unsigned int oldstate = !state; // We start with a status change.

        SDL_Event event;
        while (state != STATE_FORCE_QUIT)
        {
            // Handle SDL events
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        state = STATE_FORCE_QUIT;
                        break;

                    case SDL_KEYDOWN:
                        if (event.key.keysym.sym == SDLK_ESCAPE)
                        {
                            if (!quitDialog)
                            {
                                quitDialog = new QuitDialog(NULL, &quitDialog);
                            }
                            else
                            {
                                quitDialog->requestMoveToTop();
                            }
                        }
                        break;
                }

                guiInput->pushInput(event);
            }

            Net::flush();
            gui->logic();

            if (!login_wallpaper)
            {
                login_wallpaper = ResourceManager::getInstance()->
                        getImage("graphics/images/login_wallpaper.png");
                if (!login_wallpaper)
                {
                    logger->error("Couldn't load login_wallpaper.png");
                }
            }

            graphics->drawImage(login_wallpaper, 0, 0);
            gui->draw();
            graphics->updateScreen();

            // TODO: Add connect timeouts
            if (state == STATE_CONNECT_ACCOUNT &&
                    accountServerConnection->isConnected())
            {
                if (options.skipUpdate) {
                    state = STATE_LOADDATA;
                } else {
                    state = STATE_UPDATE;
                }
            }
            else if (state == STATE_CONNECT_GAME &&
                    gameServerConnection->isConnected() &&
                    chatServerConnection->isConnected())
            {
                accountServerConnection->disconnect();
                Net::clearHandlers();

                state = STATE_GAME;
            }
            else if (state == STATE_RECONNECT_ACCOUNT &&
                     accountServerConnection->isConnected())
            {
                reconnectAccount(token);
                state = STATE_WAIT;
            }

            if (state != oldstate) {
                // Load updates after exiting the update state
                if (oldstate == STATE_UPDATE)
                {
                    // TODO: Revive later
                    //loadUpdates();
                    // Reload the wallpaper in case that it was updated
                    login_wallpaper->decRef();
                    login_wallpaper = ResourceManager::getInstance()->
                        getImage("graphics/images/login_wallpaper.png");
                }

                oldstate = state;

                // Get rid of the dialog of the previous state
                if (currentDialog) {
                    delete currentDialog;
                    currentDialog = NULL;
                }
                // State has changed, while the quitDialog was active, it might
                // not be correct anymore
                if (quitDialog) {
                    quitDialog->scheduleDelete();
                }

                switch (state) {
                    case STATE_CHOOSE_SERVER:
                        logger->log("State: CHOOSE_SERVER");

                        // Allow changing this using a server choice dialog
                        // We show the dialog box only if the command-line options
                        // weren't set.
                        if (options.serverName.empty() && options.serverPort == 0) {
                            currentDialog = new ServerDialog(&loginData);
                        } else {
                            state = STATE_CONNECT_ACCOUNT;

                            // Reset options so that cancelling or connect timeout
                            // will show the server dialog
                            options.serverName = "";
                            options.serverPort = 0;
                        }
                        break;

                    case STATE_CONNECT_ACCOUNT:
                        logger->log("State: CONNECT_ACCOUNT");
                        logger->log("Trying to connect to account server...");
                        accountServerConnection->connect(loginData.hostname,
                                loginData.port);
                        currentDialog = new ConnectionDialog(STATE_SWITCH_ACCOUNTSERVER_ATTEMPT);
                        break;

                    case STATE_UPDATE:
                        logger->log("State: UPDATE");
                        // TODO: Revive later
                        //currentDialog = new UpdaterWindow();
                        state = STATE_LOADDATA;
                        break;

                    case STATE_LOGIN:
                        logger->log("State: LOGIN");
                        currentDialog = new LoginDialog(&loginData);
                        // TODO: Restore autologin
                        //if (!loginData.password.empty()) {
                        //    accountLogin(&loginData);
                        //}
                        break;

                    case STATE_LOADDATA:
                        logger->log("State: LOADDATA");

                        // Add customdata directory
                        ResourceManager::getInstance()->searchAndAddArchives(
                            "customdata/",
                            "zip",
                            false);

                        // Load XML databases
                        ItemDB::load();
                        MonsterDB::load();
                        NPCDB::load();
                        state = STATE_LOGIN;
                        break;

                    case STATE_LOGIN_ATTEMPT:
                        accountLogin(&loginData);
                        break;

                    case STATE_LOGIN_ERROR:
                        logger->log("State: LOGIN ERROR");
                        currentDialog = new OkDialog("Error ", errorMessage);
                        currentDialog->addActionListener(&loginListener);
                        currentDialog = NULL; // OkDialog deletes itself
                        break;

                    case STATE_SWITCH_ACCOUNTSERVER:
                        logger->log("State: SWITCH_ACCOUNTSERVER");

                        gameServerConnection->disconnect();
                        chatServerConnection->disconnect();
                        accountServerConnection->disconnect();

                        state = STATE_CHOOSE_SERVER;
                        break;

                    case STATE_SWITCH_ACCOUNTSERVER_ATTEMPT:
                        logger->log("State: SWITCH_ACCOUNTSERVER_ATTEMPT");
                        switchAccountServer();

                        state = STATE_SWITCH_ACCOUNTSERVER;
                        break;

                    case STATE_REGISTER:
                        logger->log("State: REGISTER");
                        currentDialog = new RegisterDialog(&loginData);
                        break;

                    case STATE_REGISTER_ATTEMPT:
                        accountRegister(&loginData);
                        break;

                    case STATE_CHAR_SELECT:
                        logger->log("State: CHAR_SELECT");
                        currentDialog =
                                      new CharSelectDialog(&charInfo, &loginData);

                        if (((CharSelectDialog*) currentDialog)->
                                selectByName(options.playername))
                            options.chooseDefault = true;
                        else
                            ((CharSelectDialog*) currentDialog)->selectByName(
                                config.getValue("lastCharacter", ""));

                        if (options.chooseDefault)
                        {
                            ((CharSelectDialog*) currentDialog)->action(
                                gcn::ActionEvent(NULL, "ok"));
                            options.chooseDefault = false;
                        }

                        break;

                    case STATE_CHANGEPASSWORD_ATTEMPT:
                        logger->log("State: CHANGE PASSWORD ATTEMPT");
                        accountChangePassword(&loginData);
                        break;

                    case STATE_CHANGEPASSWORD:
                        logger->log("State: CHANGE PASSWORD");
                        currentDialog = new OkDialog("Password change",
                                            "Password changed successfully!");
                        currentDialog->addActionListener(&accountListener);
                        currentDialog = NULL; // OkDialog deletes itself
                        loginData.password = loginData.newPassword;
                        loginData.newPassword = "";
                        break;

                    case STATE_UNREGISTER_ATTEMPT:
                        logger->log("State: UNREGISTER ATTEMPT");
                        accountUnRegister(&loginData);
                        break;

                    case STATE_UNREGISTER:
                        logger->log("State: UNREGISTER");
                        accountServerConnection->disconnect();
                        currentDialog = new OkDialog("Unregister successful",
                                             "Farewell, come back any time ....");
                        loginData.clear();
                        //The errorlistener sets the state to STATE_CHOOSE_SERVER
                        currentDialog->addActionListener(&errorListener);
                        currentDialog = NULL; // OkDialog deletes itself
                        break;

                    case STATE_ACCOUNTCHANGE_ERROR:
                        logger->log("State: ACCOUNT CHANGE ERROR");
                        currentDialog = new OkDialog("Error ", errorMessage);
                        currentDialog->addActionListener(&accountListener);
                        currentDialog = NULL; // OkDialog deletes itself
                        break;


                    case STATE_ERROR:
                        logger->log("State: ERROR");
                        currentDialog = new OkDialog("Error", errorMessage);
                        currentDialog->addActionListener(&errorListener);
                        currentDialog = NULL; // OkDialog deletes itself
                        gameServerConnection->disconnect();
                        chatServerConnection->disconnect();
                        Net::clearHandlers();
                        break;

                    case STATE_CONNECT_GAME:
                        logger->log("State: CONNECT_GAME");
                        currentDialog = new ConnectionDialog(STATE_SWITCH_ACCOUNTSERVER_ATTEMPT);
                        break;

                    case STATE_GAME:
                        logger->log("Memorizing selected character %s",
                                player_node->getName().c_str());
                        config.setValue("lastCharacter", player_node->getName());

                        Net::GameServer::connect(gameServerConnection, token);
                        Net::ChatServer::connect(chatServerConnection, token);
                        sound.fadeOutMusic(1000);

#ifdef PACKAGE_VERSION
                        delete versionLabel;
                        versionLabel = NULL;
#endif
                        currentDialog = NULL;

                        logger->log("State: GAME");
                        game = new Game;
                        game->logic();
                        delete game;

                        //If the quitdialog didn't set the next state
                        if (state == STATE_GAME)
                        {
                            state = STATE_EXIT;
                        }
                        break;

                    case STATE_SWITCH_CHARACTER:
                        logger->log("State: SWITCH_CHARACTER");
                        switchCharacter(&token);
                        break;

                    case STATE_RECONNECT_ACCOUNT:
                        logger->log("State: RECONNECT_ACCOUNT");

                        //done with game&chat
                        gameServerConnection->disconnect();
                        chatServerConnection->disconnect();

                        accountServerConnection->connect(loginData.hostname,
                                                                  loginData.port);
                        break;

                    case STATE_WAIT:
                        break;

                    case STATE_EXIT:
                        logger->log("State: EXIT");
                        logoutThenExit();
                        break;

                    default:
                        state = STATE_FORCE_QUIT;
                        break;
                }
            }
        }

#ifdef PACKAGE_VERSION
        delete versionLabel;
#endif
    }
    catch (...)
    {
        logger->log("Exception");
    }

    if (accountServerConnection)
        accountServerConnection->disconnect();
    if (gameServerConnection)
        gameServerConnection->disconnect();
    if (chatServerConnection)
        chatServerConnection->disconnect();

    delete accountServerConnection;
    delete gameServerConnection;
    delete chatServerConnection;
    Net::finalize();

    logger->log("Quitting");
    exit_engine();
    PHYSFS_deinit();
    delete logger;

    return 0;
}
