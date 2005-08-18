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

#include "game.h"

#include <sstream>

#include <guichan/sdl/sdlinput.hpp>

#include "being.h"
#include "configuration.h"
#include "engine.h"
#include "equipment.h"
#include "floor_item.h"
#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "log.h"
#include "main.h"
#include "map.h"
#include "playerinfo.h"
#include "sound.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/chat.h"
#include "gui/confirm_dialog.h"
#include "gui/equipmentwindow.h"
#include "gui/gui.h"
#include "gui/help.h"
#include "gui/inventorywindow.h"
#include "gui/npc.h"
#include "gui/npc_text.h"
#include "gui/ok_dialog.h"
#include "gui/popupmenu.h"
#include "gui/requesttrade.h"
#include "gui/sell.h"
#include "gui/setup.h"
#include "gui/skill.h"
#include "gui/stats.h"
#include "gui/status.h"
#include "gui/trade.h"

#include "net/network.h"
#include "net/protocol.h"

#include "resources/mapreader.h"


extern Graphics *graphics;

char map_path[480];
char tradePartnerName[24];

bool refresh_beings = false;
unsigned char keyb_state;
volatile int tick_time;
volatile bool action_time = false;
int server_tick;
int fps = 0, frame = 0, current_npc = 0;
bool displayPathToMouse = false;
int startX = 0, startY = 0;
int gameTime = 0;
Being *autoTarget = NULL;
Engine *engine = NULL;
SDL_Joystick *joypad = NULL;       /**< Joypad object */

OkDialog *deathNotice = NULL;
ConfirmDialog *exitConfirm = NULL;

Inventory *inventory = NULL;

const int EMOTION_TIME = 150;    /**< Duration of emotion icon */
const int MAX_TIME = 10000;

/**
 * Listener used for handling death message.
 */
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
 * Listener used for exitting handling.
 */
class ExitListener : public gcn::ActionListener {
    void action(const std::string &eventId) {
        if (eventId == "yes") {
            state = EXIT;
        }
        exitConfirm = NULL;
    }
} exitListener;

/**
 * Advances game logic counter.
 */
Uint32 nextTick(Uint32 interval, void *param)
{
    tick_time++;
    if (tick_time == MAX_TIME) tick_time = 0;
    return interval;
}

/**
 * Lets u only trigger an action every other second
 * tmp. counts fps
 */
Uint32 nextSecond(Uint32 interval, void *param)
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

Being* createBeing(unsigned int id, unsigned short job, Map *map)
{
    Being *being = new Being;

    being->setId(id);
    being->job = job;
    being->setMap(map);

    return being;
}

void do_init()
{
    std::string path(map_path);
    std::string pathDir = path.substr(0, path.rfind("."));

    // Try .tmx.gz map file
    pathDir.insert(pathDir.size(), ".tmx.gz");
    Map *tiledMap = MapReader::readMap(pathDir);

    if (!tiledMap)
    {
        logger->error("Could not find map file!");
    }
    else
    {
        engine->setCurrentMap(tiledMap);
    }

    // Initialize timers
    tick_time = 0;
    SDL_AddTimer(10, nextTick, NULL);                     // Logic counter
    SDL_AddTimer(1000, nextSecond, NULL);                 // Seconds counter

    // Initialize beings
    player_node = createBeing(account_ID, 0, tiledMap);
    player_node->x = startX;
    player_node->y = startY;
    player_node->speed = 150;
    player_node->setHairColor(char_info->hair_color);
    player_node->setHairStyle(char_info->hair_style);

    if (char_info->weapon == 11) {
        char_info->weapon = 2;
    }

    player_node->setWeapon(char_info->weapon);

    add_node(player_node);

    remove("packet.list");

    // Initialize joypad
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    //SDL_JoystickEventState(SDL_ENABLE);
    int num_joy = SDL_NumJoysticks();
    logger->log("%i joysticks/gamepads found", num_joy);
    for (int i = 0; i < num_joy; i++)
        logger->log("- %s", SDL_JoystickName(i));
    // TODO: The user should be able to choose which one to use
    // Open the first device
    if (num_joy > 0)
    {
        joypad = SDL_JoystickOpen(0);
        if (joypad == NULL)
        {
            logger->log("Couldn't open joystick: %s", SDL_GetError());
        }
        else {
            logger->log("Axes: %i ", SDL_JoystickNumAxes(joypad));
            logger->log("Balls: %i", SDL_JoystickNumBalls(joypad));
            logger->log("Hats: %i", SDL_JoystickNumHats(joypad));
            logger->log("Buttons: %i", SDL_JoystickNumButtons(joypad));
        }
    }
}

