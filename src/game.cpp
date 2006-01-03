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

#include <fstream>
#include <physfs.h>
#include <sstream>
#include <string>
#include <SDL_types.h>

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
#include "gui/chargedialog.h"
#include "gui/chat.h"
#include "gui/confirm_dialog.h"
#include "gui/equipmentwindow.h"
#include "gui/gui.h"
#include "gui/help.h"
#include "gui/inventorywindow.h"
#include "gui/minimap.h"
#include "gui/npc.h"
#include "gui/npc_text.h"
#include "gui/ok_dialog.h"
#include "gui/requesttrade.h"
#include "gui/sell.h"
#include "gui/setup.h"
#include "gui/skill.h"
#include "gui/menuwindow.h"
#include "gui/status.h"
#include "gui/ministatus.h"
#include "gui/trade.h"
#include "gui/debugwindow.h"

#include "net/messagein.h"
#include "net/messageout.h"
#include "net/network.h"
#include "net/protocol.h"

#include "resources/imagewriter.h"

extern Graphics *graphics;

std::string map_path;
std::string tradePartnerName;

bool refresh_beings = false;
unsigned char keyb_state;
volatile int tick_time;
volatile bool action_time = false;
int server_tick;
int fps = 0, frame = 0, current_npc = 0;
bool displayPathToMouse = false;
Uint16 startX = 0, startY = 0;
Being *autoTarget = NULL;
Engine *engine = NULL;
SDL_Joystick *joypad = NULL;       /**< Joypad object */

OkDialog *weightNotice = NULL;
OkDialog *deathNotice = NULL;
ConfirmDialog *exitConfirm = NULL;

ChatWindow *chatWindow;
MenuWindow *menuWindow;
StatusWindow *statusWindow;
MiniStatusWindow *miniStatusWindow;
BuyDialog *buyDialog;
SellDialog *sellDialog;
BuySellDialog *buySellDialog;
InventoryWindow *inventoryWindow;
NpcListDialog *npcListDialog;
NpcTextDialog *npcTextDialog;
SkillDialog *skillDialog;
//NewSkillDialog *newSkillWindow;
Setup* setupWindow;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
ChargeDialog *chargeDialog;
TradeWindow *tradeWindow;
//BuddyWindow *buddyWindow;
HelpWindow *helpWindow;
DebugWindow *debugWindow;

Inventory *inventory = NULL;

const int EMOTION_TIME = 150;    /**< Duration of emotion icon */
const int MAX_TIME = 10000;

class WeightNoticeListener : public gcn::ActionListener
{
    public:
        void action(const std::string &eventId)
        {
            weightNotice = NULL;
        }
} weightNoticeListener;


/**
 * Listener used for handling death message.
 */
class DeathNoticeListener : public gcn::ActionListener {
    public:
        void action(const std::string &eventId) {
            MessageOut outMsg;
            outMsg.writeInt16(0x00b2);
            outMsg.writeInt8(0);
            deathNotice = NULL;
        }
} deathNoticeListener;

/**
 * Listener used for exitting handling.
 */
