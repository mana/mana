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

#include <guichan/sdl/sdlinput.hpp>

#include <libxml/parser.h>

#if (defined __USE_UNIX98 || defined __FreeBSD__)
#include <cerrno>
#include <sys/stat.h>
#elif defined WIN32
#define NOGDI
#endif

#include "configuration.h"
#include "game.h"
#include "graphics.h"
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
#include "gui/serverdialog.h"
#include "gui/login.h"
#include "gui/ok_dialog.h"
#include "gui/register.h"
#include "gui/updatewindow.h"
#include "gui/textfield.h"

#include "net/charserverhandler.h"
#include "net/connection.h"
#include "net/loginhandler.h"
#include "net/network.h"

#include "net/accountserver/accountserver.h"

#include "net/chatserver/chatserver.h"

#include "net/gameserver/gameserver.h"

#include "resources/equipmentdb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/monsterdb.h"
#include "resources/resourcemanager.h"
#include "resources/spriteset.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

// Account infos
char n_character;
std::string token;

Graphics *graphics;

unsigned char state;
std::string errorMessage;
std::string homeDir;
unsigned char screen_mode;

Sound sound;
Music *bgm;

Configuration config;         /**< Xml file configuration reader */
Logger *logger;               /**< Log object */

Net::Connection *accountServerConnection = 0;
Net::Connection *gameServerConnection = 0;
Net::Connection *chatServerConnection = 0;

namespace {
    struct ErrorListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            state = STATE_CHOOSE_SERVER;
        }
    } errorListener;
}

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
        skipUpdate(false),
        chooseDefault(false),
        serverPort(0)
    {};

    bool printHelp;
    bool skipUpdate;
    bool chooseDefault;
    std::string playername;
    std::string password;
    std::string configPath;

    std::string serverName;
    short serverPort;
};

/**
 * Initializes the home directory. On UNIX and FreeBSD, ~/.tmw is used. On
 * Windows and other systems we use the current working directory.
 */
void initHomeDir()
{
#if !(defined __USE_UNIX98 || defined __FreeBSD__)
    homeDir = ".";
#else
    homeDir = std::string(PHYSFS_getUserDir()) + "/.tmw";

    // Checking if /home/user/.tmw folder exists.
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
    {
        std::cout << homeDir
                  << " can't be made, but it doesn't exist! Exitting."
                  << std::endl;
        exit(1);
    }
#endif
}

/**
 * Initialize configuration.
 */
void initConfiguration(const Options &options)
{
    // Fill configuration with defaults
    logger->log("Initializing configuration...");
    config.setValue("host", "animesites.de");
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
    config.setValue("homeDir", homeDir);

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
void initEngine()
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
    SDL_WM_SetIcon(IMG_Load(TMW_DATADIR "data/icons/tmw-icon.png"), NULL);

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

    // Add the main data directory to our PhysicsFS search path
    resman->addToSearchPath("data", true);
    resman->addToSearchPath(TMW_DATADIR "data", true);

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
}

/** Clear the engine */
void exit_engine()
{
    config.write();
    delete gui;
    delete graphics;

    // Shutdown libxml
    xmlCleanupParser();

    // Shutdown sound
    sound.close();

    // Unload XML databases
    EquipmentDB::unload();
    ItemDB::unload();
    MonsterDB::unload();

    ResourceManager::deleteInstance();
}

void printHelp()
{
    std::cout <<
        "tmw\n\n"
        "Options:\n"
        "  -h --help       : Display this help\n"
        "  -u --skipupdate : Skip the update process\n"
        "  -U --username   : Login with this username\n"
        "  -P --password   : Login with this password\n"
        "  -D --default    : Bypass the login process with default settings\n"
        "  -s --server     : Login Server name or IP\n"
        "  -o --port       : Login Server Port\n"
        "  -p --playername : Login with this player\n"
        "  -C --configfile : Configuration file to use\n";
}

