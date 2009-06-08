/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "main.h"

#include "configuration.h"
#include "emoteshortcut.h"
#include "game.h"
#include "graphics.h"
#include "itemshortcut.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "lockedarray.h"
#include "log.h"
#ifdef USE_OPENGL
#include "openglgraphics.h"
#endif
#include "playerrelations.h"
#include "sound.h"
#include "statuseffect.h"
#include "units.h"

#include "gui/widgets/button.h"
#include "gui/widgets/desktop.h"
#include "gui/widgets/label.h"
#include "gui/widgets/progressbar.h"

#include "gui/charselectdialog.h"
#include "gui/gui.h"
#include "gui/login.h"
#include "gui/okdialog.h"
#include "gui/palette.h"
#include "gui/register.h"
#include "gui/sdlinput.h"
#include "gui/serverselectdialog.h"
#include "gui/setup.h"
#ifdef TMWSERV_SUPPORT
#include "gui/connection.h"
#include "gui/quitdialog.h"
#include "gui/serverdialog.h"
#endif
#include "gui/updatewindow.h"

#include "net/charhandler.h"
#include "net/generalhandler.h"
#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/maphandler.h"
#include "net/net.h"
#include "net/serverinfo.h"
#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/charserverhandler.h"
#include "net/tmwserv/connection.h"
#include "net/tmwserv/generalhandler.h"
#include "net/tmwserv/loginhandler.h"
#include "net/tmwserv/logouthandler.h"
#include "net/tmwserv/network.h"
#else
#include "net/ea/generalhandler.h"
#include "net/ea/network.h"
#endif

#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/accountserver/accountserver.h"
#include "net/tmwserv/accountserver/account.h"

#include "net/tmwserv/chatserver/chatserver.h"

#include "net/tmwserv/gameserver/gameserver.h"
#endif

#include "resources/colordb.h"
#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/monsterdb.h"
#include "resources/npcdb.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <SDL_image.h>

#include <guichan/actionlistener.hpp>

#include <libxml/parser.h>

#include <getopt.h>
#include <iostream>
#include <physfs.h>
#include <unistd.h>
#include <vector>

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>
#endif

#ifdef __MINGW32__
#include <windows.h>
#define usleep(usec) (Sleep ((usec) / 1000), 0)
#endif

#ifdef WIN32
#include <SDL_syswm.h>
#else
#include <cerrno>
#include <sys/stat.h>
#endif

#ifdef TWMSERV_SUPPORT
#define DEFAULT_PORT 9601
#else
#define DEFAULT_PORT 6901
#endif

namespace
{
    struct SetupListener : public gcn::ActionListener
    {
        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);
    } listener;
}

static const int defaultScreenWidth = 800;
static const int defaultScreenHeight = 600;

static const int defaultSfxVolume = 100;
static const int defaultMusicVolume = 60;

std::string token; //used to store magic_token

// Account infos
char n_server, n_character;

// TODO Anyone knows a good location for this? Or a way to make it non-global?
class SERVER_INFO;
SERVER_INFO **server_info;

#ifdef TMWSERV_SUPPORT
extern Net::Connection *gameServerConnection;
extern Net::Connection *chatServerConnection;
extern Net::Connection *accountServerConnection;
#endif

Graphics *graphics;
Game *game = 0;

State state = STATE_NULL;
std::string errorMessage;

Sound sound;
Music *bgm;

Configuration config;         /**< XML file configuration reader */
Configuration branding;       /**< XML branding information reader */
Logger *logger;               /**< Log object */
KeyboardConfig keyboard;

LoginData loginData;
LockedArray<LocalPlayer*> charInfo(maxSlot + 1);

Palette *guiPalette;

// This anonymous namespace hides whatever is inside from other modules.
namespace {

std::string homeDir;
std::string updateHost;
std::string updatesDir;

SDL_Surface *icon;

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
        noOpenGL(false),
        serverPort(0)
    {}

