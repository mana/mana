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
#include "logindata.h"
#ifdef USE_OPENGL
#include "openglgraphics.h"
#endif
#include "player_relations.h"
#include "serverinfo.h"
#include "sound.h"
#include "statuseffect.h"
#include "units.h"

#include "gui/button.h"
#ifdef EATHENA_SUPPORT
#include "gui/char_server.h"
#endif
#include "gui/char_select.h"
#include "gui/gui.h"
#include "gui/label.h"
#include "gui/login.h"
#include "gui/ok_dialog.h"
#include "gui/palette.h"
#include "gui/progressbar.h"
#include "gui/register.h"
#include "gui/sdlinput.h"
#include "gui/setup.h"
#ifdef TMWSERV_SUPPORT
#include "gui/connection.h"
#include "gui/quitdialog.h"
#include "gui/serverdialog.h"
#endif
#include "gui/updatewindow.h"

#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/charserverhandler.h"
#include "net/tmwserv/connection.h"
#include "net/tmwserv/loginhandler.h"
#include "net/tmwserv/logouthandler.h"
#include "net/tmwserv/network.h"
#else
#include "net/ea/charserverhandler.h"
#include "net/ea/loginhandler.h"
#include "net/ea/network.h"
#include "net/ea/maphandler.h"
#include "net/messageout.h"
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

#ifdef TMWSERV_SUPPORT
#include "utils/dtor.h"
#endif
#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/strprintf.h"

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

#ifdef TMWSERV_SUPPORT
std::string token; //used to store magic_token
#else
// Account infos
char n_server, n_character;

// TODO Anyone knows a good location for this? Or a way to make it non-global?
class SERVER_INFO;
SERVER_INFO **server_info;
#endif

Graphics *graphics;

unsigned char state;
std::string errorMessage;

Sound sound;
Music *bgm;

Configuration config;         /**< XML file configuration reader */
Configuration branding;       /**< XML branding information reader */
Logger *logger;               /**< Log object */
KeyboardConfig keyboard;

#ifdef TMWSERV_SUPPORT
Net::Connection *gameServerConnection = 0;
Net::Connection *chatServerConnection = 0;
Net::Connection *accountServerConnection = 0;
#endif

LoginData loginData;
#ifdef TMWSERV_SUPPORT
LoginHandler loginHandler;
LogoutHandler logoutHandler;
TmwServ::CharServerHandler charServerHandler;
#else
EAthena::CharServerHandler charServerHandler;
#endif
LockedArray<LocalPlayer*> charInfo(maxSlot + 1);

Palette *guiPalette;

// This anonymous namespace hides whatever is inside from other modules.
namespace {

std::string homeDir;
std::string updateHost;
std::string updatesDir;

#ifdef EATHENA_SUPPORT
LoginHandler loginHandler;
#endif

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
        serverPort(0)
    {}

