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
#include "map.h"
#include "being.h"
#include "log.h"
#include "./gui/chat.h"
#include "./gui/gui.h"
#include "./gui/inventory.h"
#include "./gui/shop.h"
#include "./gui/npc.h"
#include "./gui/stats.h"
#include "./gui/ok_dialog.h"
#include "./graphic/graphic.h"
#include "./sound/sound.h"
#include <SDL.h>

char map_path[480];

unsigned short dest_x, dest_y, src_x, src_y;
unsigned int player_x, player_y;
bool refresh_beings = false;
unsigned char keyb_state;
volatile int tick_time;
volatile bool action_time = false;
int current_npc, server_tick;
extern unsigned char screen_mode;
int fps = 0, frame = 0;

OkDialog *deathNotice = NULL;

#define MAX_TIME 10000

class DeatchNoticeListener : public gcn::ActionListener {
    public:
        void action(const std::string &eventId) {
            WFIFOW(0) = net_w_value(0x00b2);
            WFIFOB(2) = 0;
            WFIFOSET(3);
            deathNotice = NULL;
        }
} deathNoticeListener;

/**
 * Finite states machine to keep track of player walking status (2 steps
 * linear prediction)
 *
 *  0 = Standing
 *  1 = Walking without confirm packet
 *  2 = Walking with confirm
 */
char walk_status = 0;


Uint32 refresh_time(Uint32 interval, void *param)
{
    tick_time++;
    if (tick_time == MAX_TIME) tick_time = 0;
    return interval;
}

/**
 * Lets u only trigger an action every other second
 * tmp. counts fps
 */
Uint32 second(Uint32 interval, void *param)
{
    action_time = true;
    fps = frame;
    frame = 0;
    return interval;
}

short get_elapsed_time(short start_time) {
    if (start_time <= tick_time) {
        return (tick_time - start_time) * 10;
    }
    else {
        return (tick_time + (MAX_TIME - start_time)) * 10;
    }
}

void game() {
    do_init();
    Engine *engine = new Engine();

    while (state != EXIT)
    {
        do_input();
        gui->logic();
        engine->draw();
        graphics->updateScreen();
        do_parse();
        flush();
    }

    delete engine;
    close_session();
}

void do_init() {
    if (!tiledMap.load(map_path)) {
        error("Could not find map file");
    }

    sound.startBgm("./data/sound/Mods/somemp.xm", -1);

    // Initialize timers
    tick_time = 0;

    // The SDL way
    SDL_AddTimer(10, refresh_time, NULL);
    SDL_AddTimer(1000, second, NULL);

    // Initialize beings
    empty();
    player_node = new Being();
    player_node->id = account_ID;
    player_node->x = x;
    player_node->y = y;
    player_node->speed = 150;
    player_node->hair_color = char_info->hair_color;
    player_node->hair_style = char_info->hair_style;
    if (char_info->weapon == 11) {
        char_info->weapon = 1;
    }
    player_node->weapon = char_info->weapon;
    add_node(player_node);

    remove("./docs/packet.list");
}

void do_exit() {
}