    bool printHelp;
    bool printVersion;
    bool skipUpdate;
    bool chooseDefault;
    bool noOpenGL;
    std::string username;
    std::string password;
    std::string character;
    std::string configPath;
    std::string updateHost;
    std::string dataPath;
    std::string homeDir;

    std::string serverName;
    short serverPort;

};

/**
 * Parse the update host and determine the updates directory
 * Then verify that the directory exists (creating if needed).
 */
static void setUpdatesDir()
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
            errorMessage = _("Invalid update host: ") + updateHost;
            state = STATE_ERROR;
        }
    }
    else
    {
        logger->log("Warning: no protocol was specified for the update host");
        updates << "updates/" << updateHost << "/" << loginData.port;
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

/**
 * Initializes the home directory. On UNIX and FreeBSD, ~/.tmw is used. On
 * Windows and other systems we use the current working directory.
 */
static void initHomeDir(const Options &options)
{
    homeDir = options.homeDir;

    if (homeDir.empty())
    {
#ifdef __APPLE__
        // Use Application Directory instead of .tmw
        homeDir = std::string(PHYSFS_getUserDir()) +
            "/Library/Application Support/" +
            branding.getValue("appName", "The Mana World");
#else
        homeDir = std::string(PHYSFS_getUserDir()) +
            "/." + branding.getValue("appShort", "tmw");
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
        std::cout << homeDir
                  << _(" can't be created, but it doesn't exist! Exiting.")
                  << std::endl;
        exit(1);
    }
}

/**
 * Initialize configuration.
 */
static void initConfiguration(const Options &options)
{
    // Fill configuration with defaults
    logger->log("Initializing configuration...");
    std::string defaultHost = branding.getValue("defaultServer",
        "server.themanaworld.org");
    config.setValue("host", defaultHost);
    int defaultPort = (int)branding.getValue("defaultPort", DEFAULT_PORT);
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
    if (configFile == NULL) {
        // We reopen the file in write mode and we create it
        configFile = fopen(configPath.c_str(), "wt");
    }
    if (configFile == NULL) {
        std::cout << "Can't create " << configPath << ". "
                  << "Using Defaults." << std::endl;
    } else {
        fclose(configFile);
        config.init(configPath);
    }
}

/**
 * Do all initialization stuff.
 */
static void initEngine(const Options &options)
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

    SDL_WM_SetCaption(branding.getValue("appName", "The Mana World").c_str(),
                      NULL);

    ResourceManager *resman = ResourceManager::getInstance();

    if (!resman->setWriteDir(homeDir)) {
        std::cout << homeDir
                  << " couldn't be set as home directory! Exiting."
                  << std::endl;
        exit(1);
    }

    // Add the user's homedir to PhysicsFS search path
    resman->addToSearchPath(homeDir, false);

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
    icon = IMG_Load(resman->getPath(branding.getValue("appIcon", "data/icons/tmw.png")).c_str());
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
        std::cerr << _("Couldn't set ")
                  << width << "x" << height << "x" << bpp << _(" video mode: ")
                  << SDL_GetError() << std::endl;
        exit(1);
    }

    // Initialize for drawing
    graphics->_beginDraw();

    // Initialize the item shortcuts.
    itemShortcut = new ItemShortcut;

    // Initialize the emote shortcuts.
    emoteShortcut = new EmoteShortcut;

    gui = new Gui(graphics);
#ifdef TMWSERV_SUPPORT
    state = STATE_CHOOSE_SERVER; /**< Initial game state */
#else
    state = STATE_LOGIN; /**< Initial game state */
#endif

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
}

/** Clear the engine */
static void exitEngine()
{
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
}