    bool printHelp;
    bool printVersion;
    bool skipUpdate;
    bool chooseDefault;
    std::string username;
    std::string password;
    std::string character;
    std::string configPath;
    std::string updateHost;
    std::string dataPath;

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
static void initHomeDir()
{
    homeDir = std::string(PHYSFS_getUserDir()) +
        "/." +
        branding.getValue("appShort", "tmw");
#if defined WIN32
    if (!CreateDirectory(homeDir.c_str(), 0) &&
            GetLastError() != ERROR_ALREADY_EXISTS)
#elif defined __APPLE__
    // Use Application Directory instead of .tmw
    homeDir = std::string(PHYSFS_getUserDir()) +
        "/Library/Application Support/" +
        branding.getValue("appName", "The Mana World");
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
#else
    // Checking if /home/user/.tmw folder exists.
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
#ifdef TWMSERV_SUPPORT
    int defaultPort = (int)branding.getValue("defaultPort", 9601);
#else
    int defaultPort = (int)branding.getValue("defaultPort", 6901);
#endif
    config.setValue("port", defaultPort);
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
    std::string defaultUpdateHost = branding.getValue("defaultUpdateHost",
        "http://updates.themanaworld.org");
    config.setValue("updatehost", defaultUpdateHost);
    config.setValue("customcursor", 1);
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

    SDL_WM_SetCaption(branding.getValue("appName", "The Mana World").c_str(), NULL);

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
    bool useOpenGL = (config.getValue("opengl", 0) == 1);

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
    std::cout
        << _("tmw") << std::endl << std::endl
        << _("Options: ") << std::endl
        << _("  -C --configfile : Configuration file to use") << std::endl
        << _("  -d --data       : Directory to load game data from") << std::endl
        << _("  -D --default    : Bypass the login process with default settings")
        << std::endl
        << _("  -h --help       : Display this help") << std::endl
        << _("  -S --homedir    : Directory to use as home directory") << std::endl
        << _("  -H --updatehost : Use this update host") << std::endl
        << _("  -P --password   : Login with this password") << std::endl
        << _("  -c --character  : Login with this character") << std::endl
        << _("  -o --port       : Login Server Port") << std::endl
        << _("  -s --server     : Login Server name or IP") << std::endl
        << _("  -u --skipupdate : Skip the update downloads") << std::endl
        << _("  -U --username   : Login with this username") << std::endl
        << _("  -v --version    : Display the version") << std::endl;
}

static void printVersion()
{
#ifdef PACKAGE_VERSION
    std::cout << _("The Mana World version ") << PACKAGE_VERSION << std::endl;
#else
    std::cout << _("The Mana World version ") <<
             _("(local build?, PACKAGE_VERSION is not defined)") << std::endl;
#endif
}

static void parseOptions(int argc, char *argv[], Options &options)
{
    const char *optstring = "hvud:U:P:Dc:s:o:C:H:S:";

    const struct option long_options[] = {
        { "configfile", required_argument, 0, 'C' },
        { "data",       required_argument, 0, 'd' },
        { "default",    no_argument,       0, 'D' },
        { "password",   required_argument, 0, 'P' },
        { "character",  required_argument, 0, 'c' },
        { "help",       no_argument,       0, 'h' },
        { "homedir",    required_argument, 0, 'S' },
        { "updatehost", required_argument, 0, 'H' },
        { "port",       required_argument, 0, 'o' },
        { "server",     required_argument, 0, 's' },
        { "skipupdate", no_argument,       0, 'u' },
        { "username",   required_argument, 0, 'U' },
        { "version",    no_argument,       0, 'v' },
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
            case 'o':
                options.serverPort = (short)atoi(optarg);
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
                homeDir = optarg;
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

#ifdef TMWSERV_SUPPORT
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
    network->registerHandler(&loginHandler);
#endif
    loginHandler.setLoginData(loginData);
#ifdef TMWSERV_SUPPORT
    Net::registerHandler(&loginHandler);

    charServerHandler.setCharInfo(&charInfo);
    Net::registerHandler(&charServerHandler);
#endif

    // Send login infos
#ifdef TMWSERV_SUPPORT
    Net::AccountServer::login(accountServerConnection,
            0,  // client version
            loginData->username,
            loginData->password);
#else
    MessageOut outMsg(0x0064);
    outMsg.writeInt32(0); // client version
    outMsg.writeString(loginData->username, 24);
    outMsg.writeString(loginData->password, 24);

    /*
     * eAthena calls the last byte "client version 2", but it isn't used at
     * at all. We're retasking it, with bit 0 to indicate whether the client
     * can handle the 0x63 "update host" packet. Clients prior to 0.0.25 send
     * 0 here.
     */
    outMsg.writeInt8(0x01);
#endif

    // Clear the password, avoids auto login when returning to login
    loginData->password = "";

#ifdef EATHENA_SUPPORT
    // Remove _M or _F from username after a login for registration purpose
    if (loginData->registerLogin)
    {
        loginData->username =
            loginData->username.substr(0, loginData->username.length() - 2);
    }
#endif

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
    network->registerHandler(&charServerHandler);
    charServerHandler.setCharInfo(&charInfo);
    charServerHandler.setLoginData(loginData);

    // Send login infos
    MessageOut outMsg(0x0065);
    outMsg.writeInt32(loginData->account_ID);
    outMsg.writeInt32(loginData->session_ID1);
    outMsg.writeInt32(loginData->session_ID2);
    // [Fate] The next word is unused by the old char server, so we squeeze in
    //        tmw client version information
    outMsg.writeInt16(CLIENT_PROTOCOL_VERSION);
    outMsg.writeInt8(loginData->sex);

    // We get 4 useless bytes before the real answer comes in
    network->skip(4);
}

static void mapLogin(Network *network, LoginData *loginData)
{
    logger->log("Memorizing selected character %s",
            player_node->getName().c_str());
    config.setValue("lastCharacter", player_node->getName());

    logger->log("Trying to connect to map server...");
    logger->log("Map: %s", map_path.c_str());

    EAthena::MapHandler *mapHandler = new EAthena::MapHandler;
    network->connect(loginData->hostname, loginData->port);
    //network->registerHandler(mapHandler);
    network->registerHandler(mapHandler);

    mapHandler->connect(loginData);

    // We get 4 useless bytes before the real answer comes in (what are these?)
    network->skip(4);
}

#else

static void accountRegister(LoginData *loginData)
{
    logger->log("Username is %s", loginData->username.c_str());

    Net::registerHandler(&loginHandler);

    charServerHandler.setCharInfo(&charInfo);
    Net::registerHandler(&charServerHandler);

    Net::AccountServer::registerAccount(accountServerConnection,
            0, // client version
            loginData->username,
            loginData->password,
            loginData->email);
}

static void accountUnRegister(LoginData *loginData)
{
    Net::registerHandler(&logoutHandler);

    Net::AccountServer::Account::unregister(loginData->username,
                                            loginData->password);

}

static void accountChangePassword(LoginData *loginData)
{
    Net::registerHandler(&loginHandler);

    Net::AccountServer::Account::changePassword(loginData->username,
                                                loginData->password,
                                                loginData->newPassword);
}

static void accountChangeEmail(LoginData *loginData)
{
    Net::registerHandler(&loginHandler);

    Net::AccountServer::Account::changeEmail(loginData->newEmail);
}

static void switchCharacter(std::string *passToken)
{
    Net::registerHandler(&logoutHandler);

    logoutHandler.reset();
    logoutHandler.setScenario(LOGOUT_SWITCH_CHARACTER, passToken);

    Net::GameServer::logout(true);
    Net::ChatServer::logout();
}

static void switchAccountServer()
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

static void logoutThenExit()
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

static void reconnectAccount(const std::string &passToken)
{
    Net::registerHandler(&loginHandler);

    charServerHandler.setCharInfo(&charInfo);
    Net::registerHandler(&charServerHandler);

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

    initHomeDir();

    // Configure logger
    logger = new Logger;
    logger->setLogFile(homeDir + std::string("/tmw.log"));

    // Log the tmw version
    logger->log("The Mana World %s (%s)",
#ifdef PACKAGE_VERSION
                "v" PACKAGE_VERSION,
#else
                "- version not defined",
#endif
#ifdef TMWSERV_SUPPORT
                "tmwserv");
#else
                "eAthena");
#endif

    initConfiguration(options);
    logger->setLogToStandardOut(config.getValue("logToStandardOut", 0));

    initEngine(options);

    // Needs to be created in main, as the updater uses it
    guiPalette = new Palette;

    Game *game = NULL;
    Window *currentDialog = NULL;
#ifdef TMWSERV_SUPPORT
    QuitDialog* quitDialog = NULL;
#endif
    Image *login_wallpaper = NULL;
    setupWindow = new Setup;

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());
#ifdef PACKAGE_VERSION
#ifdef TMWSERV_SUPPORT
    gcn::Label *versionLabel = new Label(strprintf("%s (tmwserv)", PACKAGE_VERSION));
#else
    gcn::Label *versionLabel = new Label(strprintf("%s (eAthena)", PACKAGE_VERSION));
#endif
    top->add(versionLabel, 25, 2);
#endif
    ProgressBar *progressBar = new ProgressBar(0.0f, 100, 20, 168, 116, 31);
    gcn::Label *progressLabel = new Label;
    top->add(progressBar, 5, top->getHeight() - 5 - progressBar->getHeight());
    top->add(progressLabel, 15 + progressBar->getWidth(),
                            progressBar->getY() + 4);
    progressBar->setVisible(false);
    gcn::Button *setup = new Button(_("Setup"), "Setup", &listener);
    setup->setPosition(top->getWidth() - setup->getWidth() - 3, 3);
    top->add(setup);