void do_input()
{
    // Events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        bool used = false;

        // For discontinuous keys
        if (event.type == SDL_KEYDOWN)
        {
            SDL_keysym keysym = event.key.keysym;

            if ((keysym.sym == SDLK_F5) && action_time)
            {
                if (player_node->action == STAND)
                    action(2, 0);
                else if (player_node->action == SIT)
                    action(3, 0);
                action_time = false;
            }

            // Emotions, Skill dialog
            if (keysym.mod & KMOD_ALT && action_time)
            {
                if (player_node->emotion == 0)
                {
                    unsigned char emotion = 0;
                    if (keysym.sym == SDLK_1) emotion = 1;
                    else if (keysym.sym == SDLK_2) emotion = 2;
                    else if (keysym.sym == SDLK_3) emotion = 3;
                    else if (keysym.sym == SDLK_4) emotion = 4;
                    else if (keysym.sym == SDLK_5) emotion = 5;
                    else if (keysym.sym == SDLK_6) emotion = 6;
                    else if (keysym.sym == SDLK_7) emotion = 7;
                    else if (keysym.sym == SDLK_8) emotion = 8;
                    else if (keysym.sym == SDLK_9) emotion = 9;
                    else if (keysym.sym == SDLK_0) emotion = 10;
                    if (emotion != 0) {
                        WFIFOW(0) = net_w_value(0x00bf);
                        WFIFOB(2) = emotion;
                        WFIFOSET(3);
                        action_time = false;
                        used = true;
                    }
                }

            }

            if (keysym.mod & KMOD_ALT) {
                if (keysym.sym == SDLK_i) {
                    inventoryWindow->setVisible(!inventoryWindow->isVisible());
                    used = true;
                }
                else if (keysym.sym == SDLK_s) {
                    statsWindow->setVisible(!statsWindow->isVisible());
                    used = true;
                }
                else if (keysym.sym == SDLK_k) {
                    skillDialog->setVisible(!skillDialog->isVisible());
                    used = true;
                }
                else if (keysym.sym == SDLK_c) {
                    setupWindow->setVisible(true);
                    used = true;
                }
            }

            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                state = EXIT;
            }

            if (keysym.sym == SDLK_f)
            {
                if (keysym.mod & KMOD_CTRL)
                {
                    // Workaround for Win and else
                    #if __USE_UNIX98
                        SDL_WM_ToggleFullScreen(screen);
                        if ((int)config.getValue("screen", 0) == 0) {
                            config.setValue("screen", 1);
                        }
                        else {
                            config.setValue("screen", 0);
                        }
                    #else
                        int displayFlags = 0;
                        if ((int)config.getValue("screen", 0) == 0) {
                        displayFlags |= SDL_FULLSCREEN;
                            config.setValue("screen", 1);
                        }
                        else {
                            config.setValue("screen", 0);
                        }
                        if ((int)config.getValue("hwaccel", 0)) {
                            displayFlags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
                        }
                        else {
                            displayFlags |= SDL_SWSURFACE;
                        }
                        screen = SDL_SetVideoMode(800, 600, 32, displayFlags);
                    #endif
                }
            }
        } // End key down

        if (event.button.type == SDL_MOUSEBUTTONDOWN)
        {
            if (event.button.button == 3)
            {
                // We click the right button
                // NPC Call
                int npc_x = event.motion.x / 32 + camera_x;
                int npc_y = event.motion.y / 32 + camera_y;
                int id = find_npc(npc_x, npc_y);
                if (id != 0)
                {
                    WFIFOW(0) = net_w_value(0x0090);
                    WFIFOL(2) = net_l_value(id);
                    WFIFOB(6) = 0;
                    WFIFOSET(7);
                }

            }

        }

        // Push input to GUI when not used
        if (!used) {
            guiInput->pushInput(event);
        }

    } // End while


    // Get the state of the keyboard keys
    Uint8* keys;
    keys = SDL_GetKeyState(NULL);

    if (walk_status == 0 && player_node->action != DEAD)
    {
        int x = player_node->x;
        int y = player_node->y;

        if (keys[SDLK_UP] || keys[SDLK_KP8])
        {
            // UP
            if (tiledMap.getWalk(x, y - 1) != 0)
            {
                walk(x, y-1, NORTH);
                walk_status = 1;
                src_x = x;
                src_y = y;
                player_node->action = WALK;
                player_node->walk_time = tick_time;
                player_node->y = y - 1;
                player_node->direction = NORTH;
            }
            else player_node->direction = NORTH;
        }
        else if (keys[SDLK_DOWN] || keys[SDLK_KP2])
        {
            // Down
            if (tiledMap.getWalk(x, y + 1) != 0)
            {
                walk(x, y + 1, SOUTH);
                walk_status = 1;
                src_x = x;
                src_y = y;
                player_node->action = WALK;
                player_node->walk_time = tick_time;
                player_node->y = y + 1;
                player_node->direction = SOUTH;
            }
            else player_node->direction = SOUTH;
        }
        else if (keys[SDLK_LEFT] || keys[SDLK_KP4])
        {
            if (tiledMap.getWalk(x - 1, y) != 0) {
                walk(x - 1, y, WEST);
                walk_status = 1;
                src_x = x;
                src_y = y;
                player_node->action = WALK;
                player_node->walk_time = tick_time;
                player_node->x = x - 1;
                player_node->direction = WEST;
            }
            else player_node->direction = WEST;
        }
        else if (keys[SDLK_RIGHT] || keys[SDLK_KP6])
        {
            if (tiledMap.getWalk(x + 1, y) != 0) {
                walk(x + 1, y, EAST);
                walk_status = 1;
                src_x = x;
                src_y = y;
                player_node->action = WALK;
                player_node->walk_time = tick_time;
                player_node->x = x + 1;
                player_node->direction = EAST;
            }
            else player_node->direction = EAST;
        }

        if (player_node->action == STAND)
        {
            if (keys[SDLK_LCTRL])
            {
                player_node->action = ATTACK;
                attack(player_node->x,
                        player_node->y,
                        player_node->direction);
                player_node->walk_time = tick_time;
            }
        }

    } // End if alive

}

