/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#include <getopt.h>
#include <iostream>
#include <physfs.h>
#include <SDL_image.h>
#include <unistd.h>
#include <vector>

#include <guichan/actionlistener.hpp>

#include <guichan/widgets/label.hpp>

#include <libxml/parser.h>

#include <SDL/SDL_ttf.h>

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
#include "main.h"
#ifdef USE_OPENGL
#include "openglgraphics.h"
#endif
#include "player_relations.h"
#include "serverinfo.h"
#include "sound.h"

#include "gui/button.h"
#include "gui/char_server.h"
#include "gui/char_select.h"
#include "gui/colour.h"
#include "gui/gui.h"
#include "gui/login.h"
#include "gui/ok_dialog.h"
#include "gui/progressbar.h"
#include "gui/register.h"
#include "gui/sdlinput.h"
#include "gui/setup.h"
#include "gui/updatewindow.h"

#include "net/charserverhandler.h"
#include "net/loginhandler.h"
#include "net/maploginhandler.h"
#include "net/messageout.h"
#include "net/network.h"

#include "resources/colordb.h"
#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/itemdb.h"
#include "resources/monsterdb.h"
#include "resources/npcdb.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/tostring.h"

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
    Window *setupWindow = 0;

    struct SetupListener : public gcn::ActionListener
    {
        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);
    } listener;
}

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

CharServerHandler charServerHandler;
LoginData loginData;
LockedArray<LocalPlayer*> charInfo(MAX_SLOT + 1);

Colour *textColour;

// This anonymous namespace hides whatever is inside from other modules.
namespace {

std::string homeDir;
std::string updateHost;
std::string updatesDir;

LoginHandler loginHandler;
MapLoginHandler mapLoginHandler;

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
    std::string updateHost;
    std::string dataPath;
};

/**
 * Parse the update host and determine the updates directory
 * Then verify that the directory exists (creating if needed).
 */
void setUpdatesDir()
{
    std::stringstream updates;

    // If updatesHost is currently empty, fill it from config file
    if (updateHost.empty())
    {
        updateHost =
            config.getValue("updatehost", "http://www.aethyra.org/updates");
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
            updates << "updates/" << updateHost.substr(pos + 3)
                    << "/" << loginData.port;
            updatesDir = updates.str();
        }
        else
        {
            logger->log(_("Error: Invalid update host: %s"), updateHost.c_str());
            errorMessage = _("Invalid update host: ") + updateHost;
            state = ERROR_STATE;
        }
    }
    else
    {
        logger->log(_("Warning: no protocol was specified for the update host"));
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
                logger->log(_("Error: %s can't be made, but doesn't exist!"),
                              newDir.c_str());
                errorMessage = _("Error creating updates directory!");
                state = ERROR_STATE;
            }
#else
            logger->log(_("Error: %s/%s can't be made, but doesn't exist!"),
                         homeDir.c_str(), updatesDir.c_str());
            errorMessage = _("Error creating updates directory!");
            state = ERROR_STATE;
#endif
        }
    }
}

/**
 * Do all initialization stuff
 */
void init_engine(const Options &options)
{
    homeDir = std::string(PHYSFS_getUserDir()) + "/.aethyra";
#if defined WIN32
    if (!CreateDirectory(homeDir.c_str(), 0) &&
            GetLastError() != ERROR_ALREADY_EXISTS)
#elif defined __APPLE__
    // Use Application Directory instead of .aethyra
    homeDir = std::string(PHYSFS_getUserDir()) +
        "/Library/Application Support/Aethyra";
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
#else
    // Checking if /home/user/.Aethyra folder exists.
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
#endif
    {
        std::cout << homeDir
                  << _(" can't be created, but it doesn't exist! Exiting.")
                  << std::endl;
        exit(1);
    }

    // Set log file
    logger->setLogFile(homeDir + std::string("/aethyra.log"));

    #ifdef PACKAGE_VERSION
        logger->log(_("Starting Aethyra Version %s"), PACKAGE_VERSION);
    #else
        logger->log(_("Starting Aethyra - Version not defined"));
    #endif

    // Initialize SDL
    logger->log(_("Initializing SDL..."));
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << _("Could not initialize SDL: ") <<
            SDL_GetError() << std::endl;
        exit(1);
    }
    atexit(SDL_Quit);

    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    ResourceManager *resman = ResourceManager::getInstance();

    if (!resman->setWriteDir(homeDir)) {
        std::cout << homeDir
                  << _(" couldn't be set as home directory! Exiting.")
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
        fprintf(stderr, _("Can't find Resources directory\n"));
    }
    CFRelease(resourcesURL);
    strncat(path, "/data", PATH_MAX - 1);
    resman->addToSearchPath(path, true);
#else
    resman->addToSearchPath(PKG_DATADIR "data", true);
