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
#include "gui/char_server.h"
#include "gui/char_select.h"
#include "gui/inventory.h"
#include "gui/ok_dialog.h"
#include "sound.h"
#include "graphic/graphic.h"
#include "resources/resourcemanager.h"

#include <iostream>
#include <guichan.hpp>
#include <SDL.h>
#include <libxml/xmlversion.h>

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

Spriteset *hairset, *playerset;
Image *login_wallpaper;
Graphics *graphics;

char username[LEN_USERNAME];
char password[LEN_PASSWORD];
int map_address, char_ID;
short map_port;
char map_name[16];
unsigned char state;
unsigned short x, y;
unsigned char direction;
unsigned char screen_mode;
char *dir;

Sound sound;

// ini file configuration reader
Configuration config;


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

void request_exit() {
    state = EXIT;
}

/**
 * Do all initialization stuff
 */
void init_engine()
{
    // Initialize libxml2 and check for potential ABI mismatches between
    // compiled version and the shared library actually used.
    LIBXML_TEST_VERSION

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << "Could not initialize SDL: " <<
            SDL_GetError() << std::endl;
        exit(1);
    }
    atexit(SDL_Quit);

    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    init_log();

    dir = new char[400];
    strcpy(dir, "");

#ifndef __USE_UNIX98
    // WIN32 and others
    strcpy(dir, "tmw.ini");