    sound.playMusic(branding.getValue("loginMusic", ""));

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
        loginData.port = (short) branding.getValue("defaultPort", 9601);
    }
    if (loginData.username.empty() && loginData.remember) {
        loginData.username = config.getValue("username", "");
    }

#ifdef TMWSERV_SUPPORT
    Net::initialize();
    accountServerConnection = Net::getConnection();
    gameServerConnection = Net::getConnection();
    chatServerConnection = Net::getConnection();
#else
    SDLNet_Init();
    Network *network = new Network;
#endif

    // Set the most appropriate wallpaper, based on screen width
    int screenWidth = (int) config.getValue("screenwidth", defaultScreenWidth);
#ifdef EATHENA_SUPPORT
    int screenHeight = static_cast<int>(config.getValue("screenheight",
                                                        defaultScreenHeight));
#endif
    std::string wallpaperName;

    wallpaperName = "graphics/images/login_wallpaper.png";
    if (screenWidth >= 1024 && screenWidth < 1280)
        wallpaperName = "graphics/images/login_wallpaper_1024x768.png";
    else if (screenWidth >= 1280 && screenWidth < 1440)
        wallpaperName = "graphics/images/login_wallpaper_1280x960.png";
    else if (screenWidth >= 1440 && screenWidth < 1600)
        wallpaperName = "graphics/images/login_wallpaper_1440x1080.png";
    else if (screenWidth >= 1600)
        wallpaperName = "graphics/images/login_wallpaper_1600x1200.png";

    if (!ResourceManager::getInstance()->exists(wallpaperName))
        wallpaperName = "graphics/images/login_wallpaper.png";

    login_wallpaper = ResourceManager::getInstance()->getImage(wallpaperName);

    if (!login_wallpaper)
        logger->log("Couldn't load %s as wallpaper", wallpaperName.c_str());

    unsigned int oldstate = !state; // We start with a status change.

    SDL_Event event;