class ExitListener : public gcn::ActionListener {
    void action(const std::string &eventId) {
        if (eventId == "yes") {
            state = EXIT_STATE;
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

/**
 * Create all the various globally accessible gui windows
 */
void createGuiWindows()
{
    // Create dialogs
    chatWindow = new ChatWindow(
            config.getValue("homeDir", "") + std::string("/chatlog.txt"));
    menuWindow = new MenuWindow();
    statusWindow = new StatusWindow();
    miniStatusWindow = new MiniStatusWindow();
    buyDialog = new BuyDialog();
    sellDialog = new SellDialog();
    buySellDialog = new BuySellDialog();
    inventoryWindow = new InventoryWindow();
    npcTextDialog = new NpcTextDialog();
    npcListDialog = new NpcListDialog();
    skillDialog = new SkillDialog();
    //newSkillWindow = new NewSkillDialog();
    setupWindow = new Setup();
    minimap = new Minimap();
    equipmentWindow = new EquipmentWindow();
    chargeDialog = new ChargeDialog();
    tradeWindow = new TradeWindow();
    //buddyWindow = new BuddyWindow();
    helpWindow = new HelpWindow();
    debugWindow = new DebugWindow();

    // Initialize window positions
    int screenW = graphics->getWidth();
    int screenH = graphics->getHeight();

    chargeDialog->setPosition(
            screenW - 5 - chargeDialog->getWidth(),
            screenH - chargeDialog->getHeight() - 15);
    tradeWindow->setPosition(screenW - statusWindow->getWidth() -
            tradeWindow->getWidth() - 10,
            inventoryWindow->getY() + inventoryWindow->getHeight());
    /*buddyWindow->setPosition(10,
      minimap->getHeight() + 30);*/

    // Set initial window visibility
    chatWindow->setVisible(true);
    miniStatusWindow->setVisible(true);
    statusWindow->setVisible(false);
    menuWindow->setVisible(true);
    buyDialog->setVisible(false);
    sellDialog->setVisible(false);
    buySellDialog->setVisible(false);
    inventoryWindow->setVisible(false);
    npcTextDialog->setVisible(false);
    npcListDialog->setVisible(false);
    skillDialog->setVisible(false);
    //newSkillWindow->setVisible(false);
    setupWindow->setVisible(false);
    equipmentWindow->setVisible(false);
    chargeDialog->setVisible(false);
    tradeWindow->setVisible(false);
    //buddyWindow->setVisible(false);
    helpWindow->setVisible(false);
    debugWindow->setVisible(false);

    // Do not focus any text field
    gui->focusNone();
}

/**
 * Destroy all the globally accessible gui windows
 */
void destroyGuiWindows()
{
    delete chatWindow;
    delete statusWindow;
    delete miniStatusWindow;
    delete menuWindow;
    delete buyDialog;
    delete sellDialog;
    delete buySellDialog;
    delete inventoryWindow;
    delete npcListDialog;
    delete npcTextDialog;
    delete skillDialog;
    delete setupWindow;
    delete minimap;
    delete equipmentWindow;
    delete chargeDialog;
    //delete newSkillWindow;
    delete tradeWindow;
    //delete buddyWindow;
    delete helpWindow;
    delete debugWindow;
}

void do_init()
{
    engine->changeMap(map_path);

    // Initialize timers
    tick_time = 0;
    SDL_AddTimer(10, nextTick, NULL);                     // Logic counter
    SDL_AddTimer(1000, nextSecond, NULL);                 // Seconds counter

    // Initialize beings
    player_node = createBeing(account_ID, 0, engine->getCurrentMap());
    player_node->x = startX;
    player_node->y = startY;
    player_node->setHairColor(player_info->hairColor);
    player_node->setHairStyle(player_info->hairStyle);

    if (player_info->weapon == 11)
    {
        player_info->weapon = 2;
    }

    player_node->setWeapon(player_info->weapon);

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

bool saveScreenshot(SDL_Surface *screenshot)
{
    static unsigned int screenshotCount = 0;

    // Search for an unused screenshot name
    std::stringstream filename;
    std::fstream testExists;
    bool found = false;

    do {
        screenshotCount++;
        filename.str("");
#ifdef __USE_UNIX98
        filename << PHYSFS_getUserDir() << "/";
#endif
        filename << "TMW_Screenshot_" << screenshotCount << ".png";
        testExists.open(filename.str().c_str(), std::ios::in);
        found = !testExists.is_open();
        testExists.close();
    } while (!found);

    return ImageWriter::writePNG(screenshot, filename.str());
}

void game()
{
    // Needs to be initialised _before_ the engine is created...
    inventory = new Inventory();

    createGuiWindows();
    engine = new Engine();
    do_init();

    int gameTime = tick_time;

    while (state == GAME_STATE)
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
            frame++;
            engine->draw(graphics);
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

    do_exit();
}

void do_exit()
{
    delete engine;
    delete player_node;
    destroyGuiWindows();
    closeConnection();

    delete inventory;

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

    // TODO: Only <= 6 buttons joypads are allowed
    bool joy[10];
    memset(joy, 0, 10 * sizeof(bool));

    // Get the state of the joypad axis/buttons
    if (joypad != NULL)
    {
        // TODO: one different value of tolerance is needed for each direction
        // This probably means the need for a tuning utility/window
        int tolerance = (int)config.getValue("joytolerance", 100);
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
            gcn::Window *requestedWindow = NULL;

            switch (event.key.keysym.sym)
            {
                case SDLK_F1:
                    // In-game Help
                    if (helpWindow->isVisible())
                    {
                        helpWindow->setVisible(false);
                    }
                    else
                    {
                        helpWindow->loadHelp("index");
                        helpWindow->requestMoveToTop();
                    }
                    used = true;
                    break;

                case SDLK_F2: requestedWindow = statusWindow; break;
                case SDLK_F3: requestedWindow = inventoryWindow; break;
                case SDLK_F4: requestedWindow = equipmentWindow; break;
                case SDLK_F5: requestedWindow = skillDialog; break;
                case SDLK_F6: requestedWindow = minimap; break;
                case SDLK_F7: requestedWindow = chatWindow; break;
                //case SDLK_F8: requestedWindow = buddyWindow; break;
                case SDLK_F9: requestedWindow = setupWindow; break;
                case SDLK_F10: requestedWindow = debugWindow; break;
                //case SDLK_F11: requestedWindow = newSkillWindow; break;

                case SDLK_RETURN:
                    // Input chat window
                    if (chatWindow->isFocused() ||
                        deathNotice != NULL ||
                        weightNotice != NULL)
                    {
                        break;
                    }

                    // Quit by pressing Enter if the exit confirm is there
                    if (exitConfirm)
                    {
                        state = EXIT_STATE;
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
                    if (!chatWindow->isFocused())
                    {
                        Uint32 id = find_floor_item_by_cor(
                                player_node->x, player_node->y);

                        // If none below the player, try the tile in front of
                        // the player
                        if (!id) {
                            Uint16 x = player_node->x;
                            Uint16 y = player_node->y;

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
                            pickUp(id);

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
                    exitConfirm->requestMoveToTop();
                    break;

                default:
                    break;
            }

            if (requestedWindow)
            {
                requestedWindow->setVisible(!requestedWindow->isVisible());
                if (requestedWindow->isVisible())
                {
                    requestedWindow->requestMoveToTop();
                }
                used = true;
            }

            // Keys pressed together with Alt/Meta
            // Emotions and some internal gui windows
            if (event.key.keysym.mod & KMOD_ALT)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_s:
                        // Player sit action
                        if (!action_time) {
                            break;
                        }

                        switch (player_node->action)
                        {
                            case Being::STAND: action(2, 0); break;
                            case Being::SIT: action(3, 0); break;
                        }
                        used = true;
                        break;

                    case SDLK_p:
                        // Screenshot (picture, hence the p)
                        {
                            SDL_Surface *screenshot = graphics->getScreenshot();
                            if (!saveScreenshot(screenshot))
                            {
                                logger->log("Error: could not save Screenshot.");
                            }
                            SDL_FreeSurface(screenshot);
                        }
                        used = true;
                        break;

                    default:
                        break;

                    case SDLK_f:
                        // Find path to mouse (debug purpose)
                        displayPathToMouse = !displayPathToMouse;
                        used = true;
                        break;
                }

                // Emotions
                if (action_time && !player_node->emotion)
                {
                    Uint8 emotion = 0;
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
                        MessageOut outMsg;
                        outMsg.writeInt16(0x00bf);
                        outMsg.writeInt8(emotion);
                        action_time = false;
                        used = true;
                    }
                }
            }
        }

        // Quit event
        else if (event.type == SDL_QUIT)
        {
            state = EXIT_STATE;
        }

        // Push input to GUI when not used
        if (!used) {
            guiInput->pushInput(event);
        }

    } // End while

    // Moving player around
    if (player_node->action != Being::DEAD &&
        current_npc == 0 &&
        !chatWindow->isFocused())
    {
        Uint16 x = player_node->x;
        Uint16 y = player_node->y;
        Sint16 xDirection = 0;
        Sint16 yDirection = 0;
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
        if ((xDirection != 0 || yDirection != 0) &&
            player_node->action == Being::WALK)
        {
            player_node->setDestination(x, y);
        }

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
                if (monster == NULL && autoTarget != NULL &&
                    monster != player_node)
                {
                    attack(autoTarget);
                }
                else if ((keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]) && monster != player_node)
                {
                    autoTarget = monster;
                }
            }
        }

        if (joy[JOY_BTN1])
        {
            Uint32 id = find_floor_item_by_cor(player_node->x, player_node->y);

            if (id)
                pickUp(id);
        }
        else if (joy[JOY_BTN2] && action_time)
        {
            if (player_node->action == Being::STAND)
                action(2, 0);
            else if (player_node->action == Being::SIT)
                action(3, 0);
            action_time = false;
        }
    }
}