static void printHelp()
{
    using std::endl;

    std::cout
        << _("tmw") << endl << endl
        << _("Options:") << endl
        << _("  -C --config-file : Configuration file to use") << endl
        << _("  -d --data        : Directory to load game data from") << endl
        << _("  -D --default     : Choose default character server and "
                                  "character") << endl
        << _("  -h --help        : Display this help") << endl
        << _("  -S --home-dir    : Directory to use as home directory") << endl
        << _("  -H --update-host : Use this update host") << endl
        << _("  -P --password    : Login with this password") << endl
        << _("  -c --character   : Login with this character") << endl
        << _("  -p --port        : Login server port") << endl
        << _("  -s --server      : Login server name or IP") << endl
        << _("  -u --skip-update : Skip the update downloads") << endl
        << _("  -U --username    : Login with this username") << endl
#ifdef USE_OPENGL
        << _("  -O --no-opengl   : Disable OpenGL for this session") << endl
#endif
        << _("  -v --version     : Display the version") << endl;
}

static void printVersion()
{
    std::cout << strprintf("The Mana World %s", FULL_VERSION) << std::endl;
}

static void parseOptions(int argc, char *argv[], Options &options)
{
    const char *optstring = "hvud:U:P:Dc:s:p:C:H:S:O";

    const struct option long_options[] = {
        { "config-file", required_argument, 0, 'C' },
        { "data",        required_argument, 0, 'd' },
        { "default",     no_argument,       0, 'D' },
        { "password",    required_argument, 0, 'P' },
        { "character",   required_argument, 0, 'c' },
        { "help",        no_argument,       0, 'h' },
        { "home-dir",    required_argument, 0, 'S' },
        { "update-host", required_argument, 0, 'H' },
        { "port",        required_argument, 0, 'p' },
        { "server",      required_argument, 0, 's' },
        { "skip-update", no_argument,       0, 'u' },
        { "username",    required_argument, 0, 'U' },
        { "no-opengl",   no_argument,       0, 'O' },
        { "version",     no_argument,       0, 'v' },
        { 0 }
    };

    while (optind < argc)
    {

        int result = getopt_long(argc, argv, optstring, long_options, NULL);

        if (result == -1)
            break;

        switch (result)
        {
            case 'C':
                options.configPath = optarg;
                break;
            case 'd':
                options.dataPath = optarg;
                break;
            case 'D':
                options.chooseDefault = true;
                break;
            default: // Unknown option
            case 'h':
                options.printHelp = true;
                break;
            case 'H':
                options.updateHost = optarg;
                break;
            case 'c':
                options.character = optarg;
                break;
            case 'P':
                options.password = optarg;
                break;
            case 's':
                options.serverName = optarg;
                break;
            case 'p':
                options.serverPort = (short) atoi(optarg);
                break;
            case 'u':
                options.skipUpdate = true;
                break;
            case 'U':
                options.username = optarg;
                break;
            case 'v':
                options.printVersion = true;
                break;
            case 'S':
                options.homeDir = optarg;
                break;
            case 'O':
                options.noOpenGL = true;
                break;
        }
    }
}

/**
 * Reads the file "{Updates Directory}/resources2.txt" and attempts to load
 * each update mentioned in it.
 */
static void loadUpdates()
{
    if (updatesDir.empty()) return;
    const std::string updatesFile = "/" + updatesDir + "/resources2.txt";
    ResourceManager *resman = ResourceManager::getInstance();
    std::vector<std::string> lines = resman->loadTextFile(updatesFile);

    for (unsigned int i = 0; i < lines.size(); ++i)
    {
        std::stringstream line(lines[i]);
        std::string filename;
        line >> filename;
        resman->addToSearchPath(homeDir + "/" + updatesDir + "/"
                                + filename, false);
    }
}

struct ErrorListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event)
    {
#ifdef TMWSERV_SUPPORT
        state = STATE_CHOOSE_SERVER;
#else
        state = loginData.registerLogin ? STATE_REGISTER : STATE_LOGIN;
#endif
    }
} errorListener;

struct AccountListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event)
    {
        state = STATE_CHAR_SELECT;
    }
} accountListener;

