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

#include <guichan.hpp>
#include "main.h"
#include "game.h"
#include "log.h"
#include "serverinfo.h"
#include "gui/login.h"
#include "gui/char_server.h"
#include "gui/char_select.h"
#include "gui/ok_dialog.h"
#include "gui/updatewindow.h"
#include "sound.h"
#include "graphics.h"
#include "resources/resourcemanager.h"
#include "resources/itemmanager.h"
#include "net/protocol.h"

#include <iostream>
#include <cstdio>
#include <guichan.hpp>
#include <physfs.h>
#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <curl/curl.h>
#include <SDL.h>
#include <SDL_thread.h>
#ifdef USE_OPENGL
#include <SDL_opengl.h>
#endif
#include <SDL_image.h>

#ifdef __USE_UNIX98
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#endif

// Account infos
int account_ID, session_ID1, session_ID2;
char sex, n_server, n_character;
SERVER_INFO *server_info;
PLAYER_INFO *char_info = new PLAYER_INFO;

Spriteset *hairset = NULL, *playerset = NULL;
Image *login_wallpaper = NULL;
Graphics *graphics;

char username[LEN_USERNAME];
char password[LEN_PASSWORD];
int map_address, char_ID;
short map_port;
char map_name[16];
unsigned char state;
unsigned char screen_mode;
int displayFlags, screenW, screenH, bitDepth;
bool useOpenGL = false;
volatile int framesToDraw = 0;

Sound sound;
Music *bgm;

Configuration config;         /**< Xml file configuration reader */
Logger *logger;               /**< Log object */
ItemManager *itemDb;          /**< Item database object */

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
            state = LOGIN;
        }
    }
} mapStartErrorListener;

/**
 * Listener used for responding to init warning.
 */