#else
    // UNIX
    char *userHome;
    char *name = getlogin();
    passwd *pass;

    if (name != NULL) {
        pass = getpwnam(name);
    }
    else {
        pass = getpwuid(geteuid());
    }

    if (pass == NULL) {
        printf("Couldn't determine the user home directory. Exitting.\n");
        exit(1);
    }

    userHome = pass->pw_dir;

    // Checking if homeuser/.manaworld folder exists.
    sprintf(dir, "%s/.manaworld", userHome);
    if ((mkdir(dir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) &&
            (errno != EEXIST))
    {
        printf("%s can't be made... And doesn't exist ! Exitting ...", dir);
        exit(1);
    }
    sprintf(dir, "%s/.manaworld/tmw.ini", userHome);
#endif

    // Checking if the tmw.ini file exists... otherwise creates it with
    // default options !
    FILE *tmwFile = 0;
    tmwFile = fopen(dir, "r");

    // If we can't read it, it doesn't exist !
    if (tmwFile == NULL) {
        // We reopen the file in write mode and we create it
        printf("No file : %s\n, Creating Default Options...\n", dir);
        tmwFile = fopen(dir, "wt");
        if (tmwFile == NULL) {
            printf("Can't create %s file. Using Defaults.\n", dir);
        }
        else {
            fclose(tmwFile);
            // Fill tmw.ini with defaults
            config.setValue("host", "animesites.de");
            config.setValue("port", 6901);
            config.setValue("hwaccel", 0);
            config.setValue("screen", 0);
            config.setValue("sound", 1);
#ifdef __USE_UNIX98
            char *chatlogFilename = new char[400];
            sprintf(chatlogFilename, "%s/.manaworld/chatlog.txt", userHome);
            config.setValue("chatlog", chatlogFilename);
            delete chatlogFilename;
#else
            config.setValue("chatlog", "chatlog.txt");
#endif
            config.setValue("remember", 1);
            config.setValue("username", "Player");

            config.write(dir);
        }
    }

    config.init(dir);

    SDL_WM_SetCaption("The Mana World", NULL);


    int displayFlags = SDL_ANYFORMAT;

    if ((int)config.getValue("screen", 0) == 1) {
        displayFlags |= SDL_FULLSCREEN;
    }
#ifndef USE_OPENGL
    if ((int)config.getValue("hwaccel", 0)) {
        std::cout << "Attempting to use hardware acceleration.\n";
        displayFlags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
    }
    else {
        displayFlags |= SDL_SWSURFACE;
    }
#else
    displayFlags |= SDL_OPENGL;
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif

    screen = SDL_SetVideoMode(800, 600, 32, displayFlags);
    if (screen == NULL) {
        std::cerr << "Couldn't set 800x600x32 video mode: " <<
            SDL_GetError() << std::endl;
        exit(1);
    }

#ifdef USE_OPENGL
    // Setup OpenGL
    glViewport(0, 0, 800, 600);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
#endif

#ifdef __DEBUG
    const SDL_VideoInfo *vi = SDL_GetVideoInfo();
    std::cout << "It is " << ((vi->hw_available) ? "" : "not") <<
        " possible to create hardware surfaces.\n";
    std::cout << "There is " << ((vi->wm_available) ? "a" : "no") <<
        " window manager available.\n";
    std::cout << "Hardware to hardware blits are " <<
        ((vi->blit_hw) ? "" : "not") << " accelerated.\n";
    std::cout << "Hardware to hardware colorkey blits are " <<
        ((vi->blit_hw_CC) ? "" : "not") << " accelerated.\n";
    std::cout << "Hardware to hardware alpha blits are " <<
        ((vi->blit_hw_A) ? "" : "not") << " accelerated.\n";
    std::cout << "Software to hardware blits are " <<
        ((vi->blit_sw) ? "" : "not") << " accelerated.\n";
    std::cout << "Software to hardware colorkey blits are " <<
        ((vi->blit_sw_CC) ? "" : "not") << " accelerated.\n";
    std::cout << "Software to hardware alpha blits are " <<
        ((vi->blit_sw_A) ? "" : "not") << " accelerated.\n";
    std::cout << "Color fills are " <<
        ((vi->blit_fill) ? "" : "not") << " accelerated.\n";
    std::cout << "Available video memory: " << vi->video_mem << "\n";
#endif

    //vfmt Pixel format of the video device

    // Create the graphics context
    graphics = new Graphics();

    ResourceManager *resman = ResourceManager::getInstance();

    login_wallpaper = resman->getImage(
            "core/graphics/images/login_wallpaper.png");
    Image *playerImg = resman->getImage(
            "core/graphics/sprites/player_male_base.png");
    Image *hairImg = resman->getImage(
            "core/graphics/sprites/player_male_hair.png");

    if (!login_wallpaper) error("Couldn't load login_wallpaper.png");
    if (!playerImg) error("Couldn't load player_male_base.png");
    if (!hairImg) error("Couldn't load player_male_hair.png");

    playerset = new Spriteset(playerImg, 160, 120);
    hairset = new Spriteset(hairImg, 40, 40);

    init_gui(graphics);
    state = LOGIN;

    // initialize sound-engine and start playing intro-theme /-kth5
    try {
         if (config.getValue("sound", 0) == 1) {
             SDL_InitSubSystem(SDL_INIT_AUDIO);
             sound.init(32, 20);
         }
         sound.setVolume(64);
        
         // left here to serve as an example ;)
         /*
         SOUND_SID id = sound.loadItem("./data/sound/wavs/level.ogg");
         sound.startItem(id, 70);
         */
    }
    catch (const char *err) {
        state = ERROR;
        new OkDialog("Sound Engine", err, &initWarningListener);
        warning(err);
    }
}

/** Clear the engine */
void exit_engine() {
    config.write(dir);
    delete[] dir;
    gui_exit();
    ResourceManager::deleteInstance();
}

/** Main */
int main(int argc, char *argv[]) {
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
                status("LOGIN");
                login();
                break;
            case CHAR_SERVER:
                status("CHAR_SERVER");
                char_server();
                break;
            case CHAR_SELECT:
                status("CHAR_SELECT");
                charSelect();
                break;
            case GAME:
                sound.stopBgm();
                status("GAME");
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
                login_wallpaper->draw(screen, 0, 0);
                gui->logic();
                gui->draw();
                guiGraphics->updateScreen();
                break;
            default:
                state = EXIT;
                break;
        }
    }
    status("EXIT");
    exit_engine();
    return 0;
}
