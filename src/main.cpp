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
    class SetupListener : public gcn::ActionListener
    {
    public:
        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);
    } listener;
}

static const int defaultSfxVolume = 100;
static const int defaultMusicVolume = 60;

Graphics *graphics;
Game *game = 0;

State state = STATE_START;
std::string errorMessage;

Sound sound;
Music *bgm;

Configuration config;         /**< XML file configuration reader */
Configuration branding;       /**< XML branding information reader */
Logger *logger;               /**< Log object */
KeyboardConfig keyboard;

LoginData loginData;
LockedArray<LocalPlayer*> charInfo(MAX_CHARACTER_COUNT);

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
    std::string screenshotDir;

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
            errorMessage = strprintf(_("Invalid update host: %s"), updateHost.c_str());
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

/**
 * Initializes the home directory. On UNIX and FreeBSD, ~/.mana is used. On
 * Windows and other systems we use the current working directory.
 */
static void initHomeDir(const Options &options)
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
static void initConfiguration(const Options &options)
{
    // Fill configuration with defaults
    logger->log("Initializing configuration...");
    std::string defaultHost = branding.getValue("defaultServer",
        "server.themanaworld.org");
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
       logger->log("Can't create %s. Using defaults.", configPath.c_str());
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
        logger->error(strprintf("Could not initialize SDL: %s",
                      SDL_GetError()));
    }
    atexit(SDL_Quit);

    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    SDL_WM_SetCaption(branding.getValue("appName", "The Mana World").c_str(),
                      NULL);

    ResourceManager *resman = ResourceManager::getInstance();

    if (!resman->setWriteDir(homeDir)) {
        logger->error(strprintf("%s couldn't be set as home directory! "
                                "Exiting.", homeDir.c_str()));
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
    icon = IMG_Load(resman->getPath(branding.getValue("appIcon", "data/icons/mana.png")).c_str());
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
        << _("mana") << endl << endl
        << _("Options:") << endl
        << _("  -C --config-file    : Configuration file to use") << endl
        << _("  -d --data           : Directory to load game data from") << endl
        << _("  -D --default        : Choose default character server and "
                                     "character") << endl
        << _("  -h --help           : Display this help") << endl
        << _("  -S --home-dir       : Directory to use as home directory") << endl
        << _("  -i --screenshot-dir : Directory to store screenshots") << endl
        << _("  -H --update-host    : Use this update host") << endl
        << _("  -P --password       : Login with this password") << endl
        << _("  -c --character      : Login with this character") << endl
        << _("  -p --port           : Login server port") << endl
        << _("  -s --server         : Login server name or IP") << endl
        << _("  -u --skip-update    : Skip the update downloads") << endl
        << _("  -U --username       : Login with this username") << endl
#ifdef USE_OPENGL
        << _("  -O --no-opengl      : Disable OpenGL for this session") << endl
#endif
        << _("  -v --version        : Display the version") << endl;
}

static void printVersion()
{
    std::cout << strprintf("The Mana World %s", FULL_VERSION) << std::endl;
}

static void parseOptions(int argc, char *argv[], Options &options)
{
    const char *optstring = "hvud:U:P:Dc:s:p:C:H:S:Oi:";

    const struct option long_options[] = {
        { "config-file",    required_argument, 0, 'C' },
        { "data",           required_argument, 0, 'd' },
        { "default",        no_argument,       0, 'D' },
        { "password",       required_argument, 0, 'P' },
        { "character",      required_argument, 0, 'c' },
        { "help",           no_argument,       0, 'h' },
        { "home-dir",       required_argument, 0, 'S' },
        { "update-host",    required_argument, 0, 'H' },
        { "port",           required_argument, 0, 'p' },
        { "server",         required_argument, 0, 's' },
        { "skip-update",    no_argument,       0, 'u' },
        { "username",       required_argument, 0, 'U' },
        { "no-opengl",      no_argument,       0, 'O' },
        { "version",        no_argument,       0, 'v' },
        { "screenshot-dir", required_argument, 0, 'i' },
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
            case 'i':
                options.screenshotDir = optarg;
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

class ErrorListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent &event)
    {
        state = STATE_CHOOSE_SERVER;
    }
} errorListener;

class AccountListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent &event)
    {
        state = STATE_CHAR_SELECT;
    }
} accountListener;

class LoginListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent &event)
    {
        state = STATE_LOGIN;
    }
} loginListener;

} // namespace

const std::string &getHomeDirectory()
{
    return homeDir;
}

// TODO Find some nice place for these functions
static void accountLogin(LoginData *loginData)
{
    logger->log("Username is %s", loginData->username.c_str());

    Net::getCharHandler()->setCharInfo(&charInfo);

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
        config.setValue("username", loginData->username);
    }
    config.setValue("remember", loginData->remember);
}

extern "C" char const *_nl_locale_name_default(void);

static void initInternationalization()
{
#if ENABLE_NLS
#ifdef WIN32
    putenv(("LANG=" + std::string(_nl_locale_name_default())).c_str());
    // mingw doesn't like LOCALEDIR to be defined for some reason
    bindtextdomain("mana", "translations/");
#else
    bindtextdomain("mana", LOCALEDIR);
#endif
    setlocale(LC_MESSAGES, "");
    bind_textdomain_codeset("mana", "UTF-8");
    textdomain("mana");
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

    setScreenshotDir(options.screenshotDir);

    // Configure logger
    logger = new Logger;
    logger->setLogFile(homeDir + std::string("/mana.log"));

    // Log the mana version
    logger->log("Mana %s", FULL_VERSION);

    initConfiguration(options);
    logger->setLogToStandardOut(config.getValue("logToStandardOut", 0));

    initEngine(options);

    // Needs to be created in main, as the updater uses it
    guiPalette = new Palette;

    Window *currentDialog = NULL;
    QuitDialog* quitDialog = NULL;
    setupWindow = new Setup;

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());
    Desktop *desktop = 0;
    ProgressBar *progressBar = 0;
    Button *setupButton = 0;

    sound.playMusic(branding.getValue("loginMusic", "Magick - Real.ogg"));

    // Initialize default server
    ServerInfo currentServer;
    currentServer.hostname = options.serverName;
    currentServer.port = options.serverPort;
    loginData.username = options.username;
    loginData.password = options.password;
    loginData.remember = config.getValue("remember", 0);
    loginData.registerLogin = false;

    if (currentServer.hostname.empty()) {
        currentServer.hostname = branding.getValue("defaultServer",
                                            "server.themanaworld.org").c_str();
    }
    if (options.serverPort == 0) {
        currentServer.port = (short) branding.getValue("defaultPort",
                                                          DEFAULT_PORT);
    }
    if (loginData.username.empty() && loginData.remember) {
        loginData.username = config.getValue("username", "");
    }

    if (state != STATE_ERROR)
        state = STATE_CHOOSE_SERVER;
    State oldstate = STATE_START; // We start with a status change

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
                    break;
            }

            guiInput->pushInput(event);
        }

        if (Net::getGeneralHandler())
        {
            Net::getGeneralHandler()->flushNetwork();
        }
        gui->logic();

        if (progressBar && progressBar->isVisible())
        {
            progressBar->setProgress(progressBar->getProgress() + 0.005f);
            if (progressBar->getProgress() == 1.0f)
                progressBar->setProgress(0.0f);
        }

        gui->draw();
        graphics->updateScreen();

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
            Net::getCharHandler()->setCharInfo(&charInfo);
            state = STATE_LOGIN;
        }
        else if (state == STATE_WORLD_SELECT && oldstate == STATE_UPDATE)
        {
            if (Net::getLoginHandler()->getWorlds().size() < 2)
            {
                state = STATE_LOGIN;
            }
        }
        else if (oldstate == STATE_START || oldstate == STATE_GAME)
        {
            desktop = new Desktop;
            top->add(desktop);
            progressBar = new ProgressBar(0.0f, 100, 20,
                                          gcn::Color(168, 116, 31));
            progressBar->setSmoothProgress(false);
            Label *progressLabel = new Label;
            top->add(progressBar, 5, top->getHeight() - 5 -
                     progressBar->getHeight());
            top->add(progressLabel, 15 + progressBar->getWidth(),
                     progressBar->getY() + 4);
            progressBar->setVisible(false);
            setupButton = new Button(_("Setup"), "Setup", &listener);
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
                    logger->log("State: CHOOSE SERVER");

                    // Allow changing this using a server choice dialog
                    // We show the dialog box only if the command-line
                    // options weren't set.
                    if (options.serverName.empty() && options.serverPort == 0) {
                        // Don't allow an alpha opacity
                        // lower than the default value
                        SkinLoader::instance()->setMinimumOpacity(0.8f);

                        currentDialog = new ServerDialog(&currentServer,
                                                         homeDir);
                    } else {
                        state = STATE_CONNECT_SERVER;

                        // Reset options so that cancelling or connect
                        // timeout will show the server dialog.
                        options.serverName.clear();
                        options.serverPort = 0;
                    }
                    break;

                case STATE_CONNECT_SERVER:
                    logger->log("State: CONNECT SERVER");
                    currentDialog = new ConnectionDialog(STATE_SWITCH_SERVER);
                    break;

                case STATE_LOGIN:
                    logger->log("State: LOGIN");
                    // Don't allow an alpha opacity
                    // lower than the default value
                    SkinLoader::instance()->setMinimumOpacity(0.8f);

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

                case STATE_LOGIN_ATTEMPT:
                    logger->log("State: LOGIN ATTEMPT");
                    accountLogin(&loginData);
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
                    currentDialog = new ConnectionDialog(STATE_WORLD_SELECT);
                    break;

                case STATE_UPDATE:

                    // Determine which source to use for the update host
                    if (!options.updateHost.empty())
                        updateHost = options.updateHost;
                    else
                        updateHost = loginData.updateHost;
                    setUpdatesDir();

                    if (options.skipUpdate)
                    {
                        state = STATE_LOAD_DATA;
                    }
                    else
                    {
                        logger->log("State: UPDATE");
                        currentDialog = new UpdaterWindow(updateHost,
                                homeDir + "/" + updatesDir);
                    }
                    break;

                case STATE_LOAD_DATA:
                    logger->log("State: LOAD DATA");

                    // If another data path has been set,
                    // we don't load any other files...
                    if (options.dataPath.empty())
                    {
                        // Load the updates downloaded so far...
                        loadUpdates();


                        // Also add customdata directory
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
                    Net::getCharHandler()->getCharacters();
                    break;

                case STATE_CHAR_SELECT:
                    logger->log("State: CHAR SELECT");
                    // Don't allow an alpha opacity
                    // lower than the default value
                    SkinLoader::instance()->setMinimumOpacity(0.8f);

                    currentDialog =
                        new CharSelectDialog(&charInfo, &loginData);

                    if (((CharSelectDialog*) currentDialog)->
                            selectByName(options.character)) {
                        ((CharSelectDialog*) currentDialog)->chooseSelected();
                    } else {
                        ((CharSelectDialog*) currentDialog)->selectByName(
                            config.getValue("lastCharacter", ""));
                    }

                    break;

                case STATE_CONNECT_GAME:
                    logger->log("State: CONNECT GAME");

                    Net::getGameHandler()->connect();
                    currentDialog = new ConnectionDialog(STATE_SWITCH_CHARACTER);
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
                    game->logic();
                    delete game;
                    game = 0;

                    if (state == STATE_GAME)
                        state = STATE_EXIT;

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
                    currentDialog = new ConnectionDialog(STATE_LOGIN);
                    break;

                case STATE_REGISTER:
                    logger->log("State: REGISTER");
                    currentDialog = new RegisterDialog(&loginData);
                    break;

                case STATE_REGISTER_ATTEMPT:
                    logger->log("Username is %s", loginData.username.c_str());

                    Net::getCharHandler()->setCharInfo(&charInfo);
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

                    Net::getCharHandler()->getCharacters();
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
                    Net::getGeneralHandler()->unload();
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
