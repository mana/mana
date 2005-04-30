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

#include "being.h"
#include "engine.h"
#include "floor_item.h"
#include "graphics.h"
#include "log.h"
#include "main.h"
#include "map.h"
#include "sound.h"
#include "gui/chat.h"
#include "gui/gui.h"
#include "gui/inventory.h"
#include "gui/shop.h"
#include "gui/npc.h"
#include "gui/stats.h"
#include "gui/ok_dialog.h"
#include "net/protocol.h"
#include "resources/mapreader.h"
#include <SDL.h>
#include <sstream>

char map_path[480];

bool refresh_beings = false;
unsigned char keyb_state;
volatile int tick_time;
volatile bool action_time = false;
int server_tick;
int fps = 0, frame = 0, current_npc = 0;
bool displayPathToMouse = false;
int startX = 0, startY = 0;
int gameTime = 0;

OkDialog *deathNotice = NULL;

#define EMOTION_TIME 150
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

int get_elapsed_time(int start_time)
{
    if (start_time <= tick_time) {
        return (tick_time - start_time) * 10;
    }
    else {
        return (tick_time + (MAX_TIME - start_time)) * 10;
    }
}

void game()
{
    do_init();
    Engine *engine = new Engine();

    gameTime = tick_time;

    while (state != EXIT)
    {
        while (get_elapsed_time(gameTime) > 0)
        {
            do_input();
            engine->logic();
            gameTime++;
        }
        gameTime = tick_time;

        gui->logic();
        engine->draw();
        graphics->updateScreen();
        do_parse();
        flush();
    }

    delete engine;
    close_session();
}

void do_init()
{
    std::string path(map_path);
    std::string pathDir = path.substr(0, path.rfind("."));

    // Try .tmx map file
    pathDir.insert(pathDir.size(), ".tmx");
    tiledMap = Map::load(pathDir);


    if (!tiledMap)
    {
        // Try .tmx.gz map file
        pathDir.insert(pathDir.size(), ".gz");
        tiledMap = Map::load(pathDir);

        if (!tiledMap)
        {
            logger.error("Could not find map file!");
        }
    }

    // Start playing background music
    //sound.startBgm("./data/sound/Mods/somemp.xm", -1);

    // Initialize timers
    tick_time = 0;
    SDL_AddTimer(10, refresh_time, NULL);
    SDL_AddTimer(1000, second, NULL);

    // Initialize beings
    player_node = new Being();
    player_node->id = account_ID;
    player_node->x = startX;
    player_node->y = startY;
    player_node->speed = 150;
    player_node->setHairColor(char_info->hair_color);
    player_node->setHairStyle(char_info->hair_style);

    if (char_info->weapon == 11) {
        char_info->weapon = 2;
    }

    player_node->weapon = char_info->weapon;

    add_node(player_node);

    remove("packet.list");
}

