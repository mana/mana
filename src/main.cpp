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
#include "./sound/sound.h"
#include "./graphic/graphic.h"

#include <iostream>

#ifdef __USE_UNIX98
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#endif

// Language map can probably be removed after switching to SDL
typedef struct {
    unsigned int code;
    char* name;
} LanguageMap;

LanguageMap languageMap[] = {
    { 0x0813, "BE" },
    { 0x0416, "BR" },
    { 0x1009, "CF" },
    { 0x1009, "CF" },
    { 0x0807, "CH" },
    { 0x0405, "CZ" },
    { 0x0407, "DE" },
    { 0x0406, "DK" },
    { 0x040a, "ES" },
    { 0x040b, "FI" },
    { 0x040c, "FR" },
    { 0x0410, "IT" },
    { 0x0414, "NO" },
    { 0x0415, "PL" },
    { 0x0416, "PT" },
    { 0x0816, "PT" },
    { 0x0419, "RU" },
    { 0x041d, "SE" },
    { 0x041b, "SK" },
    { 0x0424, "SK" },
    { 0x0809, "UK" },
    { 0x0409, "US" },
    { 0, NULL }
};

// Account infos
int account_ID, session_ID1, session_ID2;
char sex, n_server, n_character;
SERVER_INFO *server_info;
PLAYER_INFO *char_info = new PLAYER_INFO;

BITMAP *playerset, *hairset, *login_wallpaper;
DATAFILE *weaponset;

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

void request_exit() {
  state = EXIT;
}

/**
 * Do all initialization stuff
 */
void init_engine() {
#ifdef WIN32
    // After switching to SDL this can probably be removed
    char keyb_buffer[KL_NAMELENGTH + 1];
    unsigned int langID;
    char* code = NULL;
    int running = 1;
    int a;
    if (GetKeyboardLayoutName(keyb_buffer)) {
        //printf("layout name: %s\n", buffer);
        langID = strtol(keyb_buffer, NULL, 16);
        langID &= 0xffff;
        //printf("language id: %x\n", langID);
        for (a = 0; languageMap[a].code != 0; ++a) {
            if (languageMap[a].code == langID) {
                code = languageMap[a].name;
                break;
            }
        }
    }
#endif

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
            // tmw.ini creation
            config.setValue("system", "");
            config.setValue("keyboard", "en");
            config.setValue("language", "");
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
#ifdef WIN32
    // With SDL this part should probably just be removed
    if (code) {
        set_config_string("system", "keyboard", code);
    }
#endif

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
    buffer = create_bitmap(800, 600);
    if (!buffer) {
        error("Not enough memory to create buffer");
    }

    login_wallpaper = load_bitmap("./data/graphic/login.bmp", NULL);
    if (!login_wallpaper) error("Couldn't load login.bmp");

    playerset = load_bitmap("./data/graphic/playerset.bmp", NULL);
    if (!playerset) error("Couldn't load playerset.bmp");

    hairset = load_bitmap("./data/graphic/hairset.bmp", NULL);
    if (!hairset) error("Couldn't load hairset.bmp");

    if (hairset == NULL) {
        error("Unable to load hairset bitmap");
    }

    // TODO: Remove datafile usage
    weaponset = load_datafile("./data/graphic/weapon.dat");
    if (weaponset == NULL) {
        error("Unable to load weaponset datafile");
    }

    // TODO: Remove Allegro config file usage from GUI look
    init_gui(buffer, "./data/Skin/aqua.skin");
    state = LOGIN;
}

/** Clear the engine */
void exit_engine() {
    config.write(dir);
    delete dir;
    gui_exit();
    destroy_bitmap(buffer);
    allegro_exit();
}

/** Main */
int main() {
    init_engine();
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
        
         /* left here to serve as an example ;) 
         SOUND_SID id = sound.loadItem("./data/sound/wavs/level.wav");
         sound.startItem(id, 70);*/
    } catch (const char *err) {
         ok("Sound Engine", err);
         warning(err);
    }
#endif /* not WIN32 */

    while (state != EXIT) {
        switch(state) {
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
            map_start();
            if( state==GAME )
            game();
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