#ifdef TMWSERV_SUPPORT
struct LoginListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event)
    {
        state = STATE_LOGIN;
    }
} loginListener;
#endif

} // namespace

// TODO Find some nice place for these functions
#ifdef TMWSERV_SUPPORT
static void accountLogin(LoginData *loginData)
#else
static void accountLogin(Network *network, LoginData *loginData)
#endif
{
#ifdef EATHENA_SUPPORT
    logger->log("Trying to connect to account server...");
#endif
    logger->log("Username is %s", loginData->username.c_str());
#ifdef EATHENA_SUPPORT
    network->connect(loginData->hostname, loginData->port);
#endif

#ifdef TMWSERV_SUPPORT
    Net::getCharHandler()->setCharInfo(&charInfo);
#endif

    // Send login infos
    if (loginData->registerLogin) {
        Net::getLoginHandler()->registerAccount(loginData);
    } else {
        Net::getLoginHandler()->loginAccount(loginData);
    }

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

#ifdef EATHENA_SUPPORT

static void positionDialog(Window *dialog, int screenWidth, int screenHeight)
{
    dialog->setPosition(
            (screenWidth - dialog->getWidth()) / 2,
            (screenHeight - dialog->getHeight()) / 2);
}

static void charLogin(Network *network, LoginData *loginData)
{
    logger->log("Trying to connect to char server...");
    network->connect(loginData->hostname, loginData->port);
    Net::getCharHandler()->setCharInfo(&charInfo);

    // Send login infos
    Net::getCharHandler()->connect(loginData);
}

static void mapLogin(Network *network, LoginData *loginData)
{
    logger->log("Memorizing selected character %s",
            player_node->getName().c_str());
    config.setValue("lastCharacter", player_node->getName());

    logger->log("Trying to connect to map server...");
    logger->log("Map: %s", map_path.c_str());

    // EAthena::MapHandler *mapHandler = new EAthena::MapHandler;
    network->connect(loginData->hostname, loginData->port);

    Net::getMapHandler()->connect(loginData);
}

#else

static void accountRegister(LoginData *loginData)
{
    logger->log("Username is %s", loginData->username.c_str());

    Net::getCharHandler()->setCharInfo(&charInfo);
    Net::getLoginHandler()->registerAccount(loginData);
}

static void switchCharacter(std::string *passToken)
{
    Net::getLogoutHandler()->reset();
    Net::getLogoutHandler()->setScenario(LOGOUT_SWITCH_CHARACTER, passToken);
}

static void switchAccountServer()
{
    Net::getLogoutHandler()->reset();
    Net::getLogoutHandler()->setScenario(LOGOUT_SWITCH_LOGIN);
}

static void logoutThenExit()
{
    Net::getLogoutHandler()->reset();
    Net::getLogoutHandler()->setScenario(LOGOUT_EXIT);
}

static void reconnectAccount(const std::string &passToken)
{
    Net::getCharHandler()->setCharInfo(&charInfo);

    Net::AccountServer::reconnectAccount(accountServerConnection, passToken);
}

#endif


extern "C" char const *_nl_locale_name_default(void);

static void initInternationalization()
{
#if ENABLE_NLS
#ifdef WIN32
    putenv(("LANG=" + std::string(_nl_locale_name_default())).c_str());
    // mingw doesn't like LOCALEDIR to be defined for some reason
    bindtextdomain("tmw", "translations/");
#else
    bindtextdomain("tmw", LOCALEDIR);
#endif
    setlocale(LC_MESSAGES, "");
    bind_textdomain_codeset("tmw", "UTF-8");
    textdomain("tmw");
#endif
}

static void xmlNullLogger(void *ctx, const char *msg, ...)
{
    // Does nothing, that's the whole point of it
}

// Initialize libxml2 and check for potential ABI mismatches between
// compiled version and the shared library actually used.
static void initXML()
{
    xmlInitParser();
    LIBXML_TEST_VERSION;

    // Suppress libxml2 error messages
    xmlSetGenericErrorFunc(NULL, xmlNullLogger);
}

/** Main */
int main(int argc, char *argv[])
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

    initInternationalization();

    // Initialize PhysicsFS
    PHYSFS_init(argv[0]);

    initXML();

    // Load branding information
    branding.init("data/branding.xml");

    initHomeDir(options);

    // Configure logger
    logger = new Logger;
    logger->setLogFile(homeDir + std::string("/tmw.log"));

    // Log the tmw version
    logger->log("The Mana World %s", FULL_VERSION);

    initConfiguration(options);
    logger->setLogToStandardOut(config.getValue("logToStandardOut", 0));

    initEngine(options);

    // Needs to be created in main, as the updater uses it
    guiPalette = new Palette;

    Window *currentDialog = NULL;
#ifdef TMWSERV_SUPPORT
    QuitDialog* quitDialog = NULL;
#endif
    setupWindow = new Setup;

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());
    Desktop *desktop = new Desktop;
    top->add(desktop);
    ProgressBar *progressBar = new ProgressBar(0.0f, 100, 20,
                                               gcn::Color(168, 116, 31));
    progressBar->setSmoothProgress(false);
    gcn::Label *progressLabel = new Label;
    top->add(progressBar, 5, top->getHeight() - 5 - progressBar->getHeight());
    top->add(progressLabel, 15 + progressBar->getWidth(),
                            progressBar->getY() + 4);
    progressBar->setVisible(false);
    gcn::Button *setupButton = new Button(_("Setup"), "Setup", &listener);
    setupButton->setPosition(top->getWidth() - setupButton->getWidth() - 3, 3);
    top->add(setupButton);

    sound.playMusic(branding.getValue("loginMusic", "Magick - Real.ogg"));

    // Initialize login data
    loginData.hostname = options.serverName;
    loginData.port = options.serverPort;
    loginData.username = options.username;
    loginData.password = options.password;
    loginData.remember = config.getValue("remember", 0);
    loginData.registerLogin = false;

    if (loginData.hostname.empty()) {
        loginData.hostname = branding.getValue("defaultServer",
                                               "server.themanaworld.org").c_str();
    }
    if (options.serverPort == 0) {
        loginData.port = (short) branding.getValue("defaultPort", DEFAULT_PORT);
    }
    if (loginData.username.empty() && loginData.remember) {
        loginData.username = config.getValue("username", "");
    }