void do_parse()
{
    Map *tiledMap = engine->getCurrentMap();
    Equipment *equipment = Equipment::getInstance();

    int n_items;
    Being *being;

    // We need at least 2 bytes to identify a packet
    while (in_size >= 2)
    {
        MessageIn msg = get_next_message();

        // Parse packet based on their id
        switch (msg.getId())
        {
            case SMSG_LOGIN_SUCCESS:
                // Connected to game server succesfully, set spawn point
                msg.readInt32(); // server tick
                msg.readCoordinates(player_node->x, player_node->y,
                        player_node->direction);
                msg.skip(2);    // unknown
                break;

                // Received speech from being
            case SMSG_BEING_CHAT:
                {
                    Sint16 chatMsgLength = msg.readInt16() - 8;
                    being = findNode(msg.readInt32());

                    if (!being || chatMsgLength <= 0)
                    {
                        break;
                    }

                    std::string chatMsg = msg.readString(chatMsgLength);

                    chatWindow->chatLog(chatMsg, BY_OTHER);

                    chatMsg.erase(0, chatMsg.find(" : ", 0) + 3);
                    being->setSpeech(chatMsg, SPEECH_TIME);
                }
                break;

            case SMSG_PLAYER_CHAT:
            case SMSG_GM_CHAT:
                {
                    Sint16 chatMsgLength = msg.readInt16() - 4;

                    if (chatMsgLength <= 0)
                    {
                        break;
                    }

                    std::string chatMsg = msg.readString(chatMsgLength);

                    if (msg.getId() == SMSG_PLAYER_CHAT)
                    {
                        chatWindow->chatLog(chatMsg, BY_PLAYER);

                        std::string::size_type pos = chatMsg.find(" : ", 0);
                        if (pos != std::string::npos)
                        {
                            chatMsg.erase(0, pos + 3);
                        }
                        player_node->setSpeech(chatMsg, SPEECH_TIME);
                    }
                    else
                    {
                        chatWindow->chatLog(chatMsg, BY_GM);
                    }
                }
                break;

            case SMSG_WALK_RESPONSE:
                // It is assumed by the client any request to walk actually
                // succeeds on the server. The plan is to have a correction
                // message when the server senses the client has the wrong
                // idea.
                break;

            case SMSG_BEING_VISIBLE:
            case SMSG_BEING_MOVE:
                // Information about a being in range
                {
                    Uint32 id = msg.readInt32();
                    Uint16 speed = msg.readInt16();
                    msg.readInt16();  // unknown
                    msg.readInt16();  // unknown
                    msg.readInt16();  // option
                    Uint16 job = msg.readInt16();  // class

                    being = findNode(id);

                    if (being == NULL)
                    {
                        // Being with id >= 110000000 and job 0 are better
                        // known as ghosts, so don't create those.
                        if (job == 0 && id >= 110000000)
                        {
                            break;
                        }

                        being = createBeing(id, job, tiledMap);
                    }
                    else if (msg.getId() == 0x0078)
                    {
                        being->clearPath();
                        being->mFrame = 0;
                        being->walk_time = tick_time;
                        being->action = Being::STAND;
                    }

                    // Prevent division by 0 when calculating frame
                    if (speed == 0) { speed = 150; }

                    being->setWalkSpeed(speed);
                    being->job = job;
                    being->setHairStyle(msg.readInt16());
                    being->setWeapon(msg.readInt16());
                    msg.readInt16();  // head option bottom

                    if (msg.getId() == SMSG_BEING_MOVE)
                    {
                        msg.readInt32(); // server tick
                    }

                    msg.readInt16();  // shield
                    msg.readInt16();  // head option top
                    msg.readInt16();  // head option mid
                    being->setHairColor(msg.readInt16());
                    msg.readInt16();  // unknown
                    msg.readInt16();  // head dir
                    msg.readInt16();  // guild
                    msg.readInt16();  // unknown
                    msg.readInt16();  // unknown
                    msg.readInt16();  // manner
                    msg.readInt16();  // karma
                    msg.readInt8();   // unknown
                    msg.readInt8();   // sex

                    if (msg.getId() == SMSG_BEING_MOVE)
                    {
                        Uint16 srcX, srcY, dstX, dstY;
                        msg.readCoordinatePair(srcX, srcY, dstX, dstY);
                        being->action = Being::STAND;
                        being->x = srcX;
                        being->y = srcY;
                        being->setDestination(dstX, dstY);
                    }
                    else
                    {
                        msg.readCoordinates(being->x, being->y,
                                            being->direction);
                    }

                    msg.readInt8();   // unknown
                    msg.readInt8();   // unknown
                    msg.readInt8();   // unknown / sit
                }
                break;

            case SMSG_BEING_REMOVE:
                // A being should be removed or has died
                being = findNode(msg.readInt32());

                if (being != NULL)
                {
                    if (msg.readInt8() == 1)
                    {
                        // Death
                        switch (being->getType())
                        {
                            case Being::MONSTER:
                                being->action = Being::MONSTER_DEAD;
                                being->mFrame = 0;
                                being->walk_time = tick_time;
                                break;

                            default:
                                being->action = Being::DEAD;
                                break;
                        }
                    }
                    else
                    {
                        remove_node(being);
                    }

                    if (being == autoTarget)
                    {
                        autoTarget = NULL;
                    }
                }
                break;

            case SMSG_PLAYER_UPDATE_1:
            case SMSG_PLAYER_UPDATE_2:
            case SMSG_PLAYER_MOVE:
                // An update about a player, potentially including movement.
                {
                    Uint32 id = msg.readInt32();
                    Uint16 speed = msg.readInt16();
                    msg.readInt16();  // option 1
                    msg.readInt16();  // option 2
                    msg.readInt16();  // option
                    Uint16 job = msg.readInt16();

                    being = findNode(id);

                    if (being == NULL)
                    {
                        being = createBeing(id, job, tiledMap);
                    }

                    being->setWalkSpeed(speed);
                    being->job = job;
                    being->setHairStyle(msg.readInt16());
                    being->setWeaponById(msg.readInt16());  // item id 1
                    msg.readInt16();  // item id 2
                    msg.readInt16();  // head option bottom

                    if (msg.getId() == SMSG_PLAYER_MOVE)
                    {
                        msg.readInt32(); // server tick
                    }

                    msg.readInt16();  // head option top
                    msg.readInt16();  // head option mid
                    being->setHairColor(msg.readInt16());
                    msg.readInt16();  // unknown
                    msg.readInt16();  // head dir
                    msg.readInt32();   // guild
                    msg.readInt32();   // emblem
                    msg.readInt16();  // manner
                    msg.readInt8();   // karma
                    msg.readInt8();   // sex

                    if (msg.getId() == SMSG_PLAYER_MOVE)
                    {
                        Uint16 srcX, srcY, dstX, dstY;
                        msg.readCoordinatePair(srcX, srcY, dstX, dstY);
                        being->x = srcX;
                        being->y = srcY;
                        being->setDestination(dstX, dstY);
                    }
                    else
                    {
                        msg.readCoordinates(being->x, being->y,
                                            being->direction);
                    }

                    msg.readInt8();   // unknown
                    msg.readInt8();   // unknown

                    if (msg.getId() == SMSG_PLAYER_UPDATE_1)
                    {
                        if (msg.readInt8() == 2)
                        {
                            being->action = Being::SIT;
                        }
                    }
                    else if (msg.getId() == SMSG_PLAYER_MOVE)
                    {
                        msg.readInt8(); // unknown
                    }

                    msg.readInt8();   // Lv
                    msg.readInt8();   // unknown

                    being->walk_time = tick_time;
                    being->mFrame = 0;
                }
                break;

            case SMSG_NPC_MESSAGE:
                msg.readInt16();  // length
                current_npc = msg.readInt32();
                npcTextDialog->addText(msg.readString(msg.getLength() - 8));
                npcListDialog->setVisible(false);
                npcTextDialog->setVisible(true);
                break;

            case SMSG_NPC_NEXT:
            case SMSG_NPC_CLOSE:
                // Next/Close button in NPC dialog, currently unused
                break;

            case SMSG_TRADE_REQUEST:
                // If a trade window or request window is already open, send a
                // trade cancel to any other trade request.
                //
                // Note that it would be nice if the server would prevent this
                // situation, and that the requesting player would get a
                // special message about the player being occupied.

                if (tradeWindow->isVisible() || requestTradeDialogOpen)
                {
                    MessageOut outMsg;
                    outMsg.writeInt16(CMSG_TRADE_RESPONSE);
                    outMsg.writeInt8(4);
                    break;
                }

                requestTradeDialogOpen = true;
                tradePartnerName = msg.readString(24);
                new RequestTradeDialog(tradePartnerName);
                break;

            case SMSG_TRADE_RESPONSE:
                switch (msg.readInt8())
                {
                    case 0: // Too far away
                        chatWindow->chatLog("Trading isn't possible. "
                                             "Trade partner is too far away.",
                                             BY_SERVER);
                        break;
                    case 1: // Character doesn't exist
                        chatWindow->chatLog("Trading isn't possible. "
                                             "Character doesn't exist.",
                                             BY_SERVER);
                        break;
                    case 2: // Invite request check failed...
                        chatWindow->chatLog("Trade cancelled due to an "
                                             "unknown reason.", BY_SERVER);
                        break;
                    case 3: // Trade accepted
                        tradeWindow->reset();
                        tradeWindow->setCaption(
                                "Trade: You and " + tradePartnerName);
                        tradeWindow->setVisible(true);
                        requestTradeDialogOpen = false;
                        break;
                    case 4: // Trade cancelled
                        chatWindow->chatLog("Trade cancelled.", BY_SERVER);
                        tradeWindow->setVisible(false);
                        break;
                    default: // Shouldn't happen as well, but to be sure
                        chatWindow->chatLog("Unhandled trade cancel packet",
                                             BY_SERVER);
                        break;
                }
                break;

            case SMSG_TRADE_ITEM_ADD:
                {
                    Sint32 amount = msg.readInt32();
                    Sint16 type = msg.readInt16();
                    msg.readInt8();  // identified flag
                    msg.readInt8();  // attribute
                    msg.readInt8();  // refine
                    msg.skip(8);     // card (4 shorts)

                    // TODO: handle also identified, etc
                    if (type == 0) {
                        tradeWindow->addMoney(amount);
                    } else {
                        tradeWindow->addItem(type, false, amount, false);
                    }
                }
                break;

            case SMSG_TRADE_ITEM_ADD_RESPONSE:
                // Trade: New Item add response (was 0x00ea, now 01b1)
                {
                    Item *item = inventory->getItem(msg.readInt16());
                    Sint16 quantity = msg.readInt16();

                    switch (msg.readInt8())
                    {
                        case 0:
                            // Successfully added item
                            if (item->isEquipment() && item->isEquipped())
                            {
                                inventory->unequipItem(item);
                            }
                            tradeWindow->addItem(item->getId(), true, quantity,
                                                 item->isEquipment());
                            item->increaseQuantity(-quantity);
                            break;
                        case 1:
                            // Add item failed - player overweighted
                            chatWindow->chatLog("Failed adding item. Trade "
                                                 "partner is over weighted.",
                                                 BY_SERVER);
                            break;
                        default:
                            chatWindow->chatLog("Failed adding item for "
                                                 "unknown reason.", BY_SERVER);
                            break;
                    }
                }
                break;

            case SMSG_TRADE_OK:
                // 0 means ok from myself, 1 means ok from other;
                tradeWindow->receivedOk(msg.readInt8() == 0);
                break;

            case SMSG_TRADE_CANCEL:
                chatWindow->chatLog("Trade canceled.", BY_SERVER);
                tradeWindow->setVisible(false);
                tradeWindow->reset();
                break;

            case SMSG_TRADE_COMPLETE:
                chatWindow->chatLog("Trade completed.", BY_SERVER);
                tradeWindow->setVisible(false);
                tradeWindow->reset();
                break;

            case SMSG_PLAYER_INVENTORY:
                {
                    // Only called on map load / warp. First reset all items
                    // to not load them twice on map change.
                    inventory->resetItems();
                    msg.readInt16();  // length
                    Sint32 number = (msg.getLength() - 4) / 18;

                    for (int loop = 0; loop < number; loop++)
                    {
                        Sint16 index = msg.readInt16();
                        Sint16 itemId = msg.readInt16();
                        msg.readInt8(); // type
                        msg.readInt8(); // identify flag
                        Sint16 amount = msg.readInt16();
                        msg.skip(2);    // unknown
                        msg.skip(8);    // card (4 shorts)

                        inventory->addItem(index, itemId, amount, false);

                        // Trick because arrows are not considered equipment
                        if (itemId == 1199 || itemId == 529)
                        {
                            inventory->getItem(index)->setEquipment(true);
                        }
                    }
                }
                break;

            case SMSG_PLAYER_EQUIPMENT:
                {
                    msg.readInt16(); // length
                    Sint32 number = (msg.getLength() - 4) / 20;

                    for (int loop = 0; loop < number; loop++)
                    {
                        Sint16 index = msg.readInt16();
                        Sint16 itemId = msg.readInt16();
                        msg.readInt8();  // type
                        msg.readInt8();  // identify flag
                        msg.readInt16(); // equip type
                        Sint16 equipPoint = msg.readInt16();
                        msg.readInt8();  // attribute
                        msg.readInt8();  // refine
                        msg.skip(8);     // card

                        inventory->addItem(index, itemId, 1, true);

                        if (equipPoint)
                        {
                            int mask = 1;
                            int position = 0;
                            while (!(equipPoint & mask))
                            {
                                mask <<= 1;
                                position++;
                            }
                            Item *item = inventory->getItem(index);
                            item->setEquipped(true);
                            equipment->setEquipment(position - 1, item);
                        }
                    }
                }
                break;

            case SMSG_ITEM_USE_RESPONSE:
                {
                    Sint16 index = msg.readInt16();
                    Sint16 amount = msg.readInt16();

                    if (msg.readInt8() == 0) {
                        chatWindow->chatLog("Failed to use item", BY_SERVER);
                    } else {
                        inventory->getItem(index)->setQuantity(amount);
                    }
                }
                break;

            case SMSG_PLAYER_WARP:
                {
                    // Set new map path
                    map_path = "maps/" + msg.readString(16);
                    map_path = map_path.substr(0, map_path.rfind(".")) +
                               ".tmx.gz";

                    Uint16 x = msg.readInt16();
                    Uint16 y = msg.readInt16();

                    logger->log("Warping to %s (%d, %d)",
                                map_path.c_str(), x, y);

                    // Switch the actual map, deleting the previous one
                    engine->changeMap(map_path);
                    tiledMap = engine->getCurrentMap();

                    autoTarget = NULL;
                    current_npc = 0;

                    player_node->action = Being::STAND;
                    player_node->mFrame = 0;
                    player_node->x = x;
                    player_node->y = y;

                    // Send "map loaded"
                    MessageOut outMsg;
                    outMsg.writeInt16(CMSG_MAP_LOADED);
                }
                break;

            case SMSG_SKILL_FAILED:
                // Action failed (ex. sit because you have not reached the
                // right level)
                CHATSKILL action;
                action.skill   = msg.readInt16();
                action.bskill  = msg.readInt16();
                action.unused  = msg.readInt16(); // unknown
                action.success = msg.readInt8();
                action.reason  = msg.readInt8();
                if (action.success != SKILL_FAILED &&
                    action.bskill == BSKILL_EMOTE)
                {
                    printf("Action: %d/%d", action.bskill, action.success);
                }
                chatWindow->chatLog(action);
                break;

            case SMSG_PLAYER_STAT_UPDATE_1:
                {
                    Sint16 type = msg.readInt16();
                    Sint32 value = msg.readInt32();

                    switch (type)
                    {
                        //case 0x0000:
                        //    player_node->setWalkSpeed(msg.readInt32());
                        //    break;
                        case 0x0005: player_info->hp = value; break;
                        case 0x0006: player_info->maxHp = value; break;
                        case 0x0007: player_info->mp = value; break;
                        case 0x0008: player_info->maxMp = value; break;
                        case 0x000b: player_info->lvl = value; break;
                        case 0x000c:
                            player_info->skillPoint = value;
                            skillDialog->setPoints(player_info->skillPoint);
                            break;
                        case 0x0018:
                            if (value >= player_info->maxWeight / 2 &&
                                player_info->totalWeight <
                                  player_info->maxWeight / 2)
                            {
                                weightNotice = new OkDialog("Message",
                                        "You are carrying more then half your "
                                        "weight. You are unable to regain "
                                        "health.",
                                        &weightNoticeListener);
                            }
                            player_info->totalWeight = value;
                            break;
                        case 0x0019: player_info->maxWeight = value; break;
                        case 0x0037: player_info->jobLvl = value; break;
                        case 0x0009:
                            player_info->statsPointsToAttribute = value;
                            break;
                        case 0x0029: player_info->ATK = value; break;
                        case 0x002b: player_info->MATK = value; break;
                        case 0x002d: player_info->DEF = value; break;
                        case 0x002f: player_info->MDEF = value; break;
                        case 0x0031: player_info->HIT = value; break;
                        case 0x0032: player_info->FLEE = value; break;
                        case 0x0035: player_node->aspd = value; break;
                    }

                    if (player_info->hp == 0 && deathNotice == NULL)
                    {
                        deathNotice = new OkDialog("Message",
                                "You're now dead, press ok to restart",
                                &deathNoticeListener);
                        player_node->action = Being::DEAD;
                    }
                }
                break;

                // Stop walking
                // case 0x0088:  // Disabled because giving some problems
                //if (being = findNode(readInt32(2))) {
                //    if (being->getId() != player_node->getId()) {
                //        being->action = STAND;
                //        being->mFrame = 0;
                //        set_coordinates(being->coordinates,
                //                        readWord(6), readWord(8),
                //                        get_direction(being->coordinates));
                //    }
                //}
                //break;

            case SMSG_BEING_ACTION:
                {
                    Being *srcBeing = findNode(msg.readInt32());
                    Being *dstBeing = findNode(msg.readInt32());
                    msg.readInt32();   // server tick
                    msg.readInt32();   // src speed
                    msg.readInt32();   // dst speed
                    Sint16 param1 = msg.readInt16();
                    msg.readInt16();  // param 2
                    Sint8 type = msg.readInt8();
                    msg.readInt16();  // param 3

                    switch (type)
                    {
                        case 0: // Damage
                            if (dstBeing == NULL) break;

                            dstBeing->setDamage(param1, SPEECH_TIME);

                            if (srcBeing != NULL &&
                                srcBeing != player_node)
                            {
                                // buggy
                                srcBeing->action = Being::ATTACK;
                                srcBeing->mFrame = 0;
                                srcBeing->walk_time = tick_time;
                            }
                            break;

                        case 2: // Sit
                            if (srcBeing == NULL) break;
                            srcBeing->mFrame = 0;
                            srcBeing->action = Being::SIT;
                            break;

                        case 3: // Stand up
                            if (srcBeing == NULL) break;
                            srcBeing->mFrame = 0;
                            srcBeing->action = Being::STAND;
                            break;
                    }
                }
                break;

            case SMSG_PLAYER_STAT_UPDATE_2:
                switch (msg.readInt16()) {
                    case 0x0001:
                        player_info->xp = msg.readInt32();
                        break;
                    case 0x0002:
                        player_info->jobXp = msg.readInt32();
                        break;
                    case 0x0014:
                        player_info->gp = msg.readInt32();
                        break;
                    case 0x0016:
                        player_info->xpForNextLevel = msg.readInt32();
                        break;
                    case 0x0017:
                        player_info->jobXpForNextLevel = msg.readInt32();
                        break;
                }
                break;

            case SMSG_BEING_LEVELUP:
                if ((Uint32)msg.readInt32() == player_node->getId()) {
                    logger->log("Level up");
                    sound.playSfx("sfx/levelup.ogg");
                } else {
                    logger->log("Someone else went level up");
                }
                msg.readInt32();  // type
                break;

            case SMSG_BEING_EMOTION:
                if (!(being = findNode(msg.readInt32())))
                {
                    break;
                }

                being->emotion = msg.readInt8();
                being->emotion_time = EMOTION_TIME;
                break;

            case SMSG_PLAYER_STAT_UPDATE_3:
                {
                    Sint32 type = msg.readInt32();
                    Sint32 base = msg.readInt32();
                    Sint32 bonus = msg.readInt32();
                    Sint32 total = base + bonus;

                    switch (type) {
                        case 0x000d: player_info->STR = total; break;
                        case 0x000e: player_info->AGI = total; break;
                        case 0x000f: player_info->VIT = total; break;
                        case 0x0010: player_info->INT = total; break;
                        case 0x0011: player_info->DEX = total; break;
                        case 0x0012: player_info->LUK = total; break;
                    }
                }
                break;

            case SMSG_NPC_BUY_SELL_CHOICE:
                buyDialog->setVisible(false);
                buyDialog->reset();
                sellDialog->setVisible(false);
                sellDialog->reset();
                buySellDialog->setVisible(true);
                current_npc = msg.readInt32();
                break;

            case SMSG_NPC_BUY:
                msg.readInt16();  // length
                n_items = (msg.getLength() - 4) / 11;
                buyDialog->reset();
                buyDialog->setMoney(player_info->gp);
                buyDialog->setVisible(true);

                for (int k = 0; k < n_items; k++)
                {
                    Sint32 value = msg.readInt32();
                    msg.readInt32();  // DCvalue
                    msg.readInt8();  // type
                    Sint16 itemId = msg.readInt16();
                    buyDialog->addItem(itemId, value);
                }
                break;

            case SMSG_NPC_SELL:
                msg.readInt16();  // length
                n_items = (msg.getLength() - 4) / 10;
                if (n_items > 0) {
                    sellDialog->reset();
                    sellDialog->setVisible(true);

                    for (int k = 0; k < n_items; k++)
                    {
                        Sint16 index = msg.readInt16();
                        Sint32 value = msg.readInt32();
                        msg.readInt32();  // OCvalue

                        Item *item = inventory->getItem(index);
                        if (item && !(item->isEquipped())) {
                            sellDialog->addItem(item, value);
                        }
                    }
                }
                else {
                    chatWindow->chatLog("Nothing to sell", BY_SERVER);
                    current_npc = 0;
                }
                break;

            case SMSG_NPC_BUY_RESPONSE:
                if (msg.readInt8() == 0) {
                    chatWindow->chatLog("Thanks for buying", BY_SERVER);
                } else {
                    chatWindow->chatLog("Unable to buy", BY_SERVER);
                }
                break;

            case SMSG_NPC_SELL_RESPONSE:
                if (msg.readInt8() == 0) {
                    chatWindow->chatLog("Thanks for selling", BY_SERVER);
                } else {
                    chatWindow->chatLog("Unable to sell", BY_SERVER);
                }
                break;

            case SMSG_PLAYER_INVENTORY_ADD:
                {
                    Sint16 index = msg.readInt16();
                    Sint16 amount = msg.readInt16();
                    Sint16 itemId = msg.readInt16();
                    msg.readInt8();  // identify flag
                    msg.readInt8();  // attribute
                    msg.readInt8();  // refine
                    msg.skip(8);     // card
                    Sint16 equipType = msg.readInt16();
                    msg.readInt8();  // type
                    Sint8 fail = msg.readInt8();

                    if (fail > 0) {
                        chatWindow->chatLog("Unable to pick up item",
                                             BY_SERVER);
                    } else {
                        inventory->addItem(index, itemId, amount,
                                           equipType != 0);
                    }
                }
                break;

            case SMSG_PLAYER_INVENTORY_REMOVE:
                {
                    Sint16 index = msg.readInt16();
                    Sint16 amount = msg.readInt16();
                    inventory->getItem(index)->increaseQuantity(-amount);
                }
                break;

            case SMSG_PLAYER_INVENTORY_USE:
                {
                    Sint16 index = msg.readInt16();
                    msg.readInt16(); // item id
                    msg.readInt32();  // id
                    Sint16 amountLeft = msg.readInt16();
                    msg.readInt8();  // type

                    inventory->getItem(index)->setQuantity(amountLeft);
                }
                break;

            case SMSG_PLAYER_SKILLS:
                msg.readInt16();  // length
                n_items = (msg.getLength() - 4) / 37;
                skillDialog->cleanList();

                for (int k = 0; k < n_items; k++)
                {
                    Sint16 skillId = msg.readInt16();
                    msg.readInt16();  // target type
                    msg.readInt16();  // unknown
                    Sint16 level = msg.readInt16();
                    Sint16 sp = msg.readInt16();
                    msg.readInt16();  // range
                    std::string skillName = msg.readString(24);
                    Sint8 up = msg.readInt8();

                    if (level != 0 || up != 0)
                    {
                        if (skillDialog->hasSkill(skillId)) {
                            skillDialog->setSkill(skillId, level, sp);
                        }
                        else {
                            skillDialog->addSkill(skillId, level, sp);
                        }
                    }
                }
                break;

            case 0x010c:
                // Display MVP player
                msg.readInt32(); // id
                chatWindow->chatLog("MVP player", BY_SERVER);
                break;

            case SMSG_ITEM_VISIBLE:
            case SMSG_ITEM_DROPPED:
                {
                    Uint32 id = msg.readInt32();
                    Sint16 itemId = msg.readInt16();
                    msg.readInt8();  // identify flag
                    Uint16 x = msg.readInt16();
                    Uint16 y = msg.readInt16();
                    msg.skip(4);     // amount,subX,subY / subX,subY,amount

                    add_floor_item(new FloorItem(id, itemId, x, y, tiledMap));
                }
                break;

            case SMSG_ITEM_REMOVE:
                remove_floor_item(msg.readInt32());
                break;

            case SMSG_NPC_CHOICE:
                msg.readInt16();  // length
                current_npc = msg.readInt32();
                npcListDialog->parseItems(msg.readString(msg.getLength() - 8));
                npcListDialog->setVisible(true);
                break;

            case SMSG_BEING_CHANGE_LOOKS:
                if (!(being = findNode(msg.readInt32())))
                {
                    break;
                }

                switch (msg.readInt8()) {
                    case 1:
                        being->setHairStyle(msg.readInt8());
                        break;
                    case 2:
                        being->setWeapon(msg.readInt8());
                        break;
                    case 6:
                        being->setHairColor(msg.readInt8());
                        break;
                    default:
                        msg.readInt8(); // unsupported
                        break;
                }
                break;

            case SMSG_PLAYER_EQUIP:
                {
                    Sint16 index = msg.readInt16();
                    Sint16 equipPoint = msg.readInt16();
                    Sint8 type = msg.readInt8();

                    logger->log("Equipping: %i %i %i",
                                index, equipPoint, type);

                    if (type == 0) {
                        chatWindow->chatLog("Unable to equip.", BY_SERVER);
                    }
                    else if (equipPoint)
                    {
                        // Unequip any existing equipped item in this position
                        int mask = 1;
                        int position = 0;
                        while (!(equipPoint & mask)) {
                            mask <<= 1;
                            position++;
                        }
                        logger->log("Position %i", position - 1);
                        Item *item = equipment->getEquipment(position - 1);
                        if (item) {
                            item->setEquipped(false);
                        }

                        item = inventory->getItem(index);
                        item->setEquipped(true);
                        equipment->setEquipment(position - 1, item);
                        player_node->setWeaponById(item->getId());
                    }
                }
                break;

            case 0x01d7:
                // Equipment related
                {
                    being = findNode(msg.readInt32());
                    msg.readInt8();  // equip point
                    Sint16 itemId1 = msg.readInt16();
                    msg.readInt16(); // item id 2

                    if (being != NULL)
                    {
                        being->setWeaponById(itemId1);
                    }
                }
                break;

            case SMSG_PLAYER_UNEQUIP:
                {
                    Sint16 index = msg.readInt16();
                    Sint16 equipPoint = msg.readInt16();
                    Sint8 type = msg.readInt8();

                    if (type == 0) {
                        chatWindow->chatLog("Unable to unequip.", BY_SERVER);
                        break;
                    }

                    if (equipPoint == 0) {
                        // No point given, no point in searching
                        break;
                    }

                    int mask = 1;
                    int position = 0;
                    while (!(equipPoint & mask)) {
                        mask <<= 1;
                        position++;
                    }

                    Item *item = inventory->getItem(index);

                    if (item != NULL)
                    {
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
                                // TODO: Why this break? Shouldn't a weapon be
                                //       unequipped in inventory too?
                                break;
                            default:
                                equipment->removeEquipment(position - 1);
                                break;
                        }
                        logger->log("Unequipping: %i %i(%i) %i",
                                    index, equipPoint, type, position - 1);
                    }
                }
                break;

            case SMSG_PLAYER_ARROW_EQUIP:
                {
                    Sint16 id = msg.readInt16();

                    if (id > 1) {
                        Item *item = inventory->getItem(id);
                        if (item) {
                            item->setEquipped(true);
                            equipment->setArrows(item);
                            logger->log("Arrows equipped: %i", id);
                        }
                    }
                }
                break;

            case SMSG_PLAYER_ARROW_MESSAGE:
                {
                    Sint16 type = msg.readInt16();

                    switch (type) {
                        case 0:
                            chatWindow->chatLog("Equip arrows first",
                                                 BY_SERVER);
                            break;
                        default:
                            logger->log("0x013b: Unhandled message %i", type);
                            break;
                    }
                }
                break;

            case SMSG_PLAYER_STAT_UPDATE_4:
                {
                    Sint16 type = msg.readInt16();
                    Sint8 fail = msg.readInt8();
                    Sint8 value = msg.readInt8();

                    if (fail == 1)
                    {
                        switch (type) {
                            case 0x000d: player_info->STR = value; break;
                            case 0x000e: player_info->AGI = value; break;
                            case 0x000f: player_info->VIT = value; break;
                            case 0x0010: player_info->INT = value; break;
                            case 0x0011: player_info->DEX = value; break;
                            case 0x0012: player_info->LUK = value; break;
                        }
                    }
                }
                break;

                // Updates stats and status points
            case SMSG_PLAYER_STAT_UPDATE_5:
                player_info->statsPointsToAttribute = msg.readInt16();
                player_info->STR   = msg.readInt8();
                player_info->STRUp = msg.readInt8();
                player_info->AGI   = msg.readInt8();
                player_info->AGIUp = msg.readInt8();
                player_info->VIT   = msg.readInt8();
                player_info->VITUp = msg.readInt8();
                player_info->INT   = msg.readInt8();
                player_info->INTUp = msg.readInt8();
                player_info->DEX   = msg.readInt8();
                player_info->DEXUp = msg.readInt8();
                player_info->LUK   = msg.readInt8();
                player_info->LUKUp = msg.readInt8();
                player_info->ATK       = msg.readInt16();  // ATK
                player_info->ATKBonus  = msg.readInt16();  // ATK bonus
                player_info->MATK      = msg.readInt16();  // MATK max
                player_info->MATKBonus = msg.readInt16();  // MATK min
                player_info->DEF       = msg.readInt16();  // DEF
                player_info->DEFBonus  = msg.readInt16();  // DEF bonus
                player_info->MDEF      = msg.readInt16();  // MDEF
                player_info->MDEFBonus = msg.readInt16();  // MDEF bonus
                player_info->HIT       = msg.readInt16();  // HIT
                player_info->FLEE      = msg.readInt16();  // FLEE
                player_info->FLEEBonus = msg.readInt16();  // FLEE bonus
                msg.readInt16();  // critical
                msg.readInt16();  // unknown
                break;

            case SMSG_PLAYER_STAT_UPDATE_6:
                switch (msg.readInt16()) {
                    case 0x0020: player_info->STRUp = msg.readInt8(); break;
                    case 0x0021: player_info->AGIUp = msg.readInt8(); break;
                    case 0x0022: player_info->VITUp = msg.readInt8(); break;
                    case 0x0023: player_info->INTUp = msg.readInt8(); break;
                    case 0x0024: player_info->DEXUp = msg.readInt8(); break;
                    case 0x0025: player_info->LUKUp = msg.readInt8(); break;
                }
                break;

            case SMSG_BEING_NAME_RESPONSE:
                if ((being = findNode(msg.readInt32())))
                {
                    being->setName(msg.readString(24));
                }
                break;

            case 0x0119:
                // Change in players look
                break;

            default:
                // Manage non implemented packets
                logger->log("Unhandled packet: %x", msg.getId());
                break;
        }

        skip(msg.getLength());
    }
}
