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
#include <guichan/widgets/label.hpp>

#include <libxml/parser.h>

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

#include "gui/char_server.h"
#include "gui/char_select.h"
#include "gui/gui.h"
#include "gui/login.h"
#include "gui/ok_dialog.h"
#include "gui/progressbar.h"
#include "gui/register.h"
#include "gui/updatewindow.h"
#include "gui/textfield.h"

#include "net/charserverhandler.h"
#include "net/loginhandler.h"
#include "net/maploginhandler.h"
#include "net/messageout.h"
#include "net/network.h"

#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/monsterdb.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

#ifdef WIN32
#include <SDL_syswm.h>
#endif

std::string homeDir;

// Account infos
char n_server, n_character;

Graphics *graphics;

// TODO Anyone knows a good location for this? Or a way to make it non-global?
class SERVER_INFO;
SERVER_INFO **server_info;

unsigned char state;
std::string errorMessage;
unsigned char screen_mode;

Sound sound;
Music *bgm;

Configuration config;         /**< XML file configuration reader */
Logger *logger;               /**< Log object */
KeyboardConfig keyboard;

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
        chooseDefault(false)
    {};

    bool printHelp;
    bool printVersion;
    bool skipUpdate;
    bool chooseDefault;
    std::string username;
    std::string password;
    std::string playername;
    std::string configPath;
};

/**
 * Do all initialization stuff
 */
void init_engine(const Options &options)
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

    // Set log file
    logger->setLogFile(homeDir + std::string("/tmw.log"));

    #ifdef PACKAGE_VERSION
        logger->log("Starting The Mana World Version %s", PACKAGE_VERSION);
    #else
        logger->log("Starting The Mana World - Version not defined");
    #endif

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

    ResourceManager *resman = ResourceManager::getInstance();

    if (!resman->setWriteDir(homeDir)) {
        std::cout << homeDir
                  << " couldn't be set as home directory! Exiting."
                  << std::endl;
        exit(1);
    }

    // Add the user's homedir to PhysicsFS search path
    resman->addToSearchPath(homeDir, false);
    // Creating and checking the updates folder existence and rights.
    if (!resman->isDirectory("/updates")) {
        if (!resman->mkdir("/updates")) {
            std::cout << homeDir << "/updates "
                      << "can't be made, but it doesn't exist! Exiting."
                      << std::endl;
            exit(1);
        }
    }

    // Add the main data directory to our PhysicsFS search path
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

    // Fill configuration with defaults
    logger->log("Initializing configuration...");
    config.setValue("host", "server.themanaworld.org");
    config.setValue("port", 6901);
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
    config.setValue("fpslimit", 60);
    config.setValue("updatehost", "http://updates.themanaworld.org");
    config.setValue("customcursor", 1);

    // Checking if the configuration file exists... otherwise creates it with
    // default options !
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
            "Using Defaults." << std::endl;
    } else {
        fclose(tmwFile);
        config.init(configPath);
    }

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
        std::cerr << "Couldn't set " << width << "x" << height << "x" <<
            bpp << " video mode: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Initialize for drawing
    graphics->_beginDraw();

    // Initialize the item shortcuts.
    itemShortcut = new ItemShortcut();

    gui = new Gui(graphics);
    state = UPDATE_STATE; /**< Initial game state */

    // Initialize sound engine
    try {
        if (config.getValue("sound", 0) == 1) {
            sound.init();
        }
        sound.setSfxVolume((int) config.getValue("sfxVolume", 100));
        sound.setMusicVolume((int) config.getValue("musicVolume", 60));
    }
    catch (const char *err) {
        state = ERROR_STATE;
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

    ResourceManager::deleteInstance();
    delete logger;
}

void printHelp()
{
    std::cout
        << "tmw" << std::endl << std::endl
        << "Options: " << std::endl
        << "  -h --help       : Display this help" << std::endl
        << "  -v --version    : Display the version" << std::endl
        << "  -u --skipupdate : Skip the update process" << std::endl
        << "  -U --username   : Login with this username" << std::endl
        << "  -P --password   : Login with this password" << std::endl
        << "  -D --default    : Bypass the login process with default settings" << std::endl
        << "  -p --playername : Login with this player" << std::endl
        << "  -C --configfile : Configuration file to use"
        << std::endl;
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
    const char *optstring = "hvuU:P:Dp:C:";

    const struct option long_options[] = {
        { "help",       no_argument,       0, 'h' },
        { "version",    no_argument,       0, 'v' },
        { "skipupdate", no_argument,       0, 'u' },
        { "username",   required_argument, 0, 'U' },
        { "password",   required_argument, 0, 'P' },
        { "default",    no_argument,       0, 'D' },
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
            case 'v':
                options.printVersion = true;
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
LockedArray<LocalPlayer*> charInfo(MAX_SLOT + 1);
MapLoginHandler mapLoginHandler;

namespace {
    struct ErrorListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            state = loginData.registerLogin ? REGISTER_STATE : LOGIN_STATE;
        }
    } errorListener;
}