int get_packet_length(short id) {
    int len = get_length(id);
    if (len == -1)len = RFIFOW(2);
    return len;
}

void do_parse() {
    unsigned short id;
    char *temp;
    char direction;
    Being *being = NULL;
    int len, n_items;

    // We need at least 2 bytes to identify a packet
    if (in_size >= 2) {
        // Check if the received packet is complete
        while (in_size >= (len = get_packet_length(id = RFIFOW(0)))) {
            // Add infos to log file and dump the latest received packet
            char pkt_nfo[60];
            sprintf(pkt_nfo,"In-buffer size: %i Packet id: %x Packet length: %i",in_size,RFIFOW(0),len);
            /*
            log_hex("Packet", "Packet_ID", RFIFOW(0));
            log_int("Packet", "Packet_length", get_length(RFIFOW(0)));
            log_int("Packet", "Packet_in_size", RFIFOW(2));
            log_int("Packet", "In_size", in_size);
            FILE *file = fopen("packet.dump", "wb");
            for(int i=0;i<len;i++) {
                fprintf(file, "%x|%i|%c ", RFIFOB(i), RFIFOB(i), RFIFOB(i));
                if((i+1)%10==0)fprintf(file, "\n");
            }
            fclose(file);
            */
#ifdef DEBUG
            FILE *file = fopen("./docs/packet.list", "a");
            fprintf(file, "%x\n", RFIFOW(0));
            fclose(file);
#endif 
            // Parse packet based on their id
            switch(id) {
                // Received speech
                case 0x008d:
                    temp = (char *)malloc(RFIFOW(2)-7);
                    memset(temp, '\0', RFIFOW(2)-7);
                    memcpy(temp, RFIFOP(8), RFIFOW(2)-8);
                    being = find_node(RFIFOL(4));
                    if (being != NULL) {
                        if (being->speech != NULL) {
                            free(being->speech);
                            being->speech = NULL;
                            being->speech_time = 0;
                        }
                        being->speech = temp;
                        being->speech_time = SPEECH_TIME;
                        being->speech_color = 0;//makecol(255, 255, 255);
                        chatBox->chat_log(being->speech, BY_OTHER);
                    }
                    break;
                case 0x008e:
                case 0x009a:
                    if (RFIFOW(2)>4) {
                        if(player_node->speech!=NULL) {
                            free(player_node->speech);
                            player_node->speech = NULL;
                        }

                        player_node->speech = (char *)malloc(RFIFOW(2)-3);
                        memset(player_node->speech, '\0', RFIFOW(2)-3);
                        memcpy(player_node->speech, RFIFOP(4), RFIFOW(2)-4);  // receive 1 byte less than expected, server might be sending garbage instead of '\0' /-kth5

                        player_node->speech_time = SPEECH_TIME;
                        player_node->speech_color = 0;//makecol(255, 255, 255);

                        if(id==0x008e) {
                            chatBox->chat_log(player_node->speech, BY_PLAYER);
                        }
                        else {
                            chatBox->chat_log(player_node->speech, BY_GM);
                        }
                    }
                    break;
                    // Success to walk request
                case 0x0087:
                    if (walk_status == 1) {
                        if ((unsigned int)(RFIFOL(2)) > (unsigned int)(server_tick)) {
                            walk_status = 2;
                            server_tick = RFIFOL(2);
                        }
                    }
                    break;
                    // Add new being / stop monster
                case 0x0078:
                    if (find_node(RFIFOL(2)) == NULL) {
                        being = new Being();
                        being->id = RFIFOL(2);
                        being->speed = RFIFOW(6);
                        if(being->speed==0) {
                            being->speed = 150; // Else division by 0 when calculating frame
                        }
                        being->job = RFIFOW(14);
                        being->clearPath();
                        being->x = get_x(RFIFOP(46));
                        being->y = get_y(RFIFOP(46));
                        being->direction = get_direction(RFIFOP(46));
                        being->hair_color = RFIFOW(28);
                        being->hair_style = RFIFOW(16);
                        add_node(being);
                    }
                    else {
                        if (being) {
                            being->clearPath();
                            being->x = get_x(RFIFOP(46));
                            being->y = get_y(RFIFOP(46));
                            being->direction = get_direction(RFIFOP(46));
                            being->frame = 0;
                            being->walk_time = tick_time;
                            being->action = STAND;
                        }
                    }
                    break;
                    // Remove a being
                case 0x0080:
                    being = find_node(RFIFOL(2));
                    if (being != NULL) {
                        if(RFIFOB(6)==1) { // Death
                            if(being->job>110) {
                                being->action = MONSTER_DEAD;
                                being->frame = 0;
                                being->walk_time = tick_time;
                            }
                            //else being->action = DEAD;
                            //remove_node(RFIFOL(2));
                        }
                        else remove_node(RFIFOL(2));
                    }
                    break;
                    // Player moving
                case 0x01d8:
                case 0x01d9:
                    being = find_node(RFIFOL(2));
                    if (being == NULL) {
                        being = new Being();
                        being->id = RFIFOL(2);
                        being->job = RFIFOW(14);
                        being->x = get_x(RFIFOP(46));
                        being->y = get_y(RFIFOP(46));
                        being->direction = get_direction(RFIFOP(46));
                        add_node(being);
                        being->walk_time = tick_time;
                        being->frame = 0;
                        being->speed = RFIFOW(6);
                        being->hair_color = RFIFOW(28);
                        being->hair_style = RFIFOW(16);
                    }
                    break;
                    // Monster moving
                case 0x007b:
                    //case 0x01da:
                    being = find_node(RFIFOL(2));
                    if(being==NULL) {
                        being = new Being();
                        being->action = STAND;
                        being->x = get_src_x(RFIFOP(50));
                        being->y = get_src_y(RFIFOP(50));
                        being->direction = 0;
                        being->id = RFIFOL(2);
                        being->speed = RFIFOW(6);
                        being->job = RFIFOW(14);
                        add_node(being);
                    }
                    being->setPath(calculate_path(get_src_x(RFIFOP(50)),
                            get_src_y(RFIFOP(50)),get_dest_x(RFIFOP(50)),
                            get_dest_y(RFIFOP(50))));
                    break;
                    // Being moving
                case 0x01da:
                    being = find_node(RFIFOL(2));
                    if (being == NULL) {
                        being = new Being();
                        being->id = RFIFOL(2);
                        being->job = RFIFOW(14);
                        being->x = get_src_x(RFIFOP(50));
                        being->y = get_src_y(RFIFOP(50));
                        add_node(being);
                    }
                    if (being->action != WALK) {
                        direction = being->direction;
                        being->action = WALK;
                        if (get_dest_x(RFIFOP(50)) > being->x)
                            direction = EAST;
                        else if (get_dest_x(RFIFOP(50)) < being->x)
                            direction = WEST;
                        else if (get_dest_y(RFIFOP(50)) > being->y)
                            direction = SOUTH;
                        else if (get_dest_y(RFIFOP(50)) < being->y)
                            direction = NORTH;
                        else being->action = STAND;
                        if (being->action == WALK)
                            being->walk_time = tick_time;
                        being->x = get_dest_x(RFIFOP(50));
                        being->y = get_dest_y(RFIFOP(50));
                        being->direction = direction;
                    }
                    break;
                    // NPC dialog
                case 0x00b4:
                    npcTextDialog->addText(RFIFOP(8));
                    npcListDialog->setVisible(false);
                    npcTextDialog->setVisible(true);
                    break;
                    // Get the items
                case 0x01ee:
                    for (int loop = 0; loop < (RFIFOW(2) - 4) / 18; loop++) {
                        inventoryWindow->addItem(RFIFOW(4 + loop * 18),
                                RFIFOW(4 + loop * 18 + 2), RFIFOW(4 + loop * 18 + 6));
                    }
                    break;
                    // Can I use the item?
                case 0x00a8:
                    if (RFIFOB(6) == 0) {
                        chatBox->chat_log("Failed to use item", BY_OTHER);
                    } else {
                        inventoryWindow->changeQuantity(RFIFOW(2), RFIFOW(4));
                    }
                    break;
                    // Warp
                case 0x0091:
                    memset(map_path, '\0', 480);
                    strcat(map_path, "./data/map/");
                    strncat(map_path, RFIFOP(2), 497 - strlen(map_path));
                    if (tiledMap.load(map_path)) {
                        Being *temp;
                        temp = new Being();
                        memcpy(temp, player_node, sizeof(Being));
                        empty();
                        /*player_node = new Being();
                        player_node->job = 0;
                        player_node->action = STAND;
                        player_node->frame = 0;
                        player_node->speed = 150;
                        player_node->id = account_ID;*/
                        add_node(temp);
                        player_node = temp;
                        player_node->action = STAND;
                        player_node->frame = 0;
                        player_node->x = RFIFOW(18);
                        player_node->y = RFIFOW(20);
                        walk_status = 0;
                        // Send "map loaded"
                        WFIFOW(0) = net_w_value(0x007d);
                        WFIFOSET(2);
                        while (out_size > 0) flush();
                    }
                    else {
                        error("Could not find map file");
                    }
                    break;
                    // Skill ...
                case 0x011a:
                    break;
                case 0x01a4:
                    break;
                    // Action failed (ex. sit because you have not reached the right level)
                case 0x0110:
                    CHATSKILL action;
                    action.skill   = RFIFOW(2);
                    action.bskill  = RFIFOW(4);
                    action.unused  = RFIFOW(6);
                    action.success = RFIFOB(8);
                    action.reason  = RFIFOB(9);
                    if(action.success != SKILL_FAILED &&
                            action.bskill == BSKILL_EMOTE ) {
                        printf("Action: %d/%d", action.bskill, action.success);
                    }
                    chatBox->chat_log(action);
                    break;
                    // Update stat values
                case 0x00b0:
                    switch(RFIFOW(2)) {
                        //case 0x0000:
                        //    player_node->speed;
                        //    break;
                        case 0x0005:
                            char_info->hp = RFIFOW(4);
                            break;
                        case 0x0006:
                            char_info->max_hp = RFIFOW(4);
                            break;
                        case 0x0007:
                            char_info->sp = RFIFOW(4);
                            break;
                        case 0x0008:
                            char_info->max_sp = RFIFOW(4);
                            break;
                        case 0x000b:
                            char_info->lv = RFIFOW(4);
                            break;
                        case 0x000c:
                            char_info->skill_point = RFIFOW(4);
                            skillDialog->setPoints(char_info->skill_point);
                            break;
                        case 0x0037:
                            char_info->job_lv = RFIFOW(4);
                            break;
                        //case 0x0009:
                            // try to get the stats point to attribute...
                            // FIXME : Wrong or misplaced...
                            //char_info->statsPointsToAttribute = RFIFOW(4);
                            //break;
                    }
                    statusWindow->update();
                    if (char_info->hp == 0 && deathNotice == NULL) {
                        deathNotice = new OkDialog("Message",
                                "You're now dead, press ok to restart",
                                &deathNoticeListener);
                        //remove_node(char_info->id);
                        being->action = DEAD;
                    }
                    break;
                    // Stop walking
                case 0x0088:  // Disabled because giving some problems
                    //if (being = find_node(RFIFOL(2))) {
                    //    if (being->id!=player_node->id) {
                    //        char ids[20];
                    //        sprintf(ids,"%i",RFIFOL(2));
                    //        alert(ids,"","","","",0,0);
                    //        being->action = STAND;
                    //        being->frame = 0;
                    //        set_coordinates(being->coordinates, RFIFOW(6), RFIFOW(8), get_direction(being->coordinates));
                    //    }
                    //}
                    //break;
                    // Damage, sit, stand up
                case 0x008a:
                    switch (RFIFOB(26)) {
                        case 0: // Damage
                            being = find_node(RFIFOL(6));
                            if (being != NULL) {
                                if (being->speech != NULL) {
                                    free(being->speech);
                                    being->speech = NULL;
                                    //being->speech_time = SPEECH_TIME;
                                }
                                being->speech = (char *)malloc(5);
                                memset(being->speech, '\0', 5);
                                if (RFIFOW(22) == 0) {
                                    sprintf(being->speech, "miss");
                                    being->speech_color = 0;//makecol(255, 255, 0);
                                } else {
                                    sprintf(being->speech, "%i", RFIFOW(22));
                                    if (being->id != player_node->id) {
                                        being->speech_color = 0;//makecol(0,0,255);
                                    }
                                    else {
                                        being->speech_color = 0;//makecol(255,0,0);
                                    }
                                }
                                being->speech_time = SPEECH_TIME;
                                if (RFIFOL(2) != player_node->id) { // buggy
                                    being = find_node(RFIFOL(2));
                                    if (being != NULL) {
                                        if (being->job<10) {
                                            being->action = ATTACK;
                                        }
                                        else {
                                            being->action = MONSTER_ATTACK;
                                        }
                                        being->frame = 0;
                                    }
                                }
                            }
                            break;
                        case 2: // Sit
                        case 3: // Stand up
                            being = find_node(RFIFOL(2));
                            if (being != NULL) {
                                being->frame = 0;
                                if (RFIFOB(26) == 2) {
                                    being->action = SIT;
                                    walk_status = 0;
                                }
                                else if (RFIFOB(26) == 3) {
                                    being->action = STAND;
                                }
                            }
                            break;
                    }
                    break;
                    // Status change
                case 0x00b1:
                    switch (RFIFOW(2)) {
                        case 1:
                            char_info->xp = RFIFOL(4);
                            break;
                        case 2:
                            char_info->job_xp = RFIFOL(4);
                            break;
                        case 20:
                            char_info->gp = RFIFOL(4);
                            break;
                        case 0x0016:
                            char_info->xpForNextLevel = RFIFOL(4);
                            break;
                        case 0x0017:
                            char_info->jobXpForNextLevel = RFIFOL(4);
                            break;
                    }
                    break;
                    // Level up
                case 0x019b:
                    if (RFIFOL(2) == player_node->id) {
                        SOUND_SID sound_id = sound.loadItem(
                                "./data/sound/wavs/level.ogg");
                        sound.startItem(sound_id, 64);
                        sound.clearCache();
                    }
                    break;
                    // Emotion
                case 0x00c0:
                    being = find_node(RFIFOL(2));
                    if(being) {
                        being->emotion = RFIFOB(6);
                        being->emotion_time = EMOTION_TIME;
                    }
                    break;
                    // Update skill values
                case 0x0141:
                    switch(RFIFOL(2)) {
                        case 0x000d:
                            char_info->STR = RFIFOL(6) + RFIFOL(10); // Base + Bonus
                            break;
                        case 0x000e:
                            char_info->AGI = RFIFOL(6) + RFIFOL(10);
                            break;
                        case 0x000f:
                            char_info->VIT = RFIFOL(6) + RFIFOL(10);
                            break;
                        case 0x0010:
                            char_info->INT = RFIFOL(6) + RFIFOL(10);
                            break;
                        case 0x0011:
                            char_info->DEX = RFIFOL(6) + RFIFOL(10);
                            break;
                        case 0x0012:
                            char_info->LUK = RFIFOL(6) + RFIFOL(10);
                            break;
                    }
                    break;
                    // Buy/Sell dialog
                case 0x00c4:
                    buyDialog->setVisible(false);
                    sellDialog->setVisible(false);
                    buySellDialog->setVisible(true);
                    current_npc = RFIFOL(2);
                    break;
                    // Buy dialog
                case 0x00c6:
                    n_items = (len - 4) / 11;
                    buyDialog->reset();
                    buyDialog->setMoney(char_info->gp);
                    buyDialog->setVisible(true);
                    for (int k = 0; k < n_items; k++) {
                        buyDialog->addItem(RFIFOW(4 + 11 * k + 9), RFIFOL(4 + 11 * k));
                    }
                    break;
                    // Sell dialog
                case 0x00c7:
                    n_items = (len - 4) / 10;
                    if (n_items > 0) {
                        sellDialog->reset();
                        sellDialog->setVisible(true);
                        for (int k = 0; k < n_items; k++) {
                            sellDialog->addItem(
                                    RFIFOW(4 + 10 * k), RFIFOL(4 + 10 * k + 2));
                        }
                    }
                    else {
                        chatBox->chat_log("Nothing to sell", BY_SERVER);
                    }
                    break;
                    // Answer to buy
                case 0x00ca:
                    if (RFIFOB(2) == 0)
                        chatBox->chat_log("Thanks for buying", BY_SERVER);
                    else
                        chatBox->chat_log("Unable to buy", BY_SERVER);
                    break;
                    // Answer to sell
                case 0x00cb:
                    if (RFIFOB(2) == 0)
                        chatBox->chat_log("Thanks for selling", BY_SERVER);
                    else
                        chatBox->chat_log("Unable to sell", BY_SERVER);
                    break;
                    // Add item to inventory after you bought it
                case 0x00a0:
                    if (RFIFOB(22) > 0)
                        chatBox->chat_log("Unable to pick up item", BY_SERVER);
                    else
                        inventoryWindow->addItem(RFIFOW(2), RFIFOW(6), RFIFOW(4));
                    break;
                    // Decrease quantity of an item in inventory
                case 0x00af:
                    printf("sell %i\n", -RFIFOW(4));
                    inventoryWindow->increaseQuantity(RFIFOW(2), -RFIFOW(4));
                    break;
                    // Use an item
                case 0x01c8:
                    inventoryWindow->changeQuantity(RFIFOW(2), RFIFOW(10));
                    break;
                    // ??
                case 0x0119:
                    break;
                    // Skill list TAG
                case 0x010f:
                {
                    int n_skills = (len - 4) / 37;
                    for (int k = 0; k < n_skills; k++)
                    {
                        if (RFIFOW(4 + k * 37 + 6) != 0 ||
                                RFIFOB(4 + k * 37 + 36)!=0)
                        {
                            int skillId = RFIFOW(4 + k * 37);
                            if (skillDialog->hasSkill(skillId)) {
                                skillDialog->setSkill(skillId,
                                        RFIFOW(4 + k * 37 + 6),
                                        RFIFOW(4 + k * 37 + 36));
                            }
                            else {
                                skillDialog->addSkill(
                                        RFIFOW(4 + k * 37),
                                        RFIFOW(4 + k * 37 + 6),
                                        RFIFOW(4 + k * 37 + 8));
                            }
                        }
                    }
                } break;
                    // MVP experience
                case 0x010b:
                    break;
                    // Display MVP payer
                case 0x010c:
                    chatBox->chat_log("MVP player", BY_SERVER);
                    break;
                    // Item drop
                case 0x009e:
                    WFIFOW(0) = net_w_value(0x009f);
                    WFIFOL(2) = net_l_value(RFIFOL(2));
                    WFIFOSET(6);
                    // To be fixed or you will pick up again what you drop
                    break;
                    // Next button in NPC dialog
                case 0x00b5:
                    strcpy(npc_button, "Next");
                    current_npc = RFIFOL(2);
                    break;
                    // Close button in NPC dialog
                case 0x00b6:
                    strcpy(npc_button, "Close");
                    current_npc = RFIFOL(2);
                    break;
                    // List in NPC dialog
                case 0x00b7:
                    current_npc = RFIFOL(4);
                    // Hammerbear: Second argument here shouldn't be neccesary,
                    //  instead make sure the string is \0 terminated.
                    //parse_items(RFIFOP(8), RFIFOW(2));
                    npcListDialog->parseItems(RFIFOP(8));
                    npcListDialog->setVisible(true);
                    break;
                    // Look change
                case 0x00c3:
                    // Change hair color
                    if (RFIFOB(6) == 6) {
                        being = find_node(RFIFOL(2));
                        being->hair_color = RFIFOB(7);
                    } else if (RFIFOB(6) == 1) {
                        being = find_node(RFIFOL(2));
                        being->hair_style = RFIFOB(7);
                    }
                    break;
                case 0x00a4:
                    for (int i = 0; i < (RFIFOW(2) - 4) / 20; i++)
                        inventoryWindow->addItem(RFIFOW(4 + 20 * i), RFIFOW(6 + 20 * i), 1);
                    break;

                    // Manage non implemented packets
                default:
                    //printf("%x\n",id);
                    //alert(pkt_nfo,"","","","",0,0);
                    break;
            }

            RFIFOSKIP(len);
        }
    }
}
