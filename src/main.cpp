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
#include "gui/login.h"
#include "gui/ok_dialog.h"
#include "gui/register.h"
#include "gui/updatewindow.h"
#include "gui/textfield.h"

#include "net/charserverhandler.h"
#include "net/loginhandler.h"
#include "net/maploginhandler.h"
#include "net/messageout.h"
#include "net/network.h"
#include "net/protocol.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"
#include "resources/spriteset.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

// Account infos
char n_character;
std::string token;

std::vector<Spriteset *> hairset;
Spriteset *playerset[2];
Graphics *graphics;

unsigned char state;
std::string errorMessage;
std::string homeDir;
unsigned char screen_mode;

Sound sound;
Music *bgm;

Configuration config;         /**< Xml file configuration reader */
Logger *logger;               /**< Log object */

namespace {
    struct ErrorListener : public gcn::ActionListener
    {
        void action(const std::string &eventId, gcn::Widget *widget) {
            state = STATE_CHOOSE_SERVER;
        }
    } errorListener;
}

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
void initConfiguration()
{
    // Fill configuration with defaults
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
    config.setValue("updatehost", "http://themanaworld.org/files");
    config.setValue("customcursor", 1);
    config.setValue("homeDir", homeDir);

    // Checking if the configuration file exists... otherwise create it with
    // default options.
    FILE *tmwFile = 0;
    std::string configPath = homeDir + "/config.xml";
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
 * Do all initialization stuff
 */
void init_engine()
{
    // Initialize SDL
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

    int width = (int)config.getValue("screenwidth", 800);
    int height = (int)config.getValue("screenheight", 600);
    int bpp = 0;
    bool fullscreen = ((int)config.getValue("screen", 0) == 1);
    bool hwaccel = ((int)config.getValue("hwaccel", 0) == 1);

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

    playerset[0] = resman->getSpriteset(
            "graphics/sprites/player_male_base.png", 64, 64);
    if (!playerset[0]) logger->error("Couldn't load male player spriteset!");
    playerset[1] = resman->getSpriteset(
            "graphics/sprites/player_female_base.png", 64, 64);
    if (!playerset[1]) logger->error("Couldn't load female player spriteset!");


    for (int i = 0; i < NR_HAIR_STYLES; i++)
    {
        Spriteset *tmp = ResourceManager::getInstance()->getSpriteset(
                "graphics/sprites/hairstyle" + toString(i + 1) + ".png",
                40, 40);
        if (!tmp) {
            logger->error("Unable to load hairstyle");
        } else {
            hairset.push_back(tmp);
        }
    }

    gui = new Gui(graphics);
    state = STATE_CHOOSE_SERVER; /**< Initial game state */

    // Initialize sound engine
    try {
        if (config.getValue("sound", 0) == 1) {
            sound.init();
        }
        sound.setSfxVolume((int)config.getValue("sfxVolume", 100));
        sound.setMusicVolume((int)config.getValue("musicVolume", 60));
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

    std::for_each(hairset.begin(), hairset.end(),
            std::mem_fun(&Spriteset::decRef));
    hairset.clear();

    playerset[0]->decRef();
    playerset[1]->decRef();

    // Shutdown libxml
    xmlCleanupParser();

    // Shutdown sound
    sound.close();

    ResourceManager::deleteInstance();
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
        chooseDefault(false)
    {};

    bool printHelp;
    bool skipUpdate;
    bool chooseDefault;
    std::string username;
    std::string password;
};

void printHelp()
{
    std::cout
        << "tmw" << std::endl << std::endl
        << "Options: " << std::endl
        << "  -h --help       : Display this help" << std::endl
        << "  -u --skipupdate : Skip the update process" << std::endl
        << "  -U --username   : Login with this username" << std::endl
        << "  -P --password   : Login with this password" << std::endl
        << "  -D --default    : Bypass the login process with default settings"
        << std::endl;
}

void parseOptions(int argc, char *argv[], Options &options)
{
    const char *optstring = "huU:P:D";

    const struct option long_options[] = {
        { "help",       no_argument,       0, 'h' },
        { "skipupdate", no_argument,       0, 'u' },
        { "username",   required_argument, 0, 'U' },
        { "password",   required_argument, 0, 'P' },
        { "default",    no_argument,       0, 'D' },
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
                options.username = optarg;
                break;
            case 'P':
                options.password = optarg;
                break;
            case 'D':
                options.chooseDefault = true;
                break;
        }
    }
}

/**
 * Reads the file "updates/resources.txt" and attempts to load each update
 * mentioned in it.
 */
void loadUpdates()
{
    const std::string updatesFile = "updates/resources.txt";
    ResourceManager *resman = ResourceManager::getInstance();
    std::vector<std::string> lines = resman->loadTextFile(updatesFile);
    std::string homeDir = config.getValue("homeDir", "");

    for (unsigned int i = 0; i < lines.size(); ++i)
    {
        resman->addToSearchPath(homeDir + "/updates/" + lines[i], false);
    }
}

CharServerHandler charServerHandler;
LoginData loginData;
LoginHandler loginHandler;
LockedArray<LocalPlayer*> charInfo(MAX_SLOT + 1);
MapLoginHandler mapLoginHandler;

// TODO Find some nice place for these functions
void accountLogin(LoginData *loginData)
{
    logger->log("Username is %s", loginData->username.c_str());
    network->registerHandler(&loginHandler);
    network->registerHandler(&charServerHandler);
    loginHandler.setLoginData(loginData);
    charServerHandler.setLoginData(loginData);
    charServerHandler.setCharInfo(&charInfo);

    // Send login infos
    MessageOut msg(PAMSG_LOGIN);
    msg.writeLong(0); // client version
    msg.writeString(loginData->username);
    msg.writeString(loginData->password);
    network->send(Network::ACCOUNT, msg);

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
    network->registerHandler(&loginHandler);
    loginHandler.setLoginData(loginData);
    charServerHandler.setLoginData(loginData);
    charServerHandler.setCharInfo(&charInfo);

    // Send login infos
    MessageOut msg(PAMSG_REGISTER);
    msg.writeLong(0); // client version
    msg.writeString(loginData->username);
    msg.writeString(loginData->password);
    msg.writeString(loginData->email);
    network->send(Network::ACCOUNT, msg);
}

void mapLogin(Network *network, LoginData *loginData)
{
    network->registerHandler(&mapLoginHandler);

    // Send connect messages with the magic token to game and chat servers
    MessageOut gameServerConnect(PGMSG_CONNECT);
    gameServerConnect.writeString(token, 32);
    network->send(Network::GAME, gameServerConnect);

    MessageOut chatServerConnect(PCMSG_CONNECT);
    chatServerConnect.writeString(token, 32);
    network->send(Network::CHAT, chatServerConnect);
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
    initConfiguration();

    // Configure logger
    logger = new Logger();
    logger->setLogFile(homeDir + std::string("/tmw.log"));
    logger->setLogToStandardOut(config.getValue("logToStandardOut", 0));

    // Initialize libxml2 and check for potential ABI mismatches between
    // compiled version and the shared library actually used.
    logger->log("Initializing libxml2...");
    xmlInitParser();
    LIBXML_TEST_VERSION;

    // Redirect libxml errors to /dev/null
    FILE *nullFile = fopen("/dev/null", "w");
    xmlSetGenericErrorFunc(nullFile, NULL);

    init_engine();

    unsigned int oldstate = !state; // We start with a status change.

    Window *currentDialog = NULL;
    Image *login_wallpaper = NULL;
    Game *game = NULL;

    sound.playMusic(TMW_DATADIR "data/music/Magick - Real.ogg");

    loginData.username = options.username;
    if (loginData.username.empty()) {
        if (config.getValue("remember", 0)) {
            loginData.username = config.getValue("username", "");
        }
    }
    if (!options.password.empty()) {
        loginData.password = options.password;
    }
    loginData.hostname = config.getValue("host", "animesites.de");
    loginData.port = (short)config.getValue("port", 0);
    loginData.remember = config.getValue("remember", 0);

    if (enet_initialize() != 0)
    {
        logger->error("An error occurred while initializing ENet.");
    }
    network = new Network();


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
        network->flush();

        if (network->getState() == Network::NET_ERROR)
        {
            state = STATE_ERROR;
            errorMessage = "Got disconnected from server!";
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
                network->isConnected(Network::ACCOUNT))
        {
            if (options.skipUpdate) {
                state = STATE_LOGIN;
            } else {
                state = STATE_UPDATE;
            }
        }
        else if (state == STATE_CONNECT_GAME &&
                network->isConnected(Network::GAME) &&
                network->isConnected(Network::CHAT))
        {
            // TODO: Somehow send the token
            state = STATE_GAME;
        }

        if (state != oldstate) {
            // Load updates after exiting the update state
            if (oldstate == STATE_UPDATE)
            {
                loadUpdates();
            }

            // Disconnect from account server once connected to game server
            if (oldstate == STATE_CONNECT_GAME && state == STATE_GAME)
            {
                network->disconnect(Network::ACCOUNT);
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
                    // TODO: Allow changing this using a server choice dialog
                    logger->log("Trying to connect to account server...");
                    network->connect(Network::ACCOUNT,
                                     loginData.hostname, loginData.port);
                    state = STATE_CONNECT_ACCOUNT;
                    break;

                case STATE_CONNECT_ACCOUNT:
                    logger->log("State: CONNECT_ACCOUNT");
                    break;

                case STATE_UPDATE:
                    logger->log("State: UPDATE");
                    // TODO: Revive later
                    //currentDialog = new UpdaterWindow();
                    state = STATE_LOGIN;
                    break;

                case STATE_LOGIN:
                    logger->log("State: LOGIN");
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
                    currentDialog = new CharSelectDialog(network, &charInfo);
                    if (options.chooseDefault) {
                        ((CharSelectDialog*)currentDialog)->action("ok",
                                                                   NULL);
                    }
                    break;

                case STATE_ERROR:
                    logger->log("State: ERROR");
                    currentDialog = new OkDialog("Error", errorMessage);
                    currentDialog->addActionListener(&errorListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    network->disconnect(Network::GAME);
                    network->disconnect(Network::CHAT);
                    network->clearHandlers();
                    break;

                case STATE_CONNECT_GAME:
                    logger->log("State: CONNECT_GAME");
                    currentDialog = new ConnectionDialog();
                    break;

                case STATE_GAME:
                    mapLogin(network, &loginData);
                    sound.fadeOutMusic(1000);

                    currentDialog = NULL;
                    login_wallpaper->decRef();
                    login_wallpaper = NULL;

                    logger->log("State: GAME");
                    game = new Game(network);
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

    delete network;
    enet_deinitialize();

    if (nullFile)
    {
        fclose(nullFile);
    }
    logger->log("State: EXIT");
    exit_engine();
    PHYSFS_deinit();
    delete logger;
    return 0;
}