void do_exit()
{
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

            if (keysym.sym == SDLK_RETURN)
            {
                if (!chatWindow->isFocused())
                {
                    chatWindow->requestFocus();
                    used = true;
                }
            }

            if ((keysym.sym == SDLK_F5) && action_time)
            {
                if (player_node->action == STAND)
                    action(2, 0);
                else if (player_node->action == SIT)
                    action(3, 0);
                action_time = false;
            }
            else if ((keysym.sym == SDLK_F6))
            {
                displayPathToMouse = !displayPathToMouse;
            }
            else if ((keysym.sym == SDLK_F7))
            {
                sound.playSfx("sfx/fist-swish.ogg");
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
                else if (keysym.sym == SDLK_n) {
                    newSkillWindow->setVisible(!newSkillWindow->isVisible());
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
                else if (keysym.sym == SDLK_e) {
                    equipmentWindow->setVisible(!equipmentWindow->isVisible());
                    used = true;
                }
            }

            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                state = EXIT;
            }

            if (keysym.sym == SDLK_g)
            {
                // Get the item code
                if (!chatWindow->isFocused())
                {
                    used = true;
                    int id = 0;
                    id = find_floor_item_by_cor(player_node->x, player_node->y);
                    if (id != 0)
                    {
                        WFIFOW(0) = net_w_value(0x009f);
                        WFIFOL(2) = net_l_value(id);
                        WFIFOSET(6);
                    }
                    else {
                        switch (player_node->direction) {
                            case NORTH:
                                id = find_floor_item_by_cor(player_node->x, player_node->y-1);
                                break;
                            case SOUTH:
                                id = find_floor_item_by_cor(player_node->x, player_node->y+1);
                                break;
                            case WEST:
                                id = find_floor_item_by_cor(player_node->x-1, player_node->y);
                                break;
                            case EAST:
                                id = find_floor_item_by_cor(player_node->x+1, player_node->y);
                                break;
                            case NW:
                                id = find_floor_item_by_cor(player_node->x-1, player_node->y-1);
                                break;
                            case NE:
                                id = find_floor_item_by_cor(player_node->x+1, player_node->y-1);
                                break;
                            case SW:
                                id = find_floor_item_by_cor(player_node->x-1, player_node->y+1);
                                break;
                            case SE:
                                id = find_floor_item_by_cor(player_node->x+1, player_node->y+1);
                                break;
                            default: 
                                break;
                        }
                        WFIFOW(0) = net_w_value(0x009f);
                        WFIFOL(2) = net_l_value(id);
                        WFIFOSET(6);
                    }
                }
            }
        } // End key down
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            int mx = event.button.x / 32 + camera_x;
            int my = event.button.y / 32 + camera_y;

            if (event.button.button == 3)
            {
                Being *target = findNode(mx, my);
                if (target) {
                    if (target->isNpc()) {
                        // Check if no conflicting npc window is open
                        if (current_npc == 0)
                        {
                            WFIFOW(0) = net_w_value(0x0090);
                            WFIFOL(2) = net_l_value(target->id);
                            WFIFOB(6) = 0;
                            WFIFOSET(7);
                            current_npc = target->id;
                        }
                    }
                    else if (target->isMonster()) {
                        if (player_node->action == STAND) {
                            if (my > player_node->y)
                                player_node->direction = SOUTH;
                            else if (my < player_node->y)
                                player_node->direction = NORTH;
                            else if (mx > player_node->x)
                                player_node->direction = EAST;
                            else if (mx < player_node->x)
                                player_node->direction = WEST;
                            player_node->action = ATTACK;
                            action(0, target->id);
                            player_node->walk_time = tick_time;
                            if (player_node->weapon == 2)
                                sound.playSfx("sfx/bow_shoot_1.ogg");
                            else
                                sound.playSfx("sfx/fist-swish.ogg");
                        }
                    }
                    else if (target->isPlayer()) {
                        // Begin a trade
                        WFIFOW(0) = net_w_value(0x00e4);
                        WFIFOL(2) = net_l_value(target->id);
                        WFIFOSET(6);
                    }
                }
            }
        }
        else if (event.type == SDL_QUIT)
        {
            state = EXIT;
        }

        // Push input to GUI when not used
        if (!used) {
            guiInput->pushInput(event);
        }

    } // End while


    // Get the state of the keyboard keys
    Uint8* keys;
    keys = SDL_GetKeyState(NULL);
    int xDirection = 0;
    int yDirection = 0;
    int Direction = DIR_NONE;

    if (player_node->action != DEAD && current_npc == 0)
    {
        int x = player_node->x;
        int y = player_node->y;

        // Translate pressed keys to movement
        if (keys[SDLK_UP] || keys[SDLK_KP8])
        {
            yDirection = -1;
        }
        if (keys[SDLK_DOWN] || keys[SDLK_KP2])
        {
            yDirection = 1;
        }
        if (keys[SDLK_LEFT] || keys[SDLK_KP4])
        {
            xDirection = -1;
        }
        if (keys[SDLK_RIGHT] || keys[SDLK_KP6])
        {
            xDirection = 1;
        }
        if (keys[SDLK_KP1]) // Bottom Left
        {
            xDirection = -1;
            yDirection = 1;
        }
        if (keys[SDLK_KP3]) // Bottom Right
        {
            xDirection = 1;
            yDirection = 1;
        }
        if (keys[SDLK_KP7]) // Top Left
        {
            xDirection = -1;
            yDirection = -1;
        }
        if (keys[SDLK_KP9]) // Top Right
        {
            xDirection = 1;
            yDirection = -1;
        }

        // Allow keyboard control to interrupt an existing path
        if ((xDirection != 0 || yDirection != 0) &&
                player_node->action == WALK)
        {
            player_node->setDestination(player_node->x, player_node->y);
        }

        if (player_node->action != WALK)
        {
            // Translate movement to direction
            if (xDirection == 1 && yDirection == 0) // Right
            {
                Direction = EAST;
            }
            if (xDirection == -1 && yDirection == 0) // Left
            {
                Direction = WEST;
            }
            if (xDirection == 0 && yDirection == -1) // Up
            {
                Direction = NORTH;
            }
            if (xDirection == 0 && yDirection == 1) // Down
            {
                Direction = SOUTH;
            }
            if (xDirection == 1 && yDirection == 1) // Bottom-Right
            {
                Direction = SE;
            }
            if (xDirection == -1 && yDirection == -1) // Top-left
            {
                Direction = NW;
            }
            if (xDirection == 1 && yDirection == -1) // Top-Right
            {
                Direction = NE;
            }
            if (xDirection == -1 && yDirection == 1) // Bottom-Left
            {
                Direction = SW;
            }

            // Prevent skipping corners over colliding tiles
            if (xDirection != 0 && tiledMap->tileCollides(x + xDirection, y)) {
                xDirection = 0;
            }
            if (yDirection != 0 && tiledMap->tileCollides(x, y + yDirection)) {
                yDirection = 0;
            }

            // Choose a straight direction when diagonal target is blocked
            if (yDirection != 0 && xDirection != 0 &&
                    !tiledMap->getWalk(x + xDirection, y + yDirection)) {
                xDirection = 0;
            }

            // Walk to where the player can actually go
            if ((xDirection != 0 || yDirection != 0) &&
                    tiledMap->getWalk(x + xDirection, y + yDirection))
            {
                walk(x + xDirection, y + yDirection, Direction);
                player_node->setDestination(x + xDirection, y + yDirection);
            }
            else if (Direction != DIR_NONE)
            {
                // Update the player direction to where he wants to walk
                // Warning: Not communicated to the server yet
                player_node->direction = Direction;
            }
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

                if (player_node->weapon == 2) {
                    sound.playSfx("sfx/bow_shoot_1.ogg");
                }
                else {
                    sound.playSfx("sfx/fist-swish.ogg");
                }
            }
        }
    }
}

