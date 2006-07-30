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

#include "animation.h"
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
        void action(const std::string &eventId, gcn::Widget *widget) { state = LOGIN_STATE; }
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

    int width, height, bpp;
    bool fullscreen, hwaccel;

    width = (int)config.getValue("screenwidth", 800);
    height = (int)config.getValue("screenheight", 600);
    bpp = 0;
    fullscreen = ((int)config.getValue("screen", 0) == 1);
    hwaccel = ((int)config.getValue("hwaccel", 0) == 1);

#ifdef USE_OPENGL
    bool useOpenGL = (config.getValue("opengl", 0) == 1);

    // Setup image loading for the right image format
    Image::setLoadAsOpenGL(useOpenGL);

    // Create the graphics context
    if (useOpenGL) {
        graphics = new OpenGLGraphics();
    } else {
        graphics = new Graphics();
    }
#else
    // Create the graphics context
    graphics = new Graphics();
#endif

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
    state = UPDATE_STATE; /**< Initial game state */

    // Initialize sound engine
    try {
        if (config.getValue("sound", 0) == 1) {
            sound.init();
        }
        sound.setSfxVolume((int)config.getValue("sfxVolume", 100));
        sound.setMusicVolume((int)config.getValue("musicVolume", 60));
    }
    catch (const char *err) {
        state = ERROR_STATE;
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

    std::vector<Spriteset *>::iterator iter;
    for (iter = hairset.begin(); iter != hairset.end(); ++iter)
    {
        (*iter)->decRef();
    }
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
void accountLogin(Network *network, LoginData *loginData)
{
    logger->log("Trying to connect to account server...");
    logger->log("Username is %s", loginData->username.c_str());
    network->connect(loginData->hostname, loginData->port);
    network->registerHandler(&loginHandler);
    loginHandler.setCharInfo(&charInfo);
    loginHandler.setLoginData(loginData);

    // Send login infos
    MessageOut *msg = new MessageOut();
    msg->writeShort(PAMSG_LOGIN);
    msg->writeLong(0); // client version
    msg->writeString(loginData->username);
    msg->writeString(loginData->password);
    network->send(msg);

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

void mapLogin(Network *network, LoginData *loginData)
{
    MessageOut outMsg;

    logger->log("Trying to connect to map server...");
    logger->log("Map: %s", map_path.c_str());

    network->connect(loginData->hostname, loginData->port);
    network->registerHandler(&mapLoginHandler);

    // Send login infos
    outMsg.writeShort(0x0072);
    outMsg.writeLong(loginData->account_ID);
    outMsg.writeLong(player_node->mCharId);
    outMsg.writeLong(loginData->session_ID1);
    outMsg.writeLong(loginData->session_ID2);
    outMsg.writeByte(loginData->sex);
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

    if (options.skipUpdate && state != ERROR_STATE) {
        state = LOGIN_STATE;
    }
    else {
        state = UPDATE_STATE;
    }

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
    Network *network = new Network();

    SDL_Event event;

    while (state != EXIT_STATE)
    {
        // Handle SDL events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state = EXIT_STATE;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        state = EXIT_STATE;
                    break;
            }

            guiInput->pushInput(event);
        }

        gui->logic();
        network->flush();

        if (network->getState() == Network::NET_ERROR)
        {
            state = ERROR_STATE;
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

        if (state != oldstate) {
            switch (oldstate)
            {
                case UPDATE_STATE:
                    loadUpdates();
                    break;

                    // Those states don't cause a network disconnect
                case ACCOUNT_STATE:
                case CONNECTING_STATE:
                    break;

                default:
                    network->disconnect();
                    network->clearHandlers();
                    break;
            }

            oldstate = state;

            if (currentDialog && state != ACCOUNT_STATE &&
                    state != CHAR_CONNECT_STATE) {
                delete currentDialog;
                currentDialog = NULL;
            }

            switch (state) {
                case LOGIN_STATE:
                    logger->log("State: LOGIN");
                    if (!loginData.password.empty()) {
                        state = ACCOUNT_STATE;
                    } else {
                        currentDialog = new LoginDialog(&loginData);
                    }
                    break;

                case REGISTER_STATE:
                    logger->log("State: REGISTER");
                    currentDialog = new RegisterDialog(&loginData);
                    break;

                case CHAR_SELECT_STATE:
                    logger->log("State: CHAR_SELECT");
                    currentDialog = new CharSelectDialog(network, &charInfo,
                                                         1 - loginData.sex);
                    if (options.chooseDefault) {
                        ((CharSelectDialog*)currentDialog)->action("ok",
                                                                   NULL);
                    }
                    break;

                case GAME_STATE:
                    sound.fadeOutMusic(1000);

                    currentDialog = NULL;
                    login_wallpaper->decRef();
                    login_wallpaper = NULL;

                    logger->log("State: GAME");
                    game = new Game(network);
                    game->logic();
                    delete game;
                    state = EXIT_STATE;
                    break;

                case UPDATE_STATE:
                    logger->log("State: UPDATE");
                    currentDialog = new UpdaterWindow();
                    break;

                case ERROR_STATE:
                    logger->log("State: ERROR");
                    currentDialog = new OkDialog("Error", errorMessage);
                    currentDialog->addActionListener(&errorListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    network->disconnect();
                    network->clearHandlers();
                    break;

                case CONNECTING_STATE:
                    logger->log("State: CONNECTING");
                    mapLogin(network, &loginData);
                    currentDialog = new ConnectionDialog();
                    break;

                case ACCOUNT_STATE:
                    printf("Account: %i\n", loginData.sex);
                    accountLogin(network, &loginData);
                    break;

                default:
                    state = EXIT_STATE;
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