// TODO Find some nice place for these functions
void accountLogin(Network *network, LoginData *loginData)
{
    logger->log("Trying to connect to account server...");
    logger->log("Username is %s", loginData->username.c_str());
    network->connect(loginData->hostname, loginData->port);
    network->registerHandler(&loginHandler);
    loginHandler.setLoginData(loginData);

    // Send login infos
    MessageOut outMsg(network);
    outMsg.writeInt16(0x0064);
    outMsg.writeInt32(0); // client version
    outMsg.writeString(loginData->username, 24);
    outMsg.writeString(loginData->password, 24);
    outMsg.writeInt8(0); // unknown

    // Clear the password, avoids auto login when returning to login
    loginData->password = "";

    // Remove _M or _F from username after a login for registration purpose
    if (loginData->registerLogin)
    {
        loginData->username =
            loginData->username.substr(0, loginData->username.length() - 2);
    }

    // TODO This is not the best place to save the config, but at least better
    // than the login gui window
    if (loginData->remember)
    {
        config.setValue("host", loginData->hostname);
        config.setValue("username", loginData->username);
    }
    config.setValue("remember", loginData->remember);
}

void charLogin(Network *network, LoginData *loginData)
{
    logger->log("Trying to connect to char server...");
    network->connect(loginData->hostname, loginData->port);
    network->registerHandler(&charServerHandler);
    charServerHandler.setCharInfo(&charInfo);
    charServerHandler.setLoginData(loginData);

    // Send login infos
    MessageOut outMsg(network);
    outMsg.writeInt16(0x0065);
    outMsg.writeInt32(loginData->account_ID);
    outMsg.writeInt32(loginData->session_ID1);
    outMsg.writeInt32(loginData->session_ID2);
    outMsg.writeInt16(0); // unknown
    outMsg.writeInt8(loginData->sex);

    // We get 4 useless bytes before the real answer comes in
    network->skip(4);
}

void mapLogin(Network *network, LoginData *loginData)
{
    logger->log("Memorizing selected character %s",
            player_node->getName().c_str());
    config.setValue("lastCharacter", player_node->getName());

    MessageOut outMsg(network);

    logger->log("Trying to connect to map server...");
    logger->log("Map: %s", map_path.c_str());

    network->connect(loginData->hostname, loginData->port);
    network->registerHandler(&mapLoginHandler);

    // Send login infos
    outMsg.writeInt16(0x0072);
    outMsg.writeInt32(loginData->account_ID);
    outMsg.writeInt32(player_node->mCharId);
    outMsg.writeInt32(loginData->session_ID1);
    outMsg.writeInt32(loginData->session_ID2);
    outMsg.writeInt8(loginData->sex);

    // We get 4 useless bytes before the real answer comes in
    network->skip(4);
}

/** Main */
int main(int argc, char *argv[])
{
    logger = new Logger();

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
    // Initialize libxml2 and check for potential ABI mismatches between
    // compiled version and the shared library actually used.
    xmlInitParser();
    LIBXML_TEST_VERSION;

    // Redirect libxml errors to /dev/null
    FILE *nullFile = fopen("/dev/null", "w");
    xmlSetGenericErrorFunc(nullFile, NULL);

    // Initialize PhysicsFS
    PHYSFS_init(argv[0]);

    init_engine(options);

    SDL_Event event;

    if (options.skipUpdate && state != ERROR_STATE) {
        state = LOADDATA_STATE;
    }
    else {
        state = UPDATE_STATE;
    }

    unsigned int oldstate = !state; // We start with a status change.

    Game *game = NULL;
    Window *currentDialog = NULL;
    Image *login_wallpaper = NULL;

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());
#ifdef PACKAGE_VERSION
    gcn::Label *versionLabel = new gcn::Label(PACKAGE_VERSION);
    top->add(versionLabel, 2, 2);