void game()
{
    // Needs to be initialised _before_ the engine is created...
    inventory = new Inventory();

    engine = new Engine();
    do_init();

    gameTime = tick_time;

    while (state != EXIT)
    {
        // Handle all necessary game logic
        while (get_elapsed_time(gameTime) > 0)
        {
            do_input();
            engine->logic();
            gui->logic();
            gameTime++;
        }

        gameTime = tick_time;

        // Update the screen when application is active, delay otherwise
        if (SDL_GetAppState() & SDL_APPACTIVE)
        {
            engine->draw();
            graphics->updateScreen();
        }
        else
        {
            SDL_Delay(10);
        }

        // Handle network stuff and flush it
        do_parse();
        flush();
    }

    delete engine;
    close_session();
}

void do_exit()
{
    if (joypad != NULL)
    {
        SDL_JoystickClose(joypad);
    }
}

void do_input()
{
    // Get the state of the keyboard keys
    Uint8* keys;
    keys = SDL_GetKeyState(NULL);

    // Get the state of the joypad buttons
    // TODO: Only 6- buttons joypads are allowed
    bool joy[10];
    for (int i=0; i<10; i++)
    {
        joy[i] = false;
    }
    if (joypad != NULL)
    {
        // TODO: one different value of tolerance is needed for each direction
        // This probably means the need for a tuning utility/window
        int tolerance = (int)config.getValue("joytolerance", 10);
        SDL_JoystickUpdate();
        if (SDL_JoystickGetAxis(joypad, 0) > tolerance)
        {
            joy[JOY_RIGHT] = true;
        }
        if (SDL_JoystickGetAxis(joypad, 0) < -tolerance)
        {
            joy[JOY_LEFT] = true;
        }
        if (SDL_JoystickGetAxis(joypad, 1) < -tolerance)
        {
            joy[JOY_UP] = true;
        }
        if (SDL_JoystickGetAxis(joypad, 1) > tolerance)
        {
            joy[JOY_DOWN] = true;
        }
        for (int i=0; i<6; i++)
        {
            if (SDL_JoystickGetButton(joypad, i) == 1)
            {
                joy[JOY_BTN0 + i] = true;
            }
        }
    }

    // Events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        bool used = false;

        // Keyboard events (for discontinuous keys)
        if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
                    // In-game Help
                case SDLK_F1:
                    if (helpWindow->isVisible()) {
                        helpWindow->setVisible(false);
                    } else {
                        helpWindow->loadHelp("index");
                    }
                    used = true;
                    break;

                    // Player sit action
                case SDLK_F5:
                    if (!action_time) {
                        break;
                    }

                    switch (player_node->action)
                    {
                        case Being::STAND:
                            action(2, 0);
                            break;
                        case Being::SIT:
                            action(3, 0);
                            break;
                    }
                    break;

                    // Display path to mouse (debug purpose)
                case SDLK_F6:
                    displayPathToMouse = !displayPathToMouse;
                    break;

                    // Input chat window
                case SDLK_RETURN:
                    if (chatWindow->isFocused()) {
                        break;
                    }

                    // Quit by pressing Enter if the exit confirm is there
                    if (exitConfirm)
                    {
                        state = EXIT;
                    }
                    // Close the Browser if opened
                    else if (helpWindow->isVisible())
                    {
                        helpWindow->setVisible(false);
                    }
                    // Close the config window, cancelling changes if opened
                    else if (setupWindow->isVisible())
                    {
                        setupWindow->action("cancel");
                    }
                    // Else, open the chat edit box
                    else
                    {
                        chatWindow->requestChatFocus();
                        used = true;
                    }
                    break;

                    // Picking up items on the floor
                case SDLK_g:
                case SDLK_z:
                    if (!chatWindow->isFocused()) {
                        unsigned short x = player_node->x;
                        unsigned short y = player_node->y;
                        int id = find_floor_item_by_cor(x, y);

                        // If none below the player, try the tile in front of the player
                        if (!id) {
                            switch (player_node->direction)
                            {
                                case Being::NORTH: y--; break;
                                case Being::SOUTH: y++; break;
                                case Being::WEST:  x--; break;
                                case Being::EAST:  x++; break;
                                case Being::NW:    x--; y--; break;
                                case Being::NE:    x++; y--; break;
                                case Being::SW:    x--; y++; break;
                                case Being::SE:    x++; y++; break;
                                default: break;
                            }
                            id = find_floor_item_by_cor(x, y);
                        }

                        if (id)
                        {
                            WFIFOW(0) = net_w_value(0x009f);
                            WFIFOL(2) = net_l_value(id);
                            WFIFOSET(6);
                        }
                        used = true;
                    }
                    break;

                    // Quitting confirmation dialog
                case SDLK_ESCAPE:
                    if (!exitConfirm) {
                        exitConfirm = new ConfirmDialog(
                                "Quit", "Are you sure you want to quit?",
                                (gcn::ActionListener*)&exitListener);
                    }
                    break;

                default:
                    break;
            }

            // Keys pressed together with Alt/Meta
            // Emotions and some internal gui windows
            if (event.key.keysym.mod & KMOD_ALT)
            {
                switch (event.key.keysym.sym)
                {
                        // Inventory window
                    case SDLK_i:
                        inventoryWindow->setVisible(
                                !inventoryWindow->isVisible());
                        used = true;
                        break;

                        // Statistics window
                    case SDLK_s:
                        statsWindow->setVisible(!statsWindow->isVisible());
                        used = true;
                        break;

                    /*
                        // New skills window
                    case SDLK_n:
                        newSkillWindow->setVisible(!newSkillWindow->isVisible());
                        used = true;
                        break;
                    */

                        // Skill window
                    case SDLK_k:
                        skillDialog->setVisible(!skillDialog->isVisible());
                        used = true;
                        break;

                        // Setup window
                    case SDLK_c:
                        setupWindow->setVisible(true);
                        used = true;
                        break;

                        // Equipment window
                    case SDLK_e:
                        equipmentWindow->setVisible(
                                !equipmentWindow->isVisible());
                        used = true;
                        break;

                    /*
                        // Buddy window
                    case SDLK_b:
                        buddyWindow->setVisible(!buddyWindow->isVisible());
                        used = true;
                        break;
                    */

                    default:
                        break;
                }

                // Emotions
                if (action_time && !player_node->emotion)
                {
                    unsigned char emotion = 0;
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_1: emotion = 1; break;
                        case SDLK_2: emotion = 2; break;
                        case SDLK_3: emotion = 3; break;
                        case SDLK_4: emotion = 4; break;
                        case SDLK_5: emotion = 5; break;
                        case SDLK_6: emotion = 6; break;
                        case SDLK_7: emotion = 7; break;
                        case SDLK_8: emotion = 8; break;
                        case SDLK_9: emotion = 9; break;
                        case SDLK_0: emotion = 10; break;
                        default: break;
                    }

                    if (emotion)
                    {
                        WFIFOW(0) = net_w_value(0x00bf);
                        WFIFOB(2) = emotion;
                        WFIFOSET(3);
                        action_time = false;
                        used = true;
                    }
                }
            }
        }

        // Mouse events
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            int mx = event.button.x / 32 + camera_x;
            int my = event.button.y / 32 + camera_y;

            // Mouse button left
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                // Don't move when shift is pressed
                // XXX Is this too hackish? I'm not sure if making the Gui
                // class a KeyListener is a good idea and works as expected
                // at all...
                if (keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]) {
                    used = true;
                }

                // Check for default actions for NPC/Monster/Players
                Being *target = findNode(mx, my);
                unsigned int floorItemId = find_floor_item_by_cor(mx, my);

                if (target)
                {
                    switch (target->getType())
                    {
                        // Player default: trade
                        case Being::PLAYER:
                            WFIFOW(0) = net_w_value(0x00e4);
                            WFIFOL(2) = net_l_value(target->getId());
                            WFIFOSET(6);
                            strcpy(tradePartnerName, target->name);
                            break;

                            // NPC default: talk
                        case Being::NPC:
                            if (!current_npc)
                            {
                                WFIFOW(0) = net_w_value(0x0090);
                                WFIFOL(2) = net_l_value(target->getId());
                                WFIFOB(6) = 0;
                                WFIFOSET(7);
                                current_npc = target->getId();
                            }
                            break;

                            // Monster default: attack
                        case Being::MONSTER:
                            /**
                             * TODO: Move player to mouse click position before
                             * attack the monster (maybe using follow mode).
                             */
                            if (target->action != Being::MONSTER_DEAD &&
                                    player_node->action == Being::STAND)
                            {
                                attack(target);
                                // Autotarget by default with mouse
                                //if (keys[SDLK_LSHIFT])
                                //{
                                autoTarget = target;
                                //}
                            }
                            break;

                        default:
                            break;
                    }
                }
                // Check for default action to items on the floor
                else if (floorItemId != 0)
                {
                    /**
                     * TODO: Move player to mouse click position before
                     * pick up the items on the floor.
                     *
                     * Provisory: pick up only items near of player.
                     */
                    int dx = mx - player_node->x;
                    int dy = my - player_node->y;
                    // "sqrt(dx*dx + dy*dy) < 2" is equal to "dx*dx + dy*dy < 4"
                    if ((dx*dx + dy*dy) < 4)
                    {
                        WFIFOW(0) = net_w_value(0x009f);
                        WFIFOL(2) = net_l_value(floorItemId);
                        WFIFOSET(6);
                    }
                }

                // Just cancel the popup menu if shown, and don't make the character walk
                if (popupMenu->isVisible() == true)
                {
                    // If we click elsewhere than in the window, do not use the event
                    // The user wanted to close the popup.
                    // Still buggy : Wonder if the x, y, width, and height aren't reported partially
                    // with these functions.
                    if (event.button.x >= (popupMenu->getX() + popupMenu->getWidth()) ||
                            event.button.x < popupMenu->getX() ||
                            event.button.y >= (popupMenu->getY() + popupMenu->getHeight()) ||
                            event.button.y < popupMenu->getY())
                    {
                            used = true;
                            popupMenu->setVisible(false);
                    }
                }

            } // End Mouse left button

            // Mouse button middle
            else if (event.button.button == SDL_BUTTON_MIDDLE)
            {
                /**
                 * Some people haven't a mouse with three buttons,
                 * right Usiu??? ;-)
                 */
            }

            // Mouse button right
            else if (event.button.button == SDL_BUTTON_RIGHT)
            {
                Being *being;
                FloorItem *floorItem;

                if ((being = findNode(mx, my))) {
                    popupMenu->showPopup(event.button.x, event.button.y,
                            being);
                } else if ((floorItem = find_floor_item_by_id(
                            find_floor_item_by_cor(mx, my)))) {
                    popupMenu->showPopup(event.button.x, event.button.y,
                            floorItem);
                } else {
                    popupMenu->setVisible(false);
                }
            }
        }

        // Quit event
        else if (event.type == SDL_QUIT)
        {
            state = EXIT;
        }

        // Push input to GUI when not used
        if (!used) {
            guiInput->pushInput(event);
        }

    } // End while
    
    // Moving player around
    if ((player_node->action != Being::DEAD) && (current_npc == 0) &&
            !chatWindow->isFocused())
    {
        int x = player_node->x;
        int y = player_node->y;
        int xDirection = 0;
        int yDirection = 0;
        Being::Direction Direction = Being::DIR_NONE;

        // Translate pressed keys to movement and direction
        if (keys[SDLK_UP] || keys[SDLK_KP8] || joy[JOY_UP])
        {
            yDirection = -1;
            if (player_node->action != Being::WALK)
                Direction = Being::NORTH;
        }
        if (keys[SDLK_DOWN] || keys[SDLK_KP2] || joy[JOY_DOWN])
        {
            yDirection = 1;
            if (player_node->action != Being::WALK)
                Direction = Being::SOUTH;
        }
        if (keys[SDLK_LEFT] || keys[SDLK_KP4] || joy[JOY_LEFT])
        {
            xDirection = -1;
            if (player_node->action != Being::WALK)
                Direction = Being::WEST;
        }
        if (keys[SDLK_RIGHT] || keys[SDLK_KP6] || joy[JOY_RIGHT])
        {
            xDirection = 1;
            if (player_node->action != Being::WALK)
                Direction = Being::EAST;
        }
        if (keys[SDLK_KP1]) // Bottom Left
        {
            xDirection = -1;
            yDirection = 1;
            if (player_node->action != Being::WALK)
                Direction = Being::SW;
        }
        if (keys[SDLK_KP3]) // Bottom Right
        {
            xDirection = 1;
            yDirection = 1;
            if (player_node->action != Being::WALK)
                Direction = Being::SE;
        }
        if (keys[SDLK_KP7]) // Top Left
        {
            xDirection = -1;
            yDirection = -1;
            if (player_node->action != Being::WALK)
                Direction = Being::NW;
        }
        if (keys[SDLK_KP9]) // Top Right
        {
            xDirection = 1;
            yDirection = -1;
            if (player_node->action != Being::WALK)
                Direction = Being::NE;
        }

        Map *tiledMap = engine->getCurrentMap();

        // Allow keyboard control to interrupt an existing path
        if ((xDirection != 0 || yDirection != 0) && player_node->action == Being::WALK)
            player_node->setDestination(x, y);

        if (player_node->action != Being::WALK)
        {
            // Prevent skipping corners over colliding tiles
            if ((xDirection != 0) && tiledMap->tileCollides(x + xDirection, y))
                xDirection = 0;
            if ((yDirection != 0) && tiledMap->tileCollides(x, y + yDirection))
                yDirection = 0;

            // Choose a straight direction when diagonal target is blocked
            if ((yDirection != 0) && (xDirection != 0) && 
                    !tiledMap->getWalk(x + xDirection, y + yDirection))
                xDirection = 0;

            // Walk to where the player can actually go
            if (((xDirection != 0) || (yDirection != 0)) &&
                    tiledMap->getWalk(x + xDirection, y + yDirection))
            {
                walk(x + xDirection, y + yDirection, Direction);
                player_node->setDestination(x + xDirection, y + yDirection);
            }
            else if (Direction != Being::DIR_NONE)
            {
                // Update the player direction to where he wants to walk
                // Warning: Not communicated to the server yet
                player_node->direction = Direction;
            }
        }

        // Attacking monsters
        if (player_node->action == Being::STAND)
        {
            if (keys[SDLK_LCTRL] || keys[SDLK_RCTRL] || joy[JOY_BTN0])
            {
                Being *monster = attack(x, y, player_node->direction);
                if (monster == NULL && autoTarget != NULL)
                {
                    attack(autoTarget);
                }
                else if (keys[SDLK_LSHIFT])
                {
                    autoTarget = monster;
                }
            }
        }

        if (joy[JOY_BTN1]) {
            unsigned short x = player_node->x;
            unsigned short y = player_node->y;
            int id = find_floor_item_by_cor(x, y);

            if (id != 0)
            {
                WFIFOW(0) = net_w_value(0x009f);
                WFIFOL(2) = net_l_value(id);
                WFIFOSET(6);
            }
        }
        else if (joy[JOY_BTN2] && action_time) {
            if (player_node->action == Being::STAND)
                action(2, 0);
            else if (player_node->action == Being::SIT)
                action(3, 0);
            action_time = false;
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


/*switch 0079, actor connected
 	[10:44]	Joseph_: 0078, actor_exists
 	[10:44]	Munak: Heck, if Bill Clinton can do it surely *I* can!
 	[10:45]	Joseph_: 007B, 01D8,  01D9, 01DA are the others*/

    unsigned short id;
    char *temp;
    Being *being = NULL;
    FloorItem *floorItem = NULL;
    int len, n_items;
    Map *tiledMap = engine->getCurrentMap();
    Equipment *equipment = Equipment::getInstance();

    // We need at least 2 bytes to identify a packet
    if (in_size >= 2) {
        // Check if the received packet is complete
        while (in_size >= (len = get_packet_length(id = RFIFOW(0)))) {
            printf("Packet_ID: %x\n", RFIFOW(0));

            // Parse packet based on their id
            switch (id)
            {
                case SMSG_LOGIN_SUCCESS:
                    // Connected to game server succesfully, set spawn point
                    player_node->x = get_x(RFIFOP(6));
                    player_node->y = get_y(RFIFOP(6));
                    break;

                // Received speech from being
                case SMSG_BEING_CHAT:
                    being = findNode(RFIFOL(4));
                    if (being != NULL) 
                    {
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
                    if (RFIFOW(2) > 4) 
                    {
                        int length = RFIFOW(2) - 4;
                        temp = (char*)malloc(length + 1);
                        temp[length] = '\0';
                        memcpy(temp, RFIFOP(4), length);
                        std::string msg = std::string(temp);
                        unsigned int pos = msg.find(" : ", 0);

                        if (id == 0x008e) 
                        {
                            if (pos != std::string::npos)
                            {
                                msg.erase(0, pos + 3);
                            }
                            player_node->setSpeech(msg, SPEECH_TIME);
                            chatWindow->chat_log(temp, BY_PLAYER);
                        }
                        else 
                        {
                            chatWindow->chat_log(temp, BY_GM);
                        }

                        free(temp);
                    }
                    break;

                case SMSG_WALK_RESPONSE:
                    // It is assumed by the client any request to walk actually
                    // succeeds on the server. The plan is to have a correction
                    // message when the server senses the client has the wrong
                    // idea.
                    break;

                    // Add new being / stop monster
                case 0x0078:

                    int beingId;
                    beingId = RFIFOL(2);
                    int beingJob;
                    beingJob = RFIFOW(14);

                    // Being with id >= 110000000 and job 0 are better known
                    // as ghosts, so don't create those.
                    if (beingJob == 0 && beingId >= 110000000)
                    {
                        break;
                    }

                    being = findNode(beingId);

                    if (being == NULL)
                    {
                        being = createBeing(beingId, beingJob, tiledMap);
                        being->speed = RFIFOW(6);
                        if (being->speed == 0) {
                            // Else division by 0 when calculating frame
                            being->speed = 150;
                        }
                        being->setHairStyle(RFIFOW(16));
                        being->setHairColor(RFIFOW(28));
                        being->setWeapon(RFIFOW(18));
                        being->setMap(tiledMap);
                        add_node(being);
                    }
                    else
                    {
                        being->clearPath();
                        //being->setWeapon(RFIFOW(18));
                        being->frame = 0;
                        being->walk_time = tick_time;
                        being->action = Being::STAND;
                    }
                    being->x = get_x(RFIFOP(46));
                    being->y = get_y(RFIFOP(46));
                    being->direction = get_direction(RFIFOP(46));
                    break;

                case SMSG_REMOVE_BEING:
                    // A being should be removed or has died
                    being = findNode(RFIFOL(2));
                    if (being != NULL)
                    {
                        if (RFIFOB(6) == 1)
                        { // Death
                            switch (being->getType())
                            {
                                case Being::MONSTER:
                                    being->action = Being::MONSTER_DEAD;
                                    being->frame = 0;
                                    being->walk_time = tick_time;
                                    break;

                                default:
                                    being->action = Being::DEAD;
                                    break;
                            }
                            //remove_node(RFIFOL(2));
                        }
                        else {
                            remove_node(RFIFOL(2));
                        }

                        if (being == autoTarget) {
                            autoTarget = NULL;
                        }
                    }
                    break;

                case SMSG_PLAYER_UPDATE_1:
                case SMSG_PLAYER_UPDATE_2:
                    // A message about a player, doesn't include movement.
                    being = findNode(RFIFOL(2));

                    if (being == NULL) 
                    {
                        being = createBeing(RFIFOL(2), RFIFOW(14), tiledMap);
                        add_node(being);
                    }

                    being->speed = RFIFOW(6);
                    being->job = RFIFOW(14);
                    being->setHairStyle(RFIFOW(16));
                    being->setHairColor(RFIFOW(28));
                    being->x = get_x(RFIFOP(46));
                    being->y = get_y(RFIFOP(46));
                    being->direction = get_direction(RFIFOP(46));
                    being->walk_time = tick_time;
                    being->frame = 0;
                    being->setWeaponById(RFIFOW(18));
                    //logger->log("0x01d8% i %i", RFIFOW(18), RFIFOW(20));

                    if (RFIFOB(51) == 2)
                    {
                        being->action = Being::SIT;
                    }
                    break;

                case SMSG_MOVE_BEING:
                    // A being nearby is moving
                    being = findNode(RFIFOL(2));

                    if (being == NULL)
                    {
                        being = createBeing(RFIFOL(2), RFIFOW(14), tiledMap);
                        add_node(being);
                    }

                    being->action = Being::STAND;
                    being->x = get_src_x(RFIFOP(50));
                    being->y = get_src_y(RFIFOP(50));
                    being->speed = RFIFOW(6);
                    being->job = RFIFOW(14);
                    being->setDestination(
                                get_dest_x(RFIFOP(50)),
                                get_dest_y(RFIFOP(50)));
                    break;

                case SMSG_MOVE_PLAYER_BEING:
                    // A nearby player being moves
                    /*being = findNode(RFIFOL(2));

                    if (being == NULL)
                    {
                        being = createBeing(RFIFOL(2), RFIFOW(14), tiledMap);
                        add_node(being);
                    }

                    being->speed = RFIFOW(6);
                    being->job = RFIFOW(14);
                    being->x = get_src_x(RFIFOP(50));
                    being->y = get_src_y(RFIFOP(50));
                    being->setHairStyle(RFIFOW(16));
                    being->setWeaponById(RFIFOW(18));
                    being->setHairColor(RFIFOW(32));

                    being->setDestination(
                                get_dest_x(RFIFOP(50)),
                                get_dest_y(RFIFOP(50)));*/
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
                    // If a trade window is already open, send a trade cancel
                    // to any other trade request.
                    // It would still be nice to implement an independent message
                    // that the person you want to trade with can't do that now.
                    if (tradeWindow->isVisible() == true)
                    {
                        // 0xff packet means cancel
                        WFIFOW(0) = net_w_value(0x00e6);
                        WFIFOB(2) = net_b_value(4);
                        WFIFOSET(3);
                        break;
                    }
                    if (!requestTradeDialogOpen)
                    {
                        requestTradeDialogOpen = true;
                        strcpy(tradePartnerName, RFIFOP(2));
                        new RequestTradeDialog(RFIFOP(2));
                    }
                    break;

                // Trade: Response
                case 0x00e7:
                    switch (RFIFOB(2))
                    {
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
                            tradeWindow->setCaption((std::string)"Trade: You and " + (std::string)tradePartnerName);
                            tradeWindow->setVisible(true);
                            requestTradeDialogOpen = false;
                            break;
                        case 4:
                            // Trade cancelled
                            chatWindow->chat_log("Trade cancelled.", BY_SERVER);
                            tradeWindow->setVisible(false);
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
                    // TODO: handle also identified, etc
                    if (RFIFOW(6) == 0)
                    {
                        tradeWindow->addMoney(RFIFOL(2));
                    } 
                    else 
                    {
                        tradeWindow->addItem(RFIFOW(6), false, RFIFOL(2), false);
                    }
                    break;
                // Trade: New Item add response
                case 0x01b1:
                    {
                        Item *item = inventory->getItem(RFIFOW(2));
                        switch (RFIFOB(6))
                        {
                            case 0:
                                // Successfully added item
                                if (item->isEquipment() && item->isEquipped())
                                {
                                    inventory->unequipItem(item);
                                }
                                tradeWindow->addItem(
                                        item->getId(), true, RFIFOW(4),
                                        item->isEquipment());
                                item->increaseQuantity(-RFIFOW(4));
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
                    }
                    break;
                // Trade received Ok message
                case 0x00ec:
                    // 0 means ok from myself, 1 means ok from other;
                    tradeWindow->receivedOk((RFIFOB(2) == 0));
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
                    inventory->resetItems();

                    for (int loop = 0; loop < (RFIFOW(2) - 4) / 18; loop++)
                    {
                        inventory->addItem(RFIFOW(4 + loop * 18),
                                RFIFOW(4 + loop * 18 + 2),
                                RFIFOW(4 + loop * 18 + 6), false);
                        // Trick because arrows are not considered equipment
                        if (RFIFOW(4 + loop * 18 + 2) == 1199 ||
                            RFIFOW(4 + loop * 18 + 2) == 529)
                        {
                            inventory->getItem(
                                    RFIFOW(4 + loop * 18))->setEquipment(true);
                        }
                    }
                    break;

                    // Get the equipments
                case 0x00a4:

                    for (int loop = 0; loop < ((RFIFOW(2) - 4) / 20); loop++)
                    {
                        inventory->addItem(RFIFOW(4 + loop * 20),
                                RFIFOW(4 + loop * 20 + 2), 1, true);
                        if (RFIFOW(4 + loop * 20 + 8))
                        {
                            int mask = 1;
                            int position = 0;
                            while(!(RFIFOW(4+loop*20+8) & mask))
                            {
                                mask *= 2;
                                position++;
                            }
                            Item *item = inventory->getItem(RFIFOW(4+loop*20));
                            item->setEquipped(true);
                            equipment->setEquipment(position - 1, item);
                        }
                    }
                    break;
                    // Can I use the item?
                case 0x00a8:
                    if (RFIFOB(6) == 0) 
                    {
                        chatWindow->chat_log("Failed to use item", BY_SERVER);
                    }
                    else
                    {
                        inventory->getItem(RFIFOW(2))->setQuantity(RFIFOW(4));
                    }
                    break;
                    // Warp
                case 0x0091:
                    memset(map_path, '\0', 480);
                    strcat(map_path, "maps/");
                    strncat(map_path, RFIFOP(2), 497 - strlen(map_path));
                    logger->log("Warping to %s (%d, %d)",
                            map_path, RFIFOW(18), RFIFOW(20));
                    strcpy(strrchr(map_path, '.') + 1, "tmx.gz");

                    Map *oldMap;
                    oldMap = tiledMap;
                    tiledMap = MapReader::readMap(map_path);

                    if (tiledMap)
                    {
                        // Delete all beings except the local player
                        std::list<Being *>::iterator i;
                        for (i = beings.begin(); i != beings.end(); i++)
                        {
                            if ((*i) != player_node)
                            {
                                delete (*i);
                            }
                        }
                        beings.clear();
                        autoTarget = NULL;

                        // Re-add the local player node
                        add_node(player_node);

                        player_node->action = Being::STAND;
                        player_node->frame = 0;
                        player_node->x = RFIFOW(18);
                        player_node->y = RFIFOW(20);
                        player_node->setMap(tiledMap);
                        current_npc = 0;
                        // Send "map loaded"
                        WFIFOW(0) = net_w_value(0x007d);
                        WFIFOSET(2);
                        while (out_size > 0) flush();
                        engine->setCurrentMap(tiledMap);
                    }
                    else
                    {
                        logger->error("Could not find map file");
                    }
                    if (oldMap) delete oldMap;
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
                        case 0x0018:
                            char_info->totalWeight = RFIFOW(4);
                            break;
                        case 0x0019:
                            char_info->maxWeight = RFIFOW(4);
                            break;
                        case 0x0037:
                            char_info->job_lv = RFIFOW(4);
                            break;
                        case 0x0009:
                            char_info->statsPointsToAttribute = RFIFOW(4);
                            break;
                        case 0x0035:
                            player_node->aspd = RFIFOW(4);
                            break;
                    }
                    statusWindow->update();
                    if (char_info->hp == 0 && deathNotice == NULL) {
                        deathNotice = new OkDialog("Message",
                                "You're now dead, press ok to restart",
                                &deathNoticeListener);
                        deathNotice->releaseModalFocus();
                        player_node->action = Being::DEAD;
                    }
                    break;
                    // Stop walking
                // case 0x0088:  // Disabled because giving some problems
                    //if (being = findNode(RFIFOL(2))) {
                    //    if (being->getId()!=player_node->getId()) {
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

                                if (RFIFOL(2) != player_node->getId()) { // buggy
                                    being = findNode(RFIFOL(2));
                                    if (being) {
                                        being->action = Being::ATTACK;
                                        being->frame = 0;
                                        being->walk_time = tick_time;
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
                                    being->action = Being::SIT;
                                }
                                else if (RFIFOB(26) == 3) {
                                    being->action = Being::STAND;
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
                    logger->log("Level up");
                    if (RFIFOL(2) == player_node->getId()) {
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
                    // Add item to inventory after you bought it/picked up
                case 0x00a0:
                    if (RFIFOB(22) > 0)
                        chatWindow->chat_log("Unable to pick up item", BY_SERVER);
                    else {
                        inventory->addItem(RFIFOW(2), RFIFOW(6),
                                RFIFOW(4), RFIFOW(19) != 0);
                    }
                    break;
                    // Decrease quantity of an item in inventory
                case 0x00af:
                    inventory->getItem(RFIFOW(2))->increaseQuantity(-RFIFOW(4));
                    break;
                    // Use an item
                case 0x01c8:
                    inventory->getItem(RFIFOW(2))->setQuantity( RFIFOW(10));
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
                                        RFIFOB(4 + k * 37 + 36));
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
                    // Display MVP player
                case 0x010c:
                    chatWindow->chat_log("MVP player", BY_SERVER);
                    break;
                    // Item found/dropped
                case 0x009d:
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
                    remove_floor_item(net_l_value(RFIFOL(2)));
                    break;
                    // Next/Close button in NPC dialog
                case 0x00b5:
                case 0x00b6:
                    // Unused
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
                        switch (RFIFOB(6)) {
                            case 1:
                                being->setHairStyle(RFIFOB(7));
                                break;
                            case 2:
                                being->setWeapon(RFIFOB(7));
                             break;
                            case 6:
                                being->setHairColor(RFIFOB(7));
                                break;
                        }
                    }
                    break;

                    // Answer to equip item
                case 0x00aa:
                    logger->log("Equipping: %i %i %i", RFIFOW(2), RFIFOW(4), RFIFOB(6));
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
                            logger->log("Position %i", position-1);
                            Item *item = equipment->getEquipment(position - 1);
                            if (item)
                                item->setEquipped(false);

                            item = inventory->getItem(RFIFOW(2));
                            item->setEquipped(true);
                            equipment->setEquipment(position - 1, item);
                            player_node->setWeaponById(item->getId());
                        }
                    }
                    break;
                    // Equipment related
                case 0x01d7:
                    being = findNode(RFIFOL(2));
                    if (being != NULL)
                    {
                        being->setWeaponById(RFIFOW(7));
                    }
                    //logger->log("1d7 %i %i %i %i", RFIFOL(2), RFIFOB(6), RFIFOW(7), RFIFOW(9));
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

                            Item *item = inventory->getItem(RFIFOW(2));
                            item->setEquipped(false);
                            switch (item->getId()) {
                                case 529:
                                case 1199:
                                    equipment->setArrows(NULL);
                                    break;
                                case 521:
                                case 522:
                                case 530:
                                case 536:
                                case 1200:
                                case 1201:
                                    player_node->setWeapon(0);
                                    break; // TODO : why this break ? shouldn't a weapon by unequiped in inventory too ?
                                default:
                                    equipment->removeEquipment(position - 1);
                                    break;
                            }
                            logger->log("Unequipping: %i %i(%i) %i", RFIFOW(2),RFIFOW(4),RFIFOB(6), position -1);
                        }
                    }
                    break;
                    // Arrows equipped
                case 0x013c:
                    if (RFIFOW(2) > 1) {
                        Item *item = inventory->getItem(RFIFOW(2));
                        item->setEquipped(true);
                        equipment->setArrows(item);
                        logger->log("Arrows equipped: %i", RFIFOW(2));
                    }
                    break;
                    // Various messages
                case 0x013b:
                    if (RFIFOW(2) == 0)
                        chatWindow->chat_log("Equip arrows first", BY_SERVER);
                    else
                        logger->log("0x013b: Unhandled message %i", RFIFOW(2));
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
                    {
                        //std::cout << RFIFOL(2) << " is " << RFIFOP(6) << std::endl;
                        strcpy(being->name, RFIFOP(6));
                    }
                    break;

                // Change in players look
                case 0x0119:
                    //std::cout << RFIFOL(2) << " " << RFIFOW(6) << " " << RFIFOW(8) << " " << RFIFOW(10) << " " << RFIFOW(12)
                    //<< " " << RFIFOW(14) << " " << RFIFOW(16) << " " << RFIFOW(18) << " " << RFIFOW(20) << " end" << std::endl;
                    // Manage non implemented packets
                default:
                    logger->log("Unhandled packet: %x", id);
                    break;
            }

            RFIFOSKIP(len);
        }
    }
}