#endif

    // Fill configuration with defaults
    logger->log(_("Initializing configuration..."));
    config.setValue("host", "www.aethyra.org");
    config.setValue("port", 21001);
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
    config.setValue("updatehost", "http://www.aethyra.org/updates");
    config.setValue("customcursor", 1);
    config.setValue("ChatLogLength", 128);

    // Checking if the configuration file exists... otherwise creates it with
    // default options !
    FILE *aethyraFile = 0;
    std::string configPath = options.configPath;

    if (configPath.empty())
        configPath = homeDir + "/config.xml";

    aethyraFile = fopen(configPath.c_str(), "r");

    // If we can't read it, it doesn't exist !
    if (aethyraFile == NULL) {
        // We reopen the file in write mode and we create it
        aethyraFile = fopen(configPath.c_str(), "wt");
    }
    if (aethyraFile == NULL) {
        std::cout << "Can't create " << configPath << ". "
            "Using Defaults." << std::endl;
    } else {
        fclose(aethyraFile);
        config.init(configPath);
    }

    SDL_WM_SetCaption("Aethyra", NULL);
#ifdef WIN32
    static SDL_SysWMinfo pInfo;
    SDL_GetWMInfo(&pInfo);
    HICON icon = LoadIcon(GetModuleHandle(NULL), "A");
    if (icon)
    {
        SetClassLong(pInfo.window, GCL_HICON, (LONG) icon);
    }
#else
    SDL_Surface *icon = IMG_Load(PKG_DATADIR "data/icons/aethyra.png");
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
    itemShortcut = new ItemShortcut();

    // Initialize the emote shortcuts.
    emoteShortcut = new EmoteShortcut();

    gui = new Gui(graphics);
    state = LOGIN_STATE; /**< Initial game state */

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
        state = ERROR_STATE;
        errorMessage = err;
        logger->log(_("Warning: %s"), err);
    }

    // Initialize keyboard
    keyboard.init();

    // Initialise player relations
    player_relations.init();
}

/** Clear the engine */
void exit_engine()
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

    ResourceManager::deleteInstance();
    delete logger;
}

void printHelp()
{
    std::cout
        << _("aethyra") << std::endl << std::endl
        << _("Options: ") << std::endl
        << _("  -C --configfile : Configuration file to use") << std::endl
        << _("  -d --data       : Directory to load game data from") << std::endl
        << _("  -D --default    : Bypass the login process with default settings") << std::endl
        << _("  -h --help       : Display this help") << std::endl
        << _("  -H --updatehost : Use this update host") << std::endl
        << _("  -p --playername : Login with this player") << std::endl
        << _("  -P --password   : Login with this password") << std::endl
        << _("  -u --skipupdate : Skip the update downloads") << std::endl
        << _("  -U --username   : Login with this username") << std::endl
        << _("  -v --version    : Display the version") << std::endl;
}

void printVersion()
{
#ifdef PACKAGE_VERSION
    std::cout << _("Aethyra version ") << PACKAGE_VERSION <<
        std::endl;
#else
    std::cout << _("Aethyra version ") <<
             _("(local build?, PACKAGE_VERSION is not defined)") << std::endl;
#endif
}

void parseOptions(int argc, char *argv[], Options &options)
{
    const char *optstring = "hvud:U:P:Dp:C:H:";

    const struct option long_options[] = {
        { "configfile", required_argument, 0, 'C' },
        { "data",       required_argument, 0, 'd' },
        { "default",    no_argument,       0, 'D' },
        { "playername", required_argument, 0, 'p' },
        { "password",   required_argument, 0, 'P' },
        { "help",       no_argument,       0, 'h' },
        { "updatehost", required_argument, 0, 'H' },
        { "skipupdate", no_argument,       0, 'u' },
        { "username",   required_argument, 0, 'U' },
        { "version",    no_argument,       0, 'v' },
        { 0 }
    };

    while (optind < argc) {

        int result = getopt_long(argc, argv, optstring, long_options, NULL);

        if (result == -1)
            break;

        switch (result) {
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
            case 'p':
                options.playername = optarg;
                break;
            case 'P':
                options.password = optarg;
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
        }
    }
}

/**
 * Reads the file "{Updates Directory}/resources2.txt" and attempts to load
 * each update mentioned in it.
 */
void loadUpdates()
{
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
        state = loginData.registerLogin ? REGISTER_STATE : LOGIN_STATE;
    }
} errorListener;