#endif
    ProgressBar *progressBar = new ProgressBar(0.0f, 100, 20);
    gcn::Label *progressLabel = new gcn::Label();
    top->add(progressBar, 5, top->getHeight() - 5 - progressBar->getHeight());
    top->add(progressLabel, 15 + progressBar->getWidth(),
                            progressBar->getY() + 4);
    progressBar->setVisible(false);

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
    loginData.hostname = config.getValue("host", "server.themanaworld.org");
    loginData.port = (short)config.getValue("port", 0);
    loginData.remember = config.getValue("remember", 0);
    loginData.registerLogin = false;

    SDLNet_Init();
    Network *network = new Network();
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

        network->flush();
        network->dispatchMessages();
        gui->logic();

        if (network->getState() == Network::NET_ERROR)
        {
            state = ERROR_STATE;

            if (!network->getError().empty()) {
                errorMessage = network->getError();
            } else {
                errorMessage = "Got disconnected from server!";
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

        if (progressBar->isVisible())
        {
            progressBar->setProgress(progressBar->getProgress() + 0.005f);
            if (progressBar->getProgress() == 1.0f)
                progressBar->setProgress(0.0f);
        }

        graphics->drawImage(login_wallpaper, 0, 0);
        gui->draw();
        graphics->updateScreen();

        if (state != oldstate) {
            switch (oldstate)
            {
                case UPDATE_STATE:
                    loadUpdates();
                    // Reload the wallpaper in case that it was updated
                    login_wallpaper->decRef();
                    login_wallpaper = ResourceManager::getInstance()->
                        getImage("graphics/images/login_wallpaper.png");
                    break;

                    // Those states don't cause a network disconnect
                case LOADDATA_STATE:
                    break;

                case ACCOUNT_STATE:
                case CHAR_CONNECT_STATE:
                case CONNECTING_STATE:
                    progressBar->setVisible(false);
                    progressLabel->setCaption("");
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
                case LOADDATA_STATE:
                    logger->log("State: LOADDATA");

                    // Add customdata directory
                    ResourceManager::getInstance()->searchAndAddArchives(
                        "customdata/",
                        "zip",
                        false);

                    // Load XML databases
                    ItemDB::load();
                    MonsterDB::load();
                    state = LOGIN_STATE;
                    break;

                case LOGIN_STATE:
                    logger->log("State: LOGIN");

                    if (!loginData.password.empty()) {
                        loginData.registerLogin = false;
                        state = ACCOUNT_STATE;
                    } else {
                        currentDialog = new LoginDialog(&loginData);
                    }
                    break;

                case REGISTER_STATE:
                    logger->log("State: REGISTER");
                    currentDialog = new RegisterDialog(&loginData);
                    break;

                case CHAR_SERVER_STATE:
                    logger->log("State: CHAR_SERVER");
                    currentDialog = new ServerSelectDialog(&loginData);
                    if (options.chooseDefault || options.playername != "") {
                        ((ServerSelectDialog*) currentDialog)->action(
                            gcn::ActionEvent(NULL, "ok"));
                    }
                    break;

                case CHAR_SELECT_STATE:
                    logger->log("State: CHAR_SELECT");
                    currentDialog = new CharSelectDialog(network, &charInfo,
                                                         1 - loginData.sex);

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

                case GAME_STATE:
                    sound.fadeOutMusic(1000);

#ifdef PACKAGE_VERSION
                    delete versionLabel;
                    versionLabel = NULL;
#endif
                    delete progressBar;
                    delete progressLabel;
                    progressBar = NULL;
                    progressLabel = NULL;
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
                    progressBar->setVisible(true);
                    progressLabel->setCaption("Connecting to map server...");
                    progressLabel->adjustSize();
                    mapLogin(network, &loginData);
                    break;

                case CHAR_CONNECT_STATE:
                    progressBar->setVisible(true);
                    progressLabel->setCaption(
                            "Connecting to character server...");
                    progressLabel->adjustSize();
                    charLogin(network, &loginData);
                    break;

                case ACCOUNT_STATE:
                    progressBar->setVisible(true);
                    progressLabel->setCaption(
                            "Connecting to account server...");
                    progressLabel->adjustSize();
                    accountLogin(network, &loginData);
                    break;

                default:
                    state = EXIT_STATE;
                    break;
            }
        }
    }

    delete network;
    SDLNet_Quit();

    if (nullFile)
    {
        fclose(nullFile);
    }
    logger->log("State: EXIT");
    exit_engine();
    PHYSFS_deinit();
    return 0;
}