#ifdef TMWSERV_SUPPORT
    Net::initialize();
    new TmwServ::GeneralHandler;  // Currently doesn't need registration
#else
    Network *network = new Network;
    network->registerHandler(new EAthena::GeneralHandler);
#endif

    Net::getGeneralHandler()->load();

    int screenWidth = (int) config.getValue("screenwidth", defaultScreenWidth);
    int screenHeight = static_cast<int>(config.getValue("screenheight",
                                                        defaultScreenHeight));

    desktop->setSize(screenWidth, screenHeight);

    State oldstate = STATE_EXIT; // We start with a status change

    SDL_Event event;

    while (state != STATE_EXIT)
    {
        bool handledEvents = false;

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
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
#ifdef TMWSERV_SUPPORT
                        if (!quitDialog)
                            quitDialog = new QuitDialog(NULL, &quitDialog);
                        else
                            quitDialog->requestMoveToTop();
#else
                        state = STATE_EXIT;
#endif
                    }
                    break;
            }

            guiInput->pushInput(event);
        }

        Net::getGeneralHandler()->flushNetwork();
        gui->logic();

        Net::getGeneralHandler()->tick();

        if (progressBar && progressBar->isVisible())
        {
            progressBar->setProgress(progressBar->getProgress() + 0.005f);
            if (progressBar->getProgress() == 1.0f)
                progressBar->setProgress(0.0f);
        }

        gui->draw();
        graphics->updateScreen();

