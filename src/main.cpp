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
#include <SDL_image.h>

#include <guichan/sdl/sdlinput.hpp>

#include <libxml/parser.h>

#ifdef __USE_UNIX98
#include <cerrno>
#include <sys/stat.h>
#endif

#include "configuration.h"
#include "game.h"
#include "graphics.h"
#include "log.h"
#ifdef USE_OPENGL
#include "openglgraphics.h"
#endif
#include "sound.h"

#include "graphic/spriteset.h"

#include "gui/char_server.h"
#include "gui/char_select.h"
#include "gui/connection.h"
#include "gui/error.h"
#include "gui/gui.h"
#include "gui/login.h"
#include "gui/ok_dialog.h"
#include "gui/updatewindow.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

// Account infos
int account_ID, session_ID1, session_ID2;
char sex, n_server, n_character;
PLAYER_INFO **char_info;
PLAYER_INFO *player_info;

Spriteset *hairset = NULL, *playerset = NULL;
Graphics *graphics;

int map_address, char_ID;
short map_port;
unsigned char state;
std::string errorMessage;
unsigned char screen_mode;
volatile int framesToDraw = 0;

Sound sound;
Music *bgm;

Configuration config;         /**< Xml file configuration reader */
Logger *logger;               /**< Log object */

/**
 * Allows the next frame to be drawn (part of framerate limiting)
 */
Uint32 nextFrame(Uint32 interval, void *param)
{
    if (framesToDraw < 10)
    {
        framesToDraw++;
    }
    return interval;
}

/**
 * Listener used for responding to map start error dialog.
 */
class MapStartErrorListener : public gcn::ActionListener {
    void action(const std::string &eventId) {
        if (eventId == "ok") {
            state = LOGIN_STATE;
        }
    }
} mapStartErrorListener;

/**
 * Listener used for responding to init warning.
 */
class InitWarningListener : public gcn::ActionListener {
    void action(const std::string &eventId) {
        if (eventId == "ok") {
            state = LOGIN_STATE;
        }
    }
} initWarningListener;

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

    std::string homeDir = "";
#ifndef __USE_UNIX98
    // In Windows and other systems we currently store data next to executable.
    homeDir = ".";
#else
    homeDir = std::string(PHYSFS_getUserDir()) + "/.tmw";

    // Checking if /home/user/.tmw folder exists.
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
    {
        std::cout << homeDir << " can't be made, but it doesn't exist! Exitting." << std::endl;
        exit(1);
    }
#endif

    // Set log file
    logger->setLogFile(homeDir + std::string("/tmw.log"));

    ResourceManager *resman = ResourceManager::getInstance();

    if (!resman->setWriteDir(homeDir)) {
        std::cout << homeDir << " couldn't be set as home directory! Exitting." << std::endl;
        exit(1);
    }

    // Add the user's homedir to PhysicsFS search path
    resman->addToSearchPath(homeDir, false);
    // Creating and checking the updates folder existence and rights.
    if (!resman->isDirectory("/updates")) {
        if (!resman->mkdir("/updates")) {
        std::cout << homeDir << "/updates can't be made, but it doesn't exist! Exitting." << std::endl;
        exit(1);
        }
    }

    // Add the main data directory to our PhysicsFS search path
    resman->addToSearchPath("data", true);
    resman->addToSearchPath(TMW_DATADIR "data", true);
    // Add zip files to PhysicsFS
    resman->searchAndAddArchives("/", ".zip", true);
    // Updates, these override other files
    resman->searchAndAddArchives("/updates", ".zip", false);

    // Fill configuration with defaults
    config.setValue("host", "animesites.de");
    config.setValue("port", 6901);
    config.setValue("hwaccel", 0);
    #if defined __APPLE__ || defined WIN32 || defined USE_OPENGL
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
    config.setValue("joytolerance", 100);

    // Checking if the configuration file exists... otherwise creates it with
    // default options !
    FILE *tmwFile = 0;
    std::string configPath = homeDir + "/config.xml";
    tmwFile = fopen(configPath.c_str(), "r");

    // If we can't read it, it doesn't exist !
    if (tmwFile == NULL) {
        // We reopen the file in write mode and we create it
        tmwFile = fopen(configPath.c_str(), "wt");
        if (tmwFile == NULL) {
            std::cout << "Can't create " << configPath << ". Using Defaults." << std::endl;
        }
        else {
            fclose(tmwFile);
            config.init(configPath);
        }
    }
    else {
        fclose(tmwFile);
        config.init(configPath);
    }

    SDL_WM_SetCaption("The Mana World", NULL);
    SDL_WM_SetIcon(IMG_Load(TMW_DATADIR "data/icons/tmw-icon.png"), NULL);

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
        std::cerr << "Couldn't set " << width << "x" << height << "x" <<
            bpp << " video mode: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Initialize for drawing
    graphics->_beginDraw();

    playerset = resman->createSpriteset(
            "graphics/sprites/player_male_base.png", 64, 64);
    hairset = resman->createSpriteset(
            "graphics/sprites/player_male_hair.png", 40, 40);

    if (!playerset) logger->error("Couldn't load player spriteset!");
    if (!hairset) logger->error("Couldn't load hair spriteset!");

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
        new OkDialog("Sound Engine", err, &initWarningListener);
        logger->log("Warning: %s", err);
    }

    // Set frame counter when using fps limit
    int fpsLimit = (int)config.getValue("fpslimit", 0);
    if (fpsLimit)
    {
        if (fpsLimit < 10) fpsLimit = 10;
        if (fpsLimit > 200) fpsLimit = 200;
        SDL_AddTimer(1000 / fpsLimit, nextFrame, NULL);
    }
}