// TODO Find some nice place for these functions
void accountLogin(Network *network, LoginData *loginData)
{
    logger->log(_("Trying to connect to account server..."));
    logger->log(_("Username is %s"), loginData->username.c_str());
    network->connect(loginData->hostname, loginData->port);
    network->registerHandler(&loginHandler);
    loginHandler.setLoginData(loginData);

    // Send login infos
    MessageOut outMsg(network);
    outMsg.writeInt16(0x0064);
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

inline int MIN(int x, int y)
{
    return x < y ? x : y;
}

void positionDialog(Window *dialog, int screenWidth, int screenHeight)
{
    dialog->setPosition(
        MIN(screenWidth * 5 / 8, screenWidth - dialog->getWidth()),
        MIN(screenHeight * 5 / 8, screenHeight - dialog->getHeight()));
}

void charLogin(Network *network, LoginData *loginData)
{
    logger->log(_("Trying to connect to char server..."));
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
    // [Fate] The next word is unused by the old char server, so we squeeze in tmw client version information
    outMsg.writeInt16(CLIENT_PROTOCOL_VERSION);
    outMsg.writeInt8(loginData->sex);

    // We get 4 useless bytes before the real answer comes in
    network->skip(4);
}

void mapLogin(Network *network, LoginData *loginData)
{
    logger->log(_("Memorizing selected character %s"),
            player_node->getName().c_str());
    config.setValue("lastCharacter", player_node->getName());

    MessageOut outMsg(network);

    logger->log(_("Trying to connect to map server..."));
    logger->log(_("Map: %s"), map_path.c_str());

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

} // namespace

extern "C" char const *_nl_locale_name_default(void);

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

#if ENABLE_NLS
#ifdef WIN32
        putenv(("LANG=" + std::string(_nl_locale_name_default())).c_str());
#endif
        setlocale(LC_MESSAGES, "");
        bindtextdomain("aethyra", LOCALEDIR);
        bind_textdomain_codeset("aethyra", "UTF-8");
        textdomain("aethyra");
#endif

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

    unsigned int oldstate = !state; // We start with a status change.

    // Needs to be created in main, as the updater uses it
    textColour = new Colour();

    Game *game = NULL;
    Window *currentDialog = NULL;
    Image *login_wallpaper = NULL;
    setupWindow = new Setup();

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());
#ifdef PACKAGE_VERSION
    gcn::Label *versionLabel = new gcn::Label(PACKAGE_VERSION);
    top->add(versionLabel, 2, 2);