#ifdef TMWSERV_SUPPORT
    while (state != STATE_FORCE_QUIT)
#else
    while (state != STATE_EXIT)
#endif
    {
        // Handle SDL events
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
#ifdef TMWSERV_SUPPORT
                    state = STATE_FORCE_QUIT;
#else
                    state = STATE_EXIT;
#endif
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

#ifdef TMWSERV_SUPPORT
        Net::flush();
#else
        network->flush();
        network->dispatchMessages();
#endif
        gui->logic();

#ifdef EATHENA_SUPPORT
        if (network->getState() == Network::NET_ERROR)
        {
            state = STATE_ERROR;

            if (!network->getError().empty())
                errorMessage = network->getError();
            else
                errorMessage = _("Got disconnected from server!");
        }
#endif

        if (progressBar->isVisible())
        {
            progressBar->setProgress(progressBar->getProgress() + 0.005f);
            if (progressBar->getProgress() == 1.0f)
                progressBar->setProgress(0.0f);
        }

        if (graphics->getWidth() > login_wallpaper->getWidth() ||
                graphics->getHeight() > login_wallpaper->getHeight())
        {
            graphics->setColor(gcn::Color(64, 64, 64));
            graphics->fillRectangle(gcn::Rectangle(
                        0, 0, graphics->getWidth(), graphics->getHeight()));
        }
        graphics->drawImage(login_wallpaper,
                (graphics->getWidth() - login_wallpaper->getWidth()) / 2,
                (graphics->getHeight() - login_wallpaper->getHeight()) / 2);
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
            Net::clearHandlers();

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
                // Reload the wallpaper in case that it was updated
                login_wallpaper->decRef();
                login_wallpaper = ResourceManager::getInstance()->getImage(
                        branding.getValue("loginWallpaper",
                            "graphics/images/login_wallpaper.png"));
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
                    MonsterDB::load();
                    NPCDB::load();
                    EmoteDB::load();
                    Units::loadUnits();

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
                    accountChangeEmail(&loginData);
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

                    // If the quitdialog didn't set the next state
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

                    // Done with game & chat
                    gameServerConnection->disconnect();
                    chatServerConnection->disconnect();

                    accountServerConnection->connect(
                            loginData.hostname,
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

#else // no TMWSERV_SUPPORT

        if (state != oldstate)
        {
            switch (oldstate)
            {
                case STATE_UPDATE:
                    loadUpdates();
                    // Reload the wallpaper in case that it was updated
                    login_wallpaper->decRef();
                    login_wallpaper = ResourceManager::getInstance()->
                        getImage(wallpaperName);
                    break;

                    // Those states don't cause a network disconnect
                case STATE_LOADDATA:
                    break;

                case STATE_ACCOUNT:
                case STATE_CHAR_CONNECT:
                case STATE_CONNECTING:
                    progressBar->setVisible(false);
                    progressLabel->setCaption("");
                    break;

                default:
                    network->disconnect();
                    network->clearHandlers();
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
                        int nextState = STATE_UPDATE;
                        currentDialog = new ServerSelectDialog(&loginData,
                                                                nextState);
                        positionDialog(currentDialog, screenWidth,
                                                      screenHeight);
                        if (options.chooseDefault
                                || !options.character.empty())
                        {
                            ((ServerSelectDialog*) currentDialog)->action(
                                gcn::ActionEvent(NULL, "ok"));
                        }
                    }
                    break;
                case STATE_CHAR_SELECT:
                    logger->log("State: CHAR_SELECT");
                    currentDialog = new CharSelectDialog(&charInfo,
                            (loginData.sex == 0) ?
                            GENDER_FEMALE : GENDER_MALE);
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
                    sound.fadeOutMusic(1000);

#ifdef PACKAGE_VERSION
                    delete versionLabel;
                    versionLabel = NULL;
#endif
                    delete progressBar;
                    delete progressLabel;
                    delete setup;
                    progressBar = NULL;
                    progressLabel = NULL;
                    currentDialog = NULL;
                    setup = NULL;
                    login_wallpaper->decRef();
                    login_wallpaper = NULL;

                    logger->log("State: GAME");
                    game = new Game(network);
                    game->logic();
                    delete game;
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
                    network->clearHandlers();
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
        usleep(25000);
    }

    delete guiPalette;
#ifdef PACKAGE_VERSION
    delete versionLabel;
#endif
    delete progressBar;
    delete progressLabel;
    delete setup;
    delete setupWindow;

#ifdef TMWSERV_SUPPORT
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
#else
    delete network;
    SDLNet_Quit();
#endif

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