void parseOptions(int argc, char *argv[], Options &options)
{
    const char *optstring = "huU:P:Dp:s:o:C:";

    const struct option long_options[] = {
        { "help",       no_argument,       0, 'h' },
        { "skipupdate", no_argument,       0, 'u' },
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

        if (result == -1) {
            break;
        }

        switch (result) {
            default: // Unknown option
            case 'h':
                options.printHelp = true;
                break;
            case 'u':
                options.skipUpdate = true;
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
    std::string homeDir = config.getValue("homeDir", "");

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
LockedArray<LocalPlayer*> charInfo(MAX_SLOT + 1);

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
    if (loginData->remember) {
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

/** Main */
int main(int argc, char *argv[])
{
#ifdef PACKAGE_VERSION
    std::cout << "The Mana World v" << PACKAGE_VERSION << std::endl;
#endif

    // Parse command line options
    Options options;
    parseOptions(argc, argv, options);
    if (options.printHelp)
    {
        printHelp();
        return 0;
    }

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
#endif

    initXML();
    initConfiguration(options);
    initEngine();

    Window *currentDialog = NULL;
    Image *login_wallpaper = NULL;
    Game *game = NULL;

    sound.playMusic(TMW_DATADIR "data/music/Magick - Real.ogg");

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

    Net::initialize();
    accountServerConnection = Net::getConnection();
    gameServerConnection = Net::getConnection();
    chatServerConnection = Net::getConnection();

    unsigned int oldstate = !state; // We start with a status change.

    SDL_Event event;
    while (state != STATE_EXIT)
    {
        // Handle SDL events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state = STATE_EXIT;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        state = STATE_EXIT;
                    break;
            }

            guiInput->pushInput(event);
        }

        gui->logic();
        Net::flush();

        if (state > STATE_CONNECT_ACCOUNT && state < STATE_GAME)
        {
            if (!accountServerConnection->isConnected())
            {
                state = STATE_ERROR;
                errorMessage = "Got disconnected from account server!";
            }
        }

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
#ifdef PACKAGE_VERSION
        graphics->setFont(gui->getFont());
        graphics->drawText(PACKAGE_VERSION, 0, 0);
#endif
        gui->draw();
        graphics->updateScreen();

        // TODO: Add connect timeout to go back to choose server
        if (state == STATE_CONNECT_ACCOUNT &&
                accountServerConnection->isConnected())
        {
            if (options.skipUpdate) {
                state = STATE_LOGIN;
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

        if (state != oldstate) {
            // Load updates after exiting the update state
            if (oldstate == STATE_UPDATE)
            {
                loadUpdates();
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
                    currentDialog = new ConnectionDialog(STATE_CHOOSE_SERVER);
                    break;

                case STATE_UPDATE:
                    logger->log("State: UPDATE");
                    // TODO: Revive later
                    //currentDialog = new UpdaterWindow();
                    state = STATE_LOGIN;
                    break;

                case STATE_LOGIN:
                    logger->log("State: LOGIN");

                    // Load XML databases
                    EquipmentDB::load();
                    ItemDB::load();
                    MonsterDB::load();

                    currentDialog = new LoginDialog(&loginData);
                    // TODO: Restore autologin
                    //if (!loginData.password.empty()) {
                    //    accountLogin(&loginData);
                    //}
                    break;

                case STATE_LOGIN_ATTEMPT:
                    accountLogin(&loginData);
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
                    currentDialog = new CharSelectDialog(&charInfo);

                    if (((CharSelectDialog*) currentDialog)->
                            selectByName(options.playername))
                        options.chooseDefault = true;
                    else
                        ((CharSelectDialog*) currentDialog)->selectByName(
                            config.getValue("lastCharacter", ""));

                    if (options.chooseDefault)
                        ((CharSelectDialog*) currentDialog)->action(
                            gcn::ActionEvent(NULL, "ok"));
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
                    currentDialog = new ConnectionDialog(STATE_CHAR_SELECT);
                    break;

                case STATE_GAME:
                    logger->log("Memorizing selected character %s",
                            player_node->getName().c_str());
                    config.setValue("lastCharacter", player_node->getName());

                    Net::GameServer::connect(gameServerConnection, token);
                    Net::ChatServer::connect(chatServerConnection, token);
                    sound.fadeOutMusic(1000);

                    currentDialog = NULL;
                    login_wallpaper->decRef();
                    login_wallpaper = NULL;

                    logger->log("State: GAME");
                    game = new Game;
                    game->logic();
                    delete game;
                    state = STATE_EXIT;
                    break;

                default:
                    state = STATE_EXIT;
                    break;
            }
        }
    }

    delete accountServerConnection;
    delete gameServerConnection;
    delete chatServerConnection;
    Net::finalize();

    logger->log("State: EXIT");
    exit_engine();
    PHYSFS_deinit();
    delete logger;
    return 0;
}