#endif
    ProgressBar *progressBar = new ProgressBar(0.0f, 100, 20, 168, 116, 31);
    gcn::Label *progressLabel = new gcn::Label();
    top->add(progressBar, 5, top->getHeight() - 5 - progressBar->getHeight());
    top->add(progressLabel, 15 + progressBar->getWidth(),
                            progressBar->getY() + 4);
    progressBar->setVisible(false);
    gcn::Button *setup = new Button(_("Setup"), "Setup", &listener);
    setup->setPosition(top->getWidth() - setup->getWidth() - 3, 3);
    top->add(setup);

    sound.playMusic("Magick - Real.ogg");

    loginData.username = options.username;
    if (loginData.username.empty()) {
        if (config.getValue("remember", 0)) {
            loginData.username = config.getValue("username", "");
        }
    }
    if (!options.password.empty()) {
        loginData.password = options.password;
    }
    loginData.hostname = config.getValue("host", "www.aethyra.org");
    loginData.port = (short)config.getValue("port", 21001);
    loginData.remember = config.getValue("remember", 0);
    loginData.registerLogin = false;

    SDLNet_Init();
    Network *network = new Network();

    // Set the most appropriate wallpaper, based on screen width
    int screenWidth = (int) config.getValue("screenwidth", defaultScreenWidth);
    int screenHeight = static_cast<int>(config.getValue("screenheight",
                                                        defaultScreenHeight));
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

    login_wallpaper = ResourceManager::getInstance()-> getImage(wallpaperName);

    if (!login_wallpaper)
        logger->log(_("Couldn't load %s as wallpaper"), wallpaperName.c_str());

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
                    {
                        state = EXIT_STATE;
                    }
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
                errorMessage = _("Got disconnected from server!");
            }
        }

        if (progressBar->isVisible())
        {
            progressBar->setProgress(progressBar->getProgress() + 0.005f);
            if (progressBar->getProgress() == 1.0f)
                progressBar->setProgress(0.0f);
        }

        if (graphics->getWidth() > login_wallpaper->getWidth() ||
                graphics->getHeight() > login_wallpaper->getHeight())
        {
            graphics->setColor(gcn::Color(255, 255, 255));
            graphics->fillRectangle(gcn::Rectangle(
                        0, 0, graphics->getWidth(), graphics->getHeight()));
        }
        graphics->drawImage(login_wallpaper,
                (graphics->getWidth() - login_wallpaper->getWidth()) / 2,
                (graphics->getHeight() - login_wallpaper->getHeight()) / 2);
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
                        getImage(wallpaperName);
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
                    ColorDB::load();
                    ItemDB::load();
                    MonsterDB::load();
                    NPCDB::load();
                    EmoteDB::load();

                    state = CHAR_CONNECT_STATE;
                    break;

                case LOGIN_STATE:
                    logger->log("State: LOGIN");

                    if (!loginData.password.empty()) {
                        loginData.registerLogin = false;
                        state = ACCOUNT_STATE;
                    } else {
                        currentDialog = new LoginDialog(&loginData);
                        positionDialog(currentDialog, screenWidth,
                                                      screenHeight);
                    }
                    break;

                case REGISTER_STATE:
                    logger->log("State: REGISTER");
                    currentDialog = new RegisterDialog(&loginData);
                    positionDialog(currentDialog, screenWidth, screenHeight);
                    break;

                case CHAR_SERVER_STATE:
                    logger->log("State: CHAR_SERVER");

                    if (n_server == 1)
                    {
                        SERVER_INFO *si = *server_info;
                        loginData.hostname = iptostring(si->address);
                        loginData.port = si->port;
                        loginData.updateHost = si->updateHost;
                        state = UPDATE_STATE;
                    }
                    else
                    {
                        int nextState = (options.skipUpdate) ?
                            LOADDATA_STATE : UPDATE_STATE;
                        currentDialog = new ServerSelectDialog(&loginData,
                                                                nextState);
                        positionDialog(currentDialog, screenWidth,
                                                      screenHeight);
                        if (options.chooseDefault
                                || !options.playername.empty())
                        {
                            ((ServerSelectDialog*) currentDialog)->action(
                                gcn::ActionEvent(NULL, "ok"));
                        }
                    }
                    break;
                case CHAR_SELECT_STATE:
                    logger->log("State: CHAR_SELECT");
                    currentDialog = new CharSelectDialog(network, &charInfo,
                                                        (loginData.sex == 0) ?
                                                         GENDER_FEMALE : GENDER_MALE);
                    positionDialog(currentDialog, screenWidth, screenHeight);

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
                    delete setup;
                    delete setupWindow;
                    progressBar = NULL;
                    progressLabel = NULL;
                    currentDialog = NULL;
                    setup = NULL;
                    setupWindow = NULL;
                    login_wallpaper->decRef();
                    login_wallpaper = NULL;

                    logger->log("State: GAME");
                    game = new Game(network);
                    game->logic();
                    delete game;
                    state = EXIT_STATE;
                    break;

                case UPDATE_STATE:
                    // Determine which source to use for the update host
                    if (!options.updateHost.empty())
                        updateHost = options.updateHost;
                    else
                        updateHost = loginData.updateHost;

                    setUpdatesDir();
                    logger->log("State: UPDATE");

                    if (options.skipUpdate) {
                        state = LOADDATA_STATE;
                    } else {
                        currentDialog = new UpdaterWindow(updateHost,
                                                homeDir + "/" + updatesDir);
                        positionDialog(currentDialog, screenWidth,
                                                      screenHeight);
                    }
                    break;

                case ERROR_STATE:
                    logger->log("State: ERROR");
                    currentDialog = new OkDialog(_("Error"), errorMessage);
                    positionDialog(currentDialog, screenWidth, screenHeight);
                    currentDialog->addActionListener(&errorListener);
                    currentDialog = NULL; // OkDialog deletes itself
                    network->disconnect();
                    network->clearHandlers();
                    break;

                case CONNECTING_STATE:
                    logger->log("State: CONNECTING");
                    progressBar->setVisible(true);
                    progressLabel->setCaption(
                            _("Connecting to map server..."));
                    progressLabel->adjustSize();
                    mapLogin(network, &loginData);
                    break;

                case CHAR_CONNECT_STATE:
                    progressBar->setVisible(true);
                    progressLabel->setCaption(
                            _("Connecting to character server..."));
                    progressLabel->adjustSize();
                    charLogin(network, &loginData);
                    break;

                case ACCOUNT_STATE:
                    progressBar->setVisible(true);
                    progressLabel->setCaption(
                            _("Connecting to account server..."));
                    progressLabel->adjustSize();
                    accountLogin(network, &loginData);
                    break;

                default:
                    state = EXIT_STATE;
                    break;
            }
        }
        /*
         * This loop can really stress the CPU, for no reason since it's
         * just constantly redrawing the wallpaper.  Added the following
         * usleep to limit it to 20 FPS during the login sequence
         */
        usleep(50000);
    }

    delete textColour;
#ifdef PACKAGE_VERSION
    delete versionLabel;
#endif
    delete progressBar;
    delete progressLabel;
    delete setup;
    delete setupWindow;

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

void SetupListener::action(const gcn::ActionEvent &event)
{
    Window *window = NULL;

    if (event.getId() == "Setup")
    {
        window = setupWindow;
    }

    if (window)
    {
        window->setVisible(!window->isVisible());
        if (window->isVisible())
        {
            window->requestMoveToTop();
        }
    }
}