int get_packet_length(short id)
{
    int len = get_length(id);
    if (len == -1) len = RFIFOW(2);
    return len;
}

void do_parse()
{
    unsigned short id;
    char *temp;
    Being *being = NULL;
    FloorItem *floorItem = NULL;
    int len, n_items;

    // We need at least 2 bytes to identify a packet
    if (in_size >= 2) {
        // Check if the received packet is complete
        while (in_size >= (len = get_packet_length(id = RFIFOW(0)))) {
            // Add infos to log file and dump the latest received packet
            char pkt_nfo[60];
            sprintf(pkt_nfo,"In-buffer size: %i Packet id: %x Packet length: %i", in_size, RFIFOW(0), len);
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
#ifdef __DEBUG
            FILE *file = fopen("packet.list", "a");
            fprintf(file, "%x\n", RFIFOW(0));
            fclose(file);
#endif
            // Parse packet based on their id
            switch (id) {
                case SMSG_LOGIN_SUCCESS:
                    // Connected to game server succesfully, set spawn point
                    player_node->x = get_x(RFIFOP(6));
                    player_node->y = get_y(RFIFOP(6));
                    break;

                // Received speech from being
                case SMSG_BEING_CHAT:
                    being = findNode(RFIFOL(4));
                    if (being != NULL) {
                        int length = RFIFOW(2) - 8;
                        temp = (char*)malloc(length + 1);
                        temp[length] = '\0';
                        memcpy(temp, RFIFOP(8), length);
                        std::string msg = std::string(temp);
                        msg.erase(0, msg.find(" : ", 0) + 3);

                        being->setSpeech(msg, SPEECH_TIME);
                        chatWindow->chat_log(temp, BY_OTHER);

                        free(temp);
                    }
                    break;

                case SMSG_MY_BEING_CHAT:
                case SMSG_GM_CHAT:
                    if (RFIFOW(2) > 4) {
                        int length = RFIFOW(2) - 4;
                        temp = (char*)malloc(length + 1);
                        temp[length] = '\0';
                        memcpy(temp, RFIFOP(4), length);
                        std::string msg = std::string(temp);
                        unsigned int pos = msg.find(" : ", 0);

                        if (id == 0x008e) {
                            if (pos != std::string::npos) {
                                msg.erase(0, pos + 3);
                            }
                            player_node->setSpeech(msg, SPEECH_TIME);
                            chatWindow->chat_log(temp, BY_PLAYER);
                        }
                        else {
                            chatWindow->chat_log(temp, BY_GM);
                        }

                        free(temp);
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
                    being = findNode(RFIFOL(2));

                    if (being == NULL) {
                        being = new Being();
                        being->id = RFIFOL(2);
                        being->speed = RFIFOW(6);
                        if (being->speed == 0) {
                            // Else division by 0 when calculating frame
                            being->speed = 150;
                        }
                        being->job = RFIFOW(14);
                        being->x = get_x(RFIFOP(46));
                        being->y = get_y(RFIFOP(46));
                        being->direction = get_direction(RFIFOP(46));
                        being->setHairColor(RFIFOW(28));
                        being->setHairStyle(RFIFOW(16));
                        add_node(being);
                    }
                    else {
                        being->clearPath();
                        being->x = get_x(RFIFOP(46));
                        being->y = get_y(RFIFOP(46));
                        being->direction = get_direction(RFIFOP(46));
                        being->frame = 0;
                        being->walk_time = tick_time;
                        being->action = STAND;
                    }
                    break;

                case SMSG_REMOVE_BEING:
                    // A being should be removed or has died
                    being = findNode(RFIFOL(2));
                    if (being != NULL) {
                        if (RFIFOB(6) == 1) { // Death
                            if (being->job > 110) {
                                being->action = MONSTER_DEAD;
                                being->frame = 0;
                                being->walk_time = tick_time;
                            }
                            else {
                                being->action = DEAD;
                            }
                            //remove_node(RFIFOL(2));
                        }
                        else remove_node(RFIFOL(2));
                    }
                    break;

                case SMSG_PLAYER_UPDATE_1:
                case SMSG_PLAYER_UPDATE_2:
                    // A message about a player, doesn't include movement.
                    being = findNode(RFIFOL(2));

                    if (being == NULL) {
                        being = new Being();
                        being->id = RFIFOL(2);
                        being->job = RFIFOW(14);
                        add_node(being);
                    }

                    being->job = RFIFOW(14);
                    being->x = get_x(RFIFOP(46));
                    being->y = get_y(RFIFOP(46));
                    being->direction = get_direction(RFIFOP(46));
                    being->walk_time = tick_time;
                    being->frame = 0;
                    being->speed = RFIFOW(6);
                    being->setHairColor(RFIFOW(28));
                    being->setHairStyle(RFIFOW(16));
                    break;

                case SMSG_MOVE_BEING:
                    // A being nearby is moving
                    being = findNode(RFIFOL(2));

                    if (being == NULL) {
                        being = new Being();
                        being->id = RFIFOL(2);
                        being->job = RFIFOW(14);
                        add_node(being);
                    }

                    being->action = STAND;
                    being->x = get_src_x(RFIFOP(50));
                    being->y = get_src_y(RFIFOP(50));
                    being->destX = get_dest_x(RFIFOP(50));
                    being->destY = get_dest_y(RFIFOP(50));
                    being->speed = RFIFOW(6);
                    being->job = RFIFOW(14);
                    being->weapon = RFIFOW(18);

                    being->setDestination(
                            get_dest_x(RFIFOP(50)),
                            get_dest_y(RFIFOP(50)));
                    break;

                case SMSG_MOVE_PLAYER_BEING:
                    // A nearby player being moves
                    being = findNode(RFIFOL(2));

                    if (being == NULL) {
                        being = new Being();
                        being->id = RFIFOL(2);
                        being->job = RFIFOW(14);
                        add_node(being);
                    }

                    being->speed = RFIFOW(6);
                    being->job = RFIFOW(14);
                    being->x = get_src_x(RFIFOP(50));
                    being->y = get_src_y(RFIFOP(50));
                    being->destX = get_dest_x(RFIFOP(50));
                    being->destY = get_dest_y(RFIFOP(50));
                    being->setHairStyle(RFIFOW(16));
                    being->setHairColor(RFIFOW(32));

                    being->setDestination(
                            get_dest_x(RFIFOP(50)),
                            get_dest_y(RFIFOP(50)));
                    break;

                    // NPC dialog
                case 0x00b4:
                    npcTextDialog->addText(RFIFOP(8));
                    npcListDialog->setVisible(false);
                    npcTextDialog->setVisible(true);
                    current_npc = RFIFOL(4);
                    break;

                // Trade: Receiving a request to trade
                case 0x00e5:
                    //printf("Getting a call from %s\n", RFIFOP(2));
                    requestTradeDialog->request(RFIFOP(2));
                    break;

                // Trade: Response
                case 0x00e7:
                    switch (RFIFOB(2)) {
                        case 0:
                            // too far away
                            chatWindow->chat_log("Trading isn't possible. Trade partner is too far away.", BY_SERVER);
                            break;
                        case 1:
                            // Character doesn't exist
                            chatWindow->chat_log("Trading isn't possible. Character doesn't exist.", BY_SERVER);
                            break;
                        case 2:
                            // invite request check failed...
                            chatWindow->chat_log("Trade cancelled due to an unknown reason.", BY_SERVER);
                            break;
                        case 3:
                            // Trade accepted
                            tradeWindow->reset();
                            tradeWindow->setVisible(true);
                            break;
                        case 4:
                            // Trade cancelled
                            chatWindow->chat_log("Trade cancelled.", BY_SERVER);
                            break;
                        default:
                            // Shouldn't happen as well, but to be sure
                            chatWindow->chat_log("Unhandled trade cancel packet",
                                   BY_SERVER);
                            break;
                    }
                    break;
                // Trade: Item added on trade partner's side
                case 0x00e9:
                    // Should do:
                    // Maybe also handle indentified, etc
                    // later also de-clicked the ok button
                    // handle zeny as well
                    tradeWindow->addItem(
                            tradeWindow->trade_items->getFreeSlot(), RFIFOW(6),
                            false, RFIFOL(2), false);
                    break;
                // Trade: Item add response
                case 0x00ea:
                    tradeWindow->setTradeButton(false);
                    //chatWindow->chat_log("add response got", BY_SERVER);
                    switch (RFIFOB(4)) {
                        case 0:
                            // Successfully added item
                            if (inventoryWindow->items->isEquipment(RFIFOW(2))) {
                                if (inventoryWindow->items->isEquipped(RFIFOW(2))) {
                                    inventoryWindow->unequipItem(RFIFOW(2));
                                }
                            }

                            tradeWindow->addItem(
                                    tradeWindow->my_items->getFreeSlot(),
                                    inventoryWindow->items->getId(RFIFOW(2)),
                                    true, inventoryWindow->items->getQuantity(
                                    RFIFOW(2)),
                                    inventoryWindow->items->isEquipment(
                                    RFIFOW(2)));
                            inventoryWindow->changeQuantity(RFIFOW(2), 0);
                            break;
                        case 1:
                            // Add item failed - player overweighted
                            chatWindow->chat_log("Failed adding item. Trade "
                                    "partner is over weighted.", BY_SERVER);
                            break;
                        default:
                            //printf("Unhandled 0x00ea byte!\n");
                            break;
                    }
                    break;

                // Trade received Ok message
                case 0x00ec:
                    switch (RFIFOB(2)) {
                        // Received ok from myself
                        case 0:
                            tradeWindow->receivedOk(true);
                            break;
                        // Received ok from the other
                        case 1:
                            tradeWindow->receivedOk(false);
                            break;
                    }
                    break;

                // Trade cancelled
                case 0x00ee:
                    chatWindow->chat_log("Trade cancelled.", BY_SERVER);
                    tradeWindow->setVisible(false);
                    tradeWindow->reset();
                    break;

                // Trade completed
                case 0x00f0:
                    chatWindow->chat_log("Trade completed.", BY_SERVER);
                    tradeWindow->setVisible(false);
                    tradeWindow->reset();
                    break;

                    // Get the items
                    // Only called on map load / warp
                case 0x01ee:
                    // Reset all items to not load them twice on map change
                    inventoryWindow->items->resetItems();

                    for (int loop = 0; loop < (RFIFOW(2) - 4) / 18; loop++)
                    {
                       inventoryWindow->addItem(RFIFOW(4 + loop * 18),
                                RFIFOW(4 + loop * 18 + 2),
                                RFIFOW(4 + loop * 18 + 6), false);
                        // Trick because arrows are not considered equipment
                        if (RFIFOW(4 + loop * 18 + 2) == 1199 ||
                            RFIFOW(4 + loop * 18 + 2) == 529)
                            inventoryWindow->items->setEquipment(
                                RFIFOW(4 + loop * 18), true);
                        /*char info[40];
                        sprintf(info, "1ee %i", RFIFOW(4+loop*18+2));
                        chatWindow->chat_log(info, BY_SERVER);*/
                    }
                    break;

                    // Get the equipments
                case 0x00a4:
                    for (int loop = 0; loop < (RFIFOW(2) - 4) / 20; loop++)
                    {
                        inventoryWindow->addItem(RFIFOW(4 + loop * 20),
                                RFIFOW(4 + loop * 20 + 2), 1, true);
                        /*char info[40];
                        sprintf(info, "a4 %i %i %i %i %i %i %i %i",
                            RFIFOW(4+loop*20), RFIFOW(4+loop*20+2),
                            RFIFOB(4+loop*20+4), RFIFOB(4+loop*20+5),
                            RFIFOW(4+loop*20+6), RFIFOW(4+loop*20+8),
                            RFIFOB(4+loop*20+10), RFIFOB(4+loop*20+11));
                        chatWindow->chat_log(info, BY_SERVER);*/
                        if (RFIFOW(4 + loop * 20 + 8))
                        {
                            int mask = 1;
                            int position = 0;
                            while(!(RFIFOW(4+loop*20+8) & mask)) {
                                mask *= 2;
                                position++;
                            }
                            /*sprintf(info, "%i %i", mask, position);
                            chatWindow->chat_log(info, BY_SERVER);*/
                            equipmentWindow->addEquipment(position - 1,
                                RFIFOW(4+loop*20+2));
                            equipmentWindow->equipments[position - 1].inventoryIndex =
                                RFIFOW(4+loop*20);
                            inventoryWindow->items->setEquipped(
                                RFIFOW(4+loop*20), true);
                        }
                    }
                    break;
                    // Can I use the item?
                case 0x00a8:
                    if (RFIFOB(6) == 0) {
                        chatWindow->chat_log("Failed to use item", BY_OTHER);
                    } else {
                        inventoryWindow->changeQuantity(RFIFOW(2), RFIFOW(4));
                    }
                    break;
                    // Warp
                case 0x0091:
                    memset(map_path, '\0', 480);
                    strcat(map_path, "data/maps/");
                    strncat(map_path, RFIFOP(2), 497 - strlen(map_path));
                    logger.log("Warping to %s (%d, %d)",
                            map_path, RFIFOW(18), RFIFOW(20));
                    strcpy(strrchr(map_path, '.') + 1, "tmx.gz");

                    if (tiledMap) delete tiledMap;
                    tiledMap = Map::load(map_path);

                    if (tiledMap) {
                        // Delete all beings except the local player
                        std::list<Being *>::iterator i;
                        for (i = beings.begin(); i != beings.end(); i++) {
                            if ((*i) != player_node) {
                                delete (*i);
                            }
                        }
                        beings.clear();

                        // Re-add the local player node
                        add_node(player_node);

                        player_node->action = STAND;
                        player_node->frame = 0;
                        player_node->x = RFIFOW(18);
                        player_node->y = RFIFOW(20);
                        current_npc = 0;
                        walk_status = 0;
                        // Send "map loaded"
                        WFIFOW(0) = net_w_value(0x007d);
                        WFIFOSET(2);
                        while (out_size > 0) flush();
                    }
                    else {
                        logger.error("Could not find map file");
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
                    chatWindow->chat_log(action);
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
                        case 0x0009:
                            char_info->statsPointsToAttribute = RFIFOW(4);
                            /*char points[20];
                            sprintf(points, "b0 0x0009 %i", RFIFOL(4));
                            chatWindow->chat_log(points, BY_SERVER);*/
                            break;
                        case 0x0035:
                            player_node->aspd = RFIFOW(4);
                            break;
                        default:
                            /*char unknown[20];
                            sprintf(unknown, "b0 %x %i", RFIFOW(2),RFIFOL(4));
                            chatWindow->chat_log(unknown, BY_SERVER);*/
                            break;
                    }
                    statusWindow->update();
                    if (char_info->hp == 0 && deathNotice == NULL) {
                        deathNotice = new OkDialog("Message",
                                "You're now dead, press ok to restart",
                                &deathNoticeListener);
                        player_node->action = DEAD;
                    }
                    break;
                    // Stop walking
                case 0x0088:  // Disabled because giving some problems
                    //if (being = findNode(RFIFOL(2))) {
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
                            being = findNode(RFIFOL(6));
                            if (being != NULL) {

                                if (RFIFOW(22) == 0) {
                                    // Yellow
                                    being->setDamage("miss", SPEECH_TIME);
                                } else {
                                    // Blue for monster, red for player
                                    std::stringstream ss;
                                    ss << RFIFOW(22);
                                    being->setDamage(ss.str(), SPEECH_TIME);
                                }

                                if (RFIFOL(2) != player_node->id) { // buggy
                                    being = findNode(RFIFOL(2));
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
                            being = findNode(RFIFOL(2));
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
                    logger.log("Level up");
                    if (RFIFOL(2) == player_node->id) {
                        sound.playSfx("sfx/levelup.ogg");
                    }
                    break;
                    // Emotion
                case 0x00c0:
                    being = findNode(RFIFOL(2));
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
                    /*char unknown2[20];
                    sprintf(unknown2, "141 %i %i %i", RFIFOL(2),RFIFOL(6),RFIFOL(10));
                    chatWindow->chat_log(unknown2, BY_SERVER);*/
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
                        chatWindow->chat_log("Nothing to sell", BY_SERVER);
                        current_npc = 0;
                    }
                    break;
                    // Answer to buy
                case 0x00ca:
                    if (RFIFOB(2) == 0)
                        chatWindow->chat_log("Thanks for buying", BY_SERVER);
                    else
                        chatWindow->chat_log("Unable to buy", BY_SERVER);
                    break;
                    // Answer to sell
                case 0x00cb:
                    if (RFIFOB(2) == 0)
                        chatWindow->chat_log("Thanks for selling", BY_SERVER);
                    else
                        chatWindow->chat_log("Unable to sell", BY_SERVER);
                    break;
                    // Add item to inventory after you bought it
                case 0x00a0:
                    if (RFIFOB(22) > 0)
                        chatWindow->chat_log("Unable to pick up item", BY_SERVER);
                    else {
                        if(RFIFOW(19)) {
                            inventoryWindow->addItem(RFIFOW(2), RFIFOW(6),
                                RFIFOW(4), true);
                        }
                        else {
                            inventoryWindow->addItem(RFIFOW(2), RFIFOW(6),
                                RFIFOW(4), false);
                        }
                        /*char info[40];
                        sprintf(info, "a0 %i %i %i %i %i %i %i %i",
                            RFIFOW(2), RFIFOW(4), RFIFOW(6),
                            RFIFOB(8), RFIFOB(9), RFIFOB(10),
                            RFIFOW(19), RFIFOB(21));
                        chatWindow->chat_log(info, BY_SERVER);*/
                    }
                    break;
                    // Decrease quantity of an item in inventory
                case 0x00af:
                    printf("sell %i\n", -RFIFOW(4));
                    inventoryWindow->increaseQuantity(RFIFOW(2), -RFIFOW(4));
                    // If the item is arrow decrease number from equipment
                    // window when equipped
                    if (inventoryWindow->items->isEquipped(RFIFOW(2)) && (
                        inventoryWindow->items->getId(RFIFOW(2)) == 529 ||
                        inventoryWindow->items->getId(RFIFOW(2)) == 1199 ) )
                            equipmentWindow->arrowsNumber -= RFIFOW(4);
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
                }
                    break;

                    // MVP experience
                case 0x010b:
                    break;
                    // Display MVP player
                case 0x010c:
                    chatWindow->chat_log("MVP player", BY_SERVER);
                    break;
                    // Item is found
                case 0x009d:
                    floorItem = new FloorItem();
                    floorItem->id = net_w_value(RFIFOW(6));
                    floorItem->x = net_w_value(RFIFOW(9));
                    floorItem->y = net_w_value(RFIFOW(11));
                    floorItem->int_id = net_l_value(RFIFOL(2));
                    add_floor_item(floorItem);
                    break;
                    // Item drop
                case 0x009e:
                    floorItem = new FloorItem();
                    floorItem->id = net_w_value(RFIFOW(6));
                    floorItem->x = net_w_value(RFIFOW(9));
                    floorItem->y = net_w_value(RFIFOW(11));
                    floorItem->int_id = net_l_value(RFIFOL(2));
                    add_floor_item(floorItem);
                    break;
                    // Item disappearing
                case 0x00a1:
                     floorItem = find_floor_item_by_id(net_l_value(RFIFOL(2)));
                     if (floorItem != NULL) {
                         remove_floor_item(net_l_value(RFIFOL(2)));
                     }
                     else {
                         remove_floor_item(net_l_value(RFIFOL(2)));
                     }
                     break;
                    // Next button in NPC dialog
                case 0x00b5:
                    strcpy(npc_button, "Next");
                    current_npc = RFIFOL(2);
                    break;
                    // Close button in NPC dialog
                case 0x00b6:
                    strcpy(npc_button, "Close");
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

                case SMSG_CHANGE_BEING_LOOKS:
                    being = findNode(RFIFOL(2));
                    if (being) {
                        if (RFIFOB(6) == 6) {
                            being->setHairColor(RFIFOB(7));
                        } else if (RFIFOB(6) == 1) {
                            being->setHairStyle(RFIFOB(7));
                        }
                    }
                    break;

                    // Answer to equip item
                case 0x00aa:
                    logger.log("Equipping: %i %i %i", RFIFOW(2), RFIFOW(4), RFIFOB(6));
                    if (RFIFOB(6) == 0)
                        chatWindow->chat_log("Unable to equip.", BY_SERVER);
                    else {
                         if(RFIFOW(4)) {
                            int mask = 1;
                            int position = 0;
                            while(!(RFIFOW(4) & mask)) {
                                mask *= 2;
                                position++;
                            }
                            logger.log("Position %i", position-1);
                            int equippedId = equipmentWindow->equipments[position - 1].id;
                            if (equippedId > 0)
                                inventoryWindow->items->setEquipped(
                                    equipmentWindow->equipments[position - 1].inventoryIndex,
                                    false);
                            

                            inventoryWindow->items->setEquipped(RFIFOW(2),
                                true);
                            equipmentWindow->addEquipment(position - 1,
                                inventoryWindow->items->getId(RFIFOW(2)));
                            equipmentWindow->equipments[position - 1].inventoryIndex = RFIFOW(2);
                            
                            // Trick to use the proper graphic until I find
                            // the right packet
                            switch (inventoryWindow->items->getId(RFIFOW(2))) {
                                case 521:
                                case 522:
                                case 1201:
                                    player_node->weapon = 1;
                                    break;
                                case 530:
                                case 1200:
                                    player_node->weapon = 2;
                                    break;
                            }
                            //player_node->weapon = 0;
                        }
                    }
                    break;
                    // Equipment related
                case 0x01d7:
                    logger.log("1d7 %i %i %i %i", RFIFOL(2), RFIFOB(6), RFIFOW(7), RFIFOW(9));
                    break;
                    // Answer to unequip item
                case 0x00ac:
                    if (RFIFOB(6) == 0)
                        chatWindow->chat_log("Unable to unequip.", BY_SERVER);
                    else {
                        if(RFIFOW(4)) {
                            int mask = 1;
                            int position = 0;
                            while(!(RFIFOW(4) & mask)) {
                                mask *= 2;
                                position++;
                            }
                            inventoryWindow->items->setEquipped(RFIFOW(2), false);
                            switch (inventoryWindow->items->getId(RFIFOW(2))) {
                                case 529:
                                case 1199:
                                    equipmentWindow->setArrows(0);
                                    equipmentWindow->arrowsNumber = 0;
                                    break;
                                case 521:
                                case 522:
                                case 530:
                                case 1200:
                                case 1201:
                                    player_node->weapon = 0;
                                    break;
                                default:
                                    equipmentWindow->removeEquipment(position - 1);
                                    break;
                            }
                            logger.log("Unequipping: %i %i(%i) %i", RFIFOW(2),RFIFOW(4),RFIFOB(6), position -1);
                        }
                    }
                    break;
                    // Arrows equipped
                case 0x013c:
                    if (RFIFOW(2) > 1) {
                        inventoryWindow->items->setEquipped(RFIFOW(2), true);
                        equipmentWindow->setArrows(
                            inventoryWindow->items->getId(RFIFOW(2)));
                        equipmentWindow->arrowsNumber =
                            inventoryWindow->items->getQuantity(RFIFOW(2));
                            logger.log("Arrows equipped: %i", RFIFOW(2));
                    }
                    break;
                    // Various messages
                case 0x013b:
                    if (RFIFOW(2) == 0)
                        chatWindow->chat_log("Equip arrows first", BY_SERVER);
                    else
                        logger.log("0x013b: Unhandled message %i", RFIFOW(2));
                    break;
                    // Updates a stat value
                case 0x00bc:
                    /*char stat[20];
                    sprintf(stat, "bc %x %i %i", RFIFOW(2),RFIFOB(4),RFIFOB(5));
                    chatWindow->chat_log(stat, BY_SERVER);*/
                    if(RFIFOB(4)) {
                        switch(RFIFOW(2)) {
                            case 0x000d:
                                char_info->STR = RFIFOB(5);
                                break;
                            case 0x000e:
                                char_info->AGI = RFIFOB(5);
                                break;
                            case 0x000f:
                                char_info->VIT = RFIFOB(5);
                                break;
                            case 0x0010:
                                char_info->INT = RFIFOB(5);
                                break;
                            case 0x0011:
                                char_info->DEX = RFIFOB(5);
                                break;
                            case 0x0012:
                                char_info->LUK = RFIFOB(5);
                                break;                                
                        }
                    }
                    break;
                    // Updates stats and status points
                case 0x00bd:
                    char_info->statsPointsToAttribute = RFIFOW(2);
                    char_info->STR = RFIFOB(4);
                    char_info->STRUp = RFIFOB(5);
                    char_info->AGI = RFIFOB(6);
                    char_info->AGIUp = RFIFOB(7);
                    char_info->VIT = RFIFOB(8);
                    char_info->VITUp = RFIFOB(9);
                    char_info->INT = RFIFOB(10);
                    char_info->INTUp = RFIFOB(11);
                    char_info->DEX = RFIFOB(12);
                    char_info->DEXUp = RFIFOB(13);
                    char_info->LUK = RFIFOB(14);
                    char_info->LUKUp = RFIFOB(15);
                    /*char stats[100];
                    sprintf(stats, "%i %i %i %i %i %i %i %i %i %i %i %i",
                    RFIFOB(4),RFIFOB(5),RFIFOB(6),RFIFOB(7),RFIFOB(8),RFIFOB(9),
                    RFIFOB(10),RFIFOB(11),RFIFOB(12),RFIFOB(13),RFIFOB(14),RFIFOB(15));
                    chatWindow->chat_log(stats,BY_SERVER);*/
                    statusWindow->update();
                    break;
                    // Updates status point
                case 0x00be:
                    switch(RFIFOW(2)) {
                        case 0x0020:
                            char_info->STRUp = RFIFOB(4);
                            break;
                        case 0x0021:
                            char_info->AGIUp = RFIFOB(4);
                            break;
                        case 0x0022:
                            char_info->VITUp = RFIFOB(4);
                            break;
                        case 0x0023:
                            char_info->INTUp = RFIFOB(4);
                            break;
                        case 0x0024:
                            char_info->DEXUp = RFIFOB(4);
                            break;
                        case 0x0025:
                            char_info->LUKUp = RFIFOB(4);
                            break;
                    }
                    break;
                // Get being name
                case 0x0095:
                    being = findNode(RFIFOL(2));
                    if (being)
                        strcpy(being->name, RFIFOP(6));
                    break;
                    // Manage non implemented packets
                default:
                    logger.log("Unhandled packet: %x", id);
                    break;
            }

            RFIFOSKIP(len);
        }
    }
}
