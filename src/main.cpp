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
#include "./sound/sound.h"
#include "./graphic/graphic.h"
#include "resources/resourcemanager.h"
#include <iostream>

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
//unsigned short job, hair, hair_color;
unsigned char stretch_mode, screen_mode;
char *dir;

// new sound-engine /- kth5
#ifndef WIN32
Sound sound;
#endif /* not WIN32 */
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
void init_engine() {
    // Initialize SDL
    //if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
    //    std::cerr << "Could not initialize SDL: " <<
    //        SDL_GetError() << std::endl;
    //    exit(1);
    //}
    //atexit(SDL_Quit);

    // Initialize Allegro
    allegro_init();

    init_log();
    // SDL will send an event for this
    set_close_button_callback(request_exit);

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
            config.setValue("core_version", CORE_VERSION);
            config.setValue("host", "animesites.de");
            config.setValue("port", 6901);
            config.setValue("screen", 1);
            config.setValue("sound", 1);
#ifdef __USE_UNIX98
            char *chatlogFilename = new char[400];
            sprintf(chatlogFilename, "%s/.manaworld/chatlog.txt", userHome);
            config.setValue("chatlog", chatlogFilename);
            delete chatlogFilename;
#else
            config.setValue("chatlog", "chatlog.txt");
#endif
            config.setValue("stretch", 1);
            config.setValue("remember", 1);
            config.setValue("username", "Player");

            config.write(dir);
        }
    }

    config.init(dir);

#ifdef MACOSX
    set_color_depth(32);
#else
    set_color_depth(16);
#endif

    //SDL_WM_SetCaption("The Mana World", NULL);
    set_window_title("The Mana World");

    //screen = SDL_SetVideoMode(800, 600, 16, SDL_SWSURFACE | SDL_ANYFORMAT);
    //if (screen == NULL) {
    //    std::cerr << "Couldn't set 800x600x16 video mode: " <<
    //        SDL_GetError() << std::endl;
    //    exit(1);
    //}

    if (set_gfx_mode((unsigned char)config.getValue("screen", 0), 800, 600,
                0, 0)) {
        error(allegro_error);
    }

    // In SDL these three are all done in the SDL_Init call
    if (install_keyboard()) {
        error("Unable to install keyboard");
    }

    if (install_timer()) {
        error("Unable to install timer");
    }

    if (install_mouse() == -1) {
        error("Unable to install mouse");
    }

    // Buffer creation shouldn't be necessary with SDL
    // Create the graphics context
    graphics = new Graphics();

    ResourceManager *resman = ResourceManager::getInstance();

    login_wallpaper = resman->getImage("graphic/login.bmp");
    Image *playerImg = resman->getImage("graphic/playerset.bmp");
    Image *hairImg = resman->getImage("graphic/hairset.bmp");

    if (!login_wallpaper) error("Couldn't load login.bmp");
    if (!playerImg) error("Couldn't load playerset.bmp");
    if (!hairImg) error("Couldn't load hairset.bmp");

    // Stretch some bitmaps while they haven't been replaced with higher res
    Image *scaledPlayerImg = playerImg->getScaledInstance(
            playerImg->getWidth() * 2, playerImg->getHeight() * 2);
    Image *scaledHairImg = hairImg->getScaledInstance(
            hairImg->getWidth() * 2, hairImg->getHeight() * 2);

    playerset = new Spriteset(scaledPlayerImg, 160, 120);
    hairset = new Spriteset(scaledHairImg, 40, 40);

    init_gui(graphics);
    state = LOGIN;

#ifndef WIN32
    // initialize sound-engine and start playing intro-theme /-kth5
    try {
         if (config.getValue("sound", 0) == 1) {
             //SDL_InitSubSystem(SDL_INIT_AUDIO);
             SDL_Init(SDL_INIT_AUDIO);
             atexit(SDL_Quit);
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
#endif /* not WIN32 */
}

/** Clear the engine */
void exit_engine() {
    config.write(dir);
    delete dir;
    gui_exit();
    ResourceManager::deleteInstance();
    allegro_exit();
}

/** Main */
int main() {
    init_engine();

    while (state != EXIT) {
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
#ifndef WIN32
                sound.stopBgm();
#endif /* not WIN32 */
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
                login_wallpaper->draw(buffer, 0, 0);
                guiGraphics->setTarget(buffer);
                gui->logic();
                gui->draw();
                blit(buffer, screen, 0, 0, 0, 0, 800, 600);
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
END_OF_MAIN();