class InitWarningListener : public gcn::ActionListener {
    void action(const std::string &eventId) {
        if (eventId == "ok") {
            state = LOGIN;
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
    // But, in UNIX we store data in ~/.tmw/
    passwd *pass = getpwuid(geteuid());

    if (pass == NULL || pass->pw_dir == NULL) {
        std::cout << "Couldn't determine the user home directory. Exitting." << std::endl;
        exit(1);
    }

    // Checking if /home/user/.tmw folder exists.

    homeDir = pass->pw_dir;
    homeDir += "/.tmw";
    //sprintf(homeDir, "%s/.tmw", pass->pw_dir);
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
    {
        std::cout << homeDir << " can't be made, but it doesn't exist! Exitting." << std::endl;
        exit(1);
    }
#endif

    // Initialize logger
    logger = new Logger(homeDir + std::string("/tmw.log"));

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
    resman->addToSearchPath(TMW_DATADIR "data", 1);
    // Add zip files to PhysicsFS
    resman->searchAndAddArchives("/", ".zip", true);
    // Updates, these override other files
    resman->searchAndAddArchives("/updates", ".zip", false);

    // Fill configuration with defaults
    config.setValue("host", "animesites.de");
    config.setValue("port", 6901);
    config.setValue("hwaccel", 0);
    config.setValue("opengl", 0);
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
    config.setValue("joytolerance", 10);

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

#ifdef USE_OPENGL
    useOpenGL = (config.getValue("opengl", 0) == 1);
#endif

    displayFlags = SDL_ANYFORMAT;

    if ((int)config.getValue("screen", 0) == 1) {
        displayFlags |= SDL_FULLSCREEN;
    }

    if (useOpenGL) {
        displayFlags |= SDL_OPENGL;
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }
    else {
        if ((int)config.getValue("hwaccel", 0)) {
            logger->log("Attempting to use hardware acceleration.");
            displayFlags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
        }
        else {
            displayFlags |= SDL_SWSURFACE;
        }
    }

    screenW = (int)config.getValue("screenwidth", 800);
    screenH = (int)config.getValue("screenheight", 600);

    SDL_WM_SetIcon(IMG_Load(TMW_DATADIR "data/icons/tmw-icon.png"), NULL);

    SDL_Surface *screen = SDL_SetVideoMode(screenW, screenH, 0, displayFlags);
    if (screen == NULL) {
        std::cerr << "Couldn't set " << screenW << "x" << screenH << "x" <<
            bitDepth << " video mode: " << SDL_GetError() << std::endl;
        exit(1);
    }

    char videoDriverName[64];

    if (SDL_VideoDriverName(videoDriverName, 64)) {
        logger->log("Using video driver: %s", videoDriverName);
    }
    else {
        logger->log("Using video driver: unkown");
    }

    const SDL_VideoInfo *vi = SDL_GetVideoInfo();

    logger->log("Possible to create hardware surfaces: %s",
            ((vi->hw_available) ? "yes" : "no"));
    logger->log("Window manager available: %s",
            ((vi->wm_available) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware blits: %s",
            ((vi->blit_hw) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware colorkey blits: %s",
            ((vi->blit_hw_CC) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware alpha blits: %s",
            ((vi->blit_hw_A) ? "yes" : "no"));
    logger->log("Accelerated software to hardware blits: %s",
            ((vi->blit_sw) ? "yes" : "no"));
    logger->log("Accelerated software to hardware colorkey blits: %s",
            ((vi->blit_sw_CC) ? "yes" : "no"));
    logger->log("Accelerated software to hardware alpha blits: %s",
            ((vi->blit_sw_A) ? "yes" : "no"));
    logger->log("Accelerated color fills: %s",
            ((vi->blit_fill) ? "yes" : "no"));
    logger->log("Available video memory: %d", vi->video_mem);

    //vfmt Pixel format of the video device

    // Initialize item manager
    itemDb = new ItemManager();

    // Create the graphics context
    graphics = new Graphics(screen);

    login_wallpaper = resman->getImage(
            "graphics/images/login_wallpaper.png");
    Image *playerImg = resman->getImage(
            "graphics/sprites/player_male_base.png");
    Image *hairImg = resman->getImage(
            "graphics/sprites/player_male_hair.png");

    if (!login_wallpaper) logger->error("Couldn't load login_wallpaper.png");
    if (!playerImg) logger->error("Couldn't load player_male_base.png");
    if (!hairImg) logger->error("Couldn't load player_male_hair.png");

    playerset = new Spriteset(playerImg, 64, 64);
    hairset = new Spriteset(hairImg, 40, 40);

    gui = new Gui(graphics);
    state = UPDATE; /**< Initial game state */

    // Initialize sound engine
    try {
        if (config.getValue("sound", 0) == 1) {
            sound.init();
        }
        sound.setSfxVolume((int)config.getValue("sfxVolume", 100));
        sound.setMusicVolume((int)config.getValue("musicVolume", 60));
    }
    catch (const char *err) {
        state = ERROR;
        new OkDialog("Sound Engine", err, &initWarningListener);
        logger->log("Warning: %s", err);
    }

    // Set frame counter when using fps limit
    int fpsLimit = (int)config.getValue("fpslimit", 0);
    if (fpsLimit)
    {
        if (fpsLimit < 20) fpsLimit = 20;
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
    delete itemDb;

    // Shutdown libxml
    xmlCleanupParser();

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

/** Main */
int main(int argc, char *argv[])
{
    UpdaterWindow *uw;

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

    while (state != EXIT)
    {
        // Handle SDL events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state = EXIT;
                    break;
            }

            guiInput->pushInput(event);
        }

        switch (state) {
            case LOGIN:
                logger->log("State: LOGIN");
                login();
                break;
            case CHAR_SERVER:
                logger->log("State: CHAR_SERVER");
                char_server();
                break;
            case CHAR_SELECT:
                logger->log("State: CHAR_SELECT");
                charSelect();
                break;
            case GAME:
                sound.fadeOutMusic(1000);

                login_wallpaper->decRef();
                login_wallpaper = NULL;

                logger->log("State: GAME");
                try {
                    map_start();
                    game();
                }
                catch (const char* err) {
                    state = ERROR;
                    new OkDialog("Error", err, &mapStartErrorListener);
                }
                break;
            case ERROR:
                // Redraw GUI
                graphics->drawImage(login_wallpaper, 0 ,0);
                gui->logic();
                gui->draw();
                graphics->updateScreen();
                break;
            case UPDATE:
                sound.playMusic(TMW_DATADIR "data/music/Magick - Real.ogg");
                uw = new UpdaterWindow();
                uw->updateData();
                delete uw;
                ResourceManager::getInstance()->
                    searchAndAddArchives("/updates", ".zip", 0);
                break;
            default:
                state = EXIT;
                break;
        }
    }
    logger->log("State: EXIT");
    exit_engine();
    PHYSFS_deinit();
    return 0;
}