#ifdef TMWSERV_SUPPORT
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
//            Net::clearHandlers();

            state = STATE_GAME;
        }
        else if (state == STATE_RECONNECT_ACCOUNT &&
                 accountServerConnection->isConnected())
        {
            reconnectAccount(token);
            state = STATE_WAIT;
        }

        if (state != oldstate)
        {
            // Load updates after exiting the update state
            if (oldstate == STATE_UPDATE)
            {
                loadUpdates();
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
                    // We show the dialog box only if the command-line
                    // options weren't set.
                    if (options.serverName.empty() && options.serverPort == 0) {
                        currentDialog = new ServerDialog(&loginData);
                    } else {
                        state = STATE_CONNECT_ACCOUNT;

                        // Reset options so that cancelling or connect
                        // timeout will show the server dialog.
                        options.serverName.clear();
                        options.serverPort = 0;
                    }
                    break;

                case STATE_CONNECT_ACCOUNT:
                    logger->log("State: CONNECT_ACCOUNT");
                    logger->log("Trying to connect to account server...");
                    accountServerConnection->connect(loginData.hostname,
                            loginData.port);
                    currentDialog = new ConnectionDialog(
                            STATE_SWITCH_ACCOUNTSERVER_ATTEMPT);
                    break;

                case STATE_UPDATE:
                    // Determine which source to use for the update host
                    if (!options.updateHost.empty())
                        updateHost = options.updateHost;
                    else
                        updateHost = loginData.updateHost;

                    setUpdatesDir();
                    logger->log("State: UPDATE");
                    currentDialog = new UpdaterWindow(updateHost,
                            homeDir + "/" + updatesDir);
                    break;

                case STATE_LOGIN:
                    logger->log("State: LOGIN");
                    if (options.username.empty()
                            || options.password.empty()) {
                        currentDialog = new LoginDialog(&loginData);
                    } else {
                        state = STATE_LOGIN_ATTEMPT;
                        // Clear the password so that when login fails, the
                        // dialog will show up next time.
                        options.password.clear();
                    }
                    break;

                case STATE_LOADDATA:
                    logger->log("State: LOADDATA");

                    // Add customdata directory
                    ResourceManager::getInstance()->searchAndAddArchives(
                            "customdata/",
                            "zip",
                            false);

                    // Load XML databases
                    ColorDB::load();
                    ItemDB::load();
                    Being::load(); // Hairstyles
                    MonsterDB::load();
                    NPCDB::load();
                    EmoteDB::load();
                    Units::loadUnits();

                    desktop->reloadWallpaper();

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
                            selectByName(options.character)) {
                        ((CharSelectDialog*) currentDialog)->action(
                            gcn::ActionEvent(NULL, "ok"));
                    } else {
                        ((CharSelectDialog*) currentDialog)->selectByName(
                            config.getValue("lastCharacter", ""));
                    }

                    break;

                case STATE_CHANGEEMAIL_ATTEMPT:
                    logger->log("State: CHANGE EMAIL ATTEMPT");
                    Net::getLoginHandler()->changeEmail(loginData.newEmail);
                    break;

                case STATE_CHANGEEMAIL:
                    logger->log("State: CHANGE EMAIL");
                    currentDialog = new OkDialog("Email Address change",
                            "Email Address changed successfully!");
                    currentDialog->addActionListener(&accountListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    loginData.email = loginData.newEmail;
                    loginData.newEmail = "";
                    break;

                case STATE_CHANGEPASSWORD_ATTEMPT:
                    logger->log("State: CHANGE PASSWORD ATTEMPT");
                    Net::getLoginHandler()->changePassword(loginData.username,
                                                loginData.password,
                                                loginData.newPassword);
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
                    Net::getLoginHandler()->unregisterAccount(
                            loginData.username, loginData.password);
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

                    delete setupButton;
                    delete desktop;
                    setupButton = NULL;
                    desktop = NULL;

                    currentDialog = NULL;

                    logger->log("State: GAME");
                    game = new Game;
                    game->logic();
                    delete game;
                    game = 0;

                    state = STATE_EXIT;

                    logoutThenExit();
                    Net::getGeneralHandler()->unload();

                    break;

                case STATE_SWITCH_CHARACTER:
                    logger->log("State: SWITCH_CHARACTER");
                    switchCharacter(&token);
                    break;

                case STATE_RECONNECT_ACCOUNT:
                    logger->log("State: RECONNECT_ACCOUNT");

                    // Done with game & chat
                    gameServerConnection->disconnect();
                    chatServerConnection->disconnect();

                    accountServerConnection->connect(
                            loginData.hostname,
                            loginData.port);
                    break;

                case STATE_WAIT:
                    break;

                default:
                    state = STATE_FORCE_QUIT;
                    break;
            }
        }