/** Clear the engine */
void exit_engine()
{
    config.write();
    delete gui;
    delete graphics;
    delete hairset;
    delete playerset;

    // Shutdown libxml
    xmlCleanupParser();
    
    // Shutdown sound
    sound.close();

    ResourceManager::deleteInstance();
    delete logger;
}

/** Check to see if a file exists */
int exists(const std::string &file)
{
    FILE *fp = NULL;
    fp = fopen(file.c_str(), "r");
    if (!fp)
    {
        return false;
    }
    else {
        fclose(fp);
        return true;
    }
}

struct Options
{
    Options():printHelp(false),skipUpdate(false) {};

    bool printHelp;
    bool skipUpdate;
};

void printHelp()
{
    std::cout << "tmw" << std::endl << std::endl;
    std::cout << "Options: " << std::endl;
    std::cout << "  -h --help       : Display this help" << std::endl;
    std::cout << "     --skipupdate : Skip the update process" << std::endl;
}

void parseOptions(int argc, char *argv[], Options &options)
{
    const char *optstring = "h";

    const struct option long_options[] = {
        { "help",       no_argument, 0, 'h' },
        { "skipupdate", no_argument, 0, 'u' },
        0
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
        }
    }
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

    // Initialize libxml2 and check for potential ABI mismatches between
    // compiled version and the shared library actually used.
    xmlInitParser();
    LIBXML_TEST_VERSION;

    // Redirect libxml errors to /dev/null
    FILE *nullFile = fopen("/dev/null", "w");
    xmlSetGenericErrorFunc(nullFile, NULL);

    // Initialize PhysicsFS
    PHYSFS_init(argv[0]);

    init_engine();

    SDL_Event event;

    if (options.skipUpdate && state != ERROR_STATE)
    {
        state = LOGIN_STATE;
    }

    unsigned int oldstate = !state; // We start with a status change.
    Window *currentDialog = NULL;
    void (*inputHandler)(SDL_KeyboardEvent*) = NULL;

    Image *login_wallpaper = NULL;

    sound.playMusic(TMW_DATADIR "data/music/Magick - Real.ogg");

    while (state != EXIT_STATE)
    {
        // Handle SDL events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state = EXIT_STATE;
                    break;

                case SDL_KEYDOWN:
                    if (inputHandler) {
                        inputHandler(&event.key);
                    }
                    break;
            }

            guiInput->pushInput(event);
        }

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

        if (state != oldstate) {
            if (oldstate == UPDATE_STATE) {
                ResourceManager::getInstance()->
                    searchAndAddArchives("/updates", ".zip", 0);
            }

            oldstate = state;

            if (currentDialog) {
                delete currentDialog;
            }

            switch (state) {
                case LOGIN_STATE:
                    logger->log("State: LOGIN");
                    currentDialog = new LoginDialog();
                    inputHandler = loginInputHandler;
                    break;
                case CHAR_SERVER_STATE:
                    logger->log("State: CHAR_SERVER");
                    currentDialog = new ServerSelectDialog();
                    inputHandler = charServerInputHandler;
                    break;
                case CHAR_SELECT_STATE:
                    logger->log("State: CHAR_SELECT");
                    currentDialog = new CharSelectDialog();
                    inputHandler = charSelectInputHandler;
                    break;
                case GAME_STATE:
                    sound.fadeOutMusic(1000);

                    currentDialog = NULL;
                    inputHandler = NULL;
                    login_wallpaper->decRef();
                    login_wallpaper = NULL;

                    logger->log("State: GAME");
                    game();
                    break;
                case UPDATE_STATE:
                    logger->log("State: UPDATE");
                    currentDialog = new UpdaterWindow();
                    inputHandler = updateInputHandler;
                    break;
                case ERROR_STATE:
                    logger->log("State: ERROR");
                    currentDialog = new ErrorDialog(errorMessage);
                    inputHandler = errorInputHandler;
                    break;
                case CONNECTING_STATE:
                    logger->log("State: CONNECTING");
                    currentDialog = new ConnectionDialog();
                    inputHandler = connectionInputHandler;
                    break;
                default:
                    state = EXIT_STATE;
                    break;
            }
        }
    }

    if (nullFile)
    {
        fclose(nullFile);
    }
    logger->log("State: EXIT");
    exit_engine();
    PHYSFS_deinit();
    return 0;
}