#else // no TMWSERV_SUPPORT

        if (state != oldstate)
        {
            switch (oldstate)
            {
                case STATE_UPDATE:
                    loadUpdates();
                    break;

                // Those states don't cause a network disconnect
                case STATE_LOADDATA:
                case STATE_CHANGEPASSWORD_ATTEMPT:
                case STATE_CHANGEPASSWORD:
                case STATE_ACCOUNTCHANGE_ERROR:
                    break;

                case STATE_CHAR_SELECT:
                	if (state == STATE_CONNECTING)
                		network->disconnect();
                    break;

                case STATE_ACCOUNT:
                case STATE_CHAR_CONNECT:
                case STATE_CONNECTING:
                    progressBar->setVisible(false);
                    progressLabel->setCaption("");
                    break;

                default:
                    network->disconnect();
                    break;
            }

            oldstate = state;

            if (currentDialog && state != STATE_ACCOUNT &&
                    state != STATE_CHAR_CONNECT)
            {
                delete currentDialog;
                currentDialog = NULL;
            }

            switch (state)
            {
                case STATE_LOADDATA:
                    logger->log("State: LOADDATA");

                    // Add customdata directory
                    ResourceManager::getInstance()->searchAndAddArchives(
                        "customdata/",
                        "zip",
                        false);

                    // Load XML databases
                    ColorDB::load();
                    ItemDB::load();
                    MonsterDB::load();
                    NPCDB::load();
                    EmoteDB::load();
                    StatusEffect::load();
                    Being::load(); // Hairstyles

                    // Load units
                    Units::loadUnits();

                    desktop->reloadWallpaper();

                    state = STATE_CHAR_CONNECT;
                    break;

                case STATE_LOGIN:
                    logger->log("State: LOGIN");

                    if (!loginData.password.empty())
                    {
                        loginData.registerLogin = false;
                        state = STATE_ACCOUNT;
                    }
                    else
                    {
                        currentDialog = new LoginDialog(&loginData);
                        positionDialog(currentDialog, screenWidth,
                                                      screenHeight);
                    }
                    break;

                case STATE_REGISTER:
                    logger->log("State: REGISTER");
                    currentDialog = new RegisterDialog(&loginData);
                    positionDialog(currentDialog, screenWidth, screenHeight);
                    break;

                case STATE_CHAR_SERVER:
                    logger->log("State: CHAR_SERVER");

                    if (n_server == 1)
                    {
                        SERVER_INFO *si = *server_info;
                        loginData.hostname = ipToString(si->address);
                        loginData.port = si->port;
                        loginData.updateHost = si->updateHost;
                        state = STATE_UPDATE;
                    }
                    else
                    {
                        State nextState = STATE_UPDATE;
                        currentDialog = new ServerSelectDialog(&loginData,
                                                               nextState);
                        positionDialog(currentDialog, screenWidth,
                                                      screenHeight);
                        if (options.chooseDefault)
                        {
                            ((ServerSelectDialog*) currentDialog)->action(
                                gcn::ActionEvent(NULL, "ok"));
                        }
                    }
                    break;
                case STATE_CHAR_SELECT:
                    logger->log("State: CHAR_SELECT");
                    currentDialog = new CharSelectDialog(&charInfo,
                                                         &loginData);
                    positionDialog(currentDialog, screenWidth, screenHeight);

                    if (((CharSelectDialog*) currentDialog)->
                            selectByName(options.character))
                        options.chooseDefault = true;
                    else
                        ((CharSelectDialog*) currentDialog)->selectByName(
                            config.getValue("lastCharacter", ""));

                    if (options.chooseDefault)
                        ((CharSelectDialog*) currentDialog)->action(
                            gcn::ActionEvent(NULL, "ok"));

                    break;

                case STATE_GAME:
                    delete progressBar;
                    delete progressLabel;
                    delete setupButton;
                    delete desktop;
                    progressBar = NULL;
                    progressLabel = NULL;
                    currentDialog = NULL;
                    setupButton = NULL;
                    desktop = NULL;

                    logger->log("State: GAME");
                    game->logic();
                    delete game;
                    game = 0;
                    state = STATE_EXIT;
                    break;

                case STATE_UPDATE:
                    if (options.skipUpdate)
                    {
                        state = STATE_LOADDATA;
                    }
                    else
                    {
                        // Determine which source to use for the update host
                        if (!options.updateHost.empty())
                            updateHost = options.updateHost;
                        else
                            updateHost = loginData.updateHost;

                        setUpdatesDir();
                        logger->log("State: UPDATE");

                        currentDialog = new UpdaterWindow(updateHost,
                                                homeDir + "/" + updatesDir);
                        positionDialog(currentDialog, screenWidth,
                                                      screenHeight);
                    }
                    break;

                case STATE_ERROR:
                    logger->log("State: ERROR");
                    currentDialog = new OkDialog(_("Error"), errorMessage);
                    positionDialog(currentDialog, screenWidth, screenHeight);
                    currentDialog->addActionListener(&errorListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    network->disconnect();
                    break;

                case STATE_CONNECTING:
                    logger->log("State: CONNECTING");
                    progressBar->setVisible(true);
                    progressLabel->setCaption(
                            _("Connecting to map server..."));
                    progressLabel->adjustSize();
                    mapLogin(network, &loginData);
                    break;

                case STATE_CHAR_CONNECT:
                    progressBar->setVisible(true);
                    progressLabel->setCaption(
                            _("Connecting to character server..."));
                    progressLabel->adjustSize();
                    charLogin(network, &loginData);
                    break;

                case STATE_ACCOUNT:
                    progressBar->setVisible(true);
                    progressLabel->setCaption(
                            _("Connecting to account server..."));
                    progressLabel->adjustSize();
                    accountLogin(network, &loginData);
                    break;

                case STATE_CHANGEPASSWORD_ATTEMPT:
                    logger->log("State: CHANGE PASSWORD ATTEMPT");
                    Net::getLoginHandler()->changePassword(loginData.username,
                                                loginData.password,
                                                loginData.newPassword);
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

                case STATE_ACCOUNTCHANGE_ERROR:
                    logger->log("State: ACCOUNT CHANGE ERROR");
                    currentDialog = new OkDialog("Error ", errorMessage);
                    currentDialog->addActionListener(&accountListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    break;

                default:
                    state = STATE_EXIT;
                    break;
            }
        }
#endif
        /*
         * This loop can really stress the CPU, for no reason since it's
         * just constantly redrawing the wallpaper.  Added the following
         * usleep to limit it to 40 FPS during the login sequence
         */
        if (!handledEvents)
            usleep(25000);
    }

    delete guiPalette;

    logger->log("Quitting");
    exitEngine();
    PHYSFS_deinit();
    delete logger;

    return 0;
}

void SetupListener::action(const gcn::ActionEvent &event)
{
    Window *window = NULL;

    if (event.getId() == "Setup")
        window = setupWindow;

    if (window)
    {
        window->setVisible(!window->isVisible());
        if (window->isVisible())
            window->requestMoveToTop();
    }
}
