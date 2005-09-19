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
#include "gui/popupmenu.h"
#include "gui/requesttrade.h"
#include "gui/sell.h"
#include "gui/setup.h"
#include "gui/skill.h"
#include "gui/menuwindow.h"
#include "gui/status.h"
#include "gui/ministatus.h"
#include "gui/trade.h"

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
unsigned short startX = 0, startY = 0;
Being *autoTarget = NULL;
Map *tiledMap = NULL;
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
PopupMenu *popupMenu;

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
            outMsg.writeShort(0x00b2);
            outMsg.writeByte(0);
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
    popupMenu = new PopupMenu();

    // Initialize window positions
    int screenW = graphics->getWidth();
    int screenH = graphics->getHeight();

    statusWindow->setPosition((screenW - statusWindow->getWidth()) / 2,
                            (screenH - statusWindow->getHeight()) / 2);
    miniStatusWindow->setPosition(0, 0);
    minimap->setPosition(3, 30);
    chatWindow->setPosition(0, screenH - chatWindow->getHeight());
    menuWindow->setPosition(screenW - menuWindow->getWidth(), 0);
    inventoryWindow->setPosition(screenW - statusWindow->getWidth() -
            inventoryWindow->getWidth() - 10, 5);
    chargeDialog->setPosition(
            screenW - 5 - chargeDialog->getWidth(),
            screenH - chargeDialog->getHeight() - 15);
    tradeWindow->setPosition(screenW - statusWindow->getWidth() -
            tradeWindow->getWidth() - 10,
            inventoryWindow->getY() + inventoryWindow->getHeight());
    /*buddyWindow->setPosition(10,
      minimap->getHeight() + 30);*/
    equipmentWindow->setPosition(5,140);

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
    popupMenu->setVisible(false);

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
    delete popupMenu;
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
    player_node->speed = 150;
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
    
    do_exit();
}

void do_exit()
{
    delete engine;
    delete tiledMap;
    destroyGuiWindows();
    close_session();

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
                    // Accept the Death Notice...
                    else if (deathNotice)
                    {
                        deathNotice->action("ok");
                    }
                    else if (weightNotice)
                    {
                        weightNotice->action("ok");
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
                            // TODO: remove duplicated code, probably add a pick up command
                            MessageOut outMsg;
                            outMsg.writeShort(0x009f);
                            outMsg.writeLong(id);
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
                        statusWindow->setVisible(!statusWindow->isVisible());
                        used = true;
                        break;

                    /*
                        // New skills window
                    case SDLK_n:
                        newSkillWindow->setVisible(!newSkillWindow->isVisible());
                        used = true;
                        break;
                    */
                    // screenshot (picture, hence the p)
                    case SDLK_p:
                        {
                            SDL_Surface *screenshot = graphics->getScreenshot();
                            if (!saveScreenshot(screenshot))
                            {
                                logger->log("Error: could not save Screenshot.");
                            }
                            SDL_FreeSurface(screenshot);
                        }
                    break;

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
                        MessageOut outMsg;
                        outMsg.writeShort(0x00bf);
                        outMsg.writeByte(emotion);
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
                        // Player default: attack
                        case Being::PLAYER:
                            if (target != player_node)
                            {
                                autoTarget = target;
                                attack(target);
                                /*MessageOut outMsg;
                                outMsg.writeShort(CMSG_TRADE_REQUEST);
                                outMsg.writeLong(target->getId());
                                tradePartnerName = target->getName();*/
                            }
                            break;

                            // NPC default: talk
                        case Being::NPC:
                            if (!current_npc)
                            {
                                MessageOut outMsg;
                                outMsg.writeShort(CMSG_NPC_TALK);
                                outMsg.writeLong(target->getId());
                                outMsg.writeByte(0);
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
                        MessageOut outMsg;
                        outMsg.writeShort(0x009f);
                        outMsg.writeLong(floorItemId);
                    }
                }

                // Just cancel the popup menu if shown, and don't make the
                // character walk
                if (popupMenu->isVisible() == true)
                {
                    // If we click elsewhere than in the window, do not use
                    // the event
                    // The user wanted to close the popup.
                    // Still buggy : Wonder if the x, y, width, and height
                    // aren't reported partially with these functions.
                    if (event.button.x >=
                            (popupMenu->getX() + popupMenu->getWidth()) ||
                        event.button.x < popupMenu->getX() ||
                        event.button.y >=
                            (popupMenu->getY() + popupMenu->getHeight()) ||
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
                /*
                 * Some people haven't a mouse with three buttons,
                 * right Usiu??? ;-)
                 */
            }

            // Mouse button right
            else if (event.button.button == SDL_BUTTON_RIGHT)
            {
                Being *being;
                FloorItem *floorItem;

                if ((being = findNode(mx, my)) && being != player_node) {
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

        tiledMap = engine->getCurrentMap();

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
                if (monster == NULL && autoTarget != NULL && monster != player_node)
                {
                    attack(autoTarget);
                }
                else if (keys[SDLK_LSHIFT] && monster != player_node)
                {
                    autoTarget = monster;
                }
            }
        }

        if (joy[JOY_BTN1])
        {
            unsigned short x = player_node->x;
            unsigned short y = player_node->y;
            int id = find_floor_item_by_cor(x, y);

            if (id != 0)
            {
                MessageOut outMsg;
                outMsg.writeShort(CMSG_ITEM_PICKUP);
                outMsg.writeLong(id);
            }
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
    int n_items;
    Map *tiledMap = engine->getCurrentMap();
    Equipment *equipment = Equipment::getInstance();

    // We need at least 2 bytes to identify a packet
    while (in_size >= 2)
    {
        MessageIn msg = get_next_message();

        // Parse packet based on their id
        switch (msg.getId())
        {
            case SMSG_LOGIN_SUCCESS:
                // Connected to game server succesfully, set spawn point
                {
                    msg.readLong(); // server tick
                    msg.readCoordinates(player_node->x,
                            player_node->y,
                            player_node->direction);
                    msg.skip(2);    // unknown
                }
                break;

                // Received speech from being
            case SMSG_BEING_CHAT:
                {
                    int chatMsgLength = msg.readShort() - 8;
                    Being *being = findNode(msg.readLong());

                    if (being != NULL && chatMsgLength > 0)
                    {
                        std::string chatMsg = msg.readString(chatMsgLength);

                        chatWindow->chat_log(chatMsg, BY_OTHER);

                        chatMsg.erase(0, chatMsg.find(" : ", 0) + 3);
                        being->setSpeech(chatMsg, SPEECH_TIME);
                    }
                }
                break;

            case SMSG_PLAYER_CHAT:
            case SMSG_GM_CHAT:
                {
                    int chatMsgLength = msg.readShort() - 4;

                    if (chatMsgLength > 0)
                    {
                        std::string chatMsg = msg.readString(chatMsgLength);

                        if (msg.getId() == SMSG_PLAYER_CHAT)
                        {
                            chatWindow->chat_log(chatMsg, BY_PLAYER);

                            unsigned int pos = chatMsg.find(" : ", 0);
                            if (pos != std::string::npos)
                            {
                                chatMsg.erase(0, pos + 3);
                            }
                            player_node->setSpeech(chatMsg, SPEECH_TIME);
                        }
                        else
                        {
                            chatWindow->chat_log(chatMsg, BY_GM);
                        }
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
                    int id = msg.readLong();
                    unsigned short speed = msg.readShort();
                    msg.readShort();  // unknown
                    msg.readShort();  // unknown
                    msg.readShort();  // option
                    unsigned short job = msg.readShort();  // class

                    Being *being = findNode(id);

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
                        being->frame = 0;
                        being->walk_time = tick_time;
                        being->action = Being::STAND;
                    }

                    // Prevent division by 0 when calculating frame
                    if (speed == 0) { speed = 150; }

                    being->speed = speed;
                    being->job = job;
                    being->setHairStyle(msg.readShort());
                    being->setWeapon(msg.readShort());
                    msg.readShort();  // head option bottom

                    if (msg.getId() == SMSG_BEING_MOVE)
                    {
                        msg.readLong(); // server tick
                    }

                    msg.readShort();  // shield
                    msg.readShort();  // head option top
                    msg.readShort();  // head option mid
                    being->setHairColor(msg.readShort());
                    msg.readShort();  // unknown
                    msg.readShort();  // head dir
                    msg.readShort();  // guild
                    msg.readShort();  // unknown
                    msg.readShort();  // unknown
                    msg.readShort();  // manner
                    msg.readShort();  // karma
                    msg.readByte();   // unknown
                    msg.readByte();   // sex

                    if (msg.getId() == SMSG_BEING_MOVE)
                    {
                        unsigned short srcX, srcY, dstX, dstY;
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

                    msg.readByte();   // unknown
                    msg.readByte();   // unknown
                    msg.readByte();   // unknown / sit
                }
                break;

            case SMSG_BEING_REMOVE:
                // A being should be removed or has died
                {
                    Being *being = findNode(msg.readLong());

                    if (being != NULL)
                    {
                        if (msg.readByte() == 1)
                        {
                            // Death
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
                }
                break;

            case SMSG_PLAYER_UPDATE_1:
            case SMSG_PLAYER_UPDATE_2:
            case SMSG_PLAYER_MOVE:
                // An update about a player, potentially including movement.
                {
                    int id = msg.readLong();
                    unsigned short speed = msg.readShort();
                    msg.readShort();  // option 1
                    msg.readShort();  // option 2
                    msg.readShort();  // option
                    unsigned short job = msg.readShort();

                    Being *being = findNode(id);

                    if (being == NULL)
                    {
                        being = createBeing(id, job, tiledMap);
                    }

                    being->speed = speed;
                    being->job = job;
                    being->setHairStyle(msg.readShort());
                    being->setWeaponById(msg.readShort());  // item id 1
                    msg.readShort();  // item id 2
                    msg.readShort();  // head option bottom

                    if (msg.getId() == SMSG_PLAYER_MOVE)
                    {
                        msg.readLong(); // server tick
                    }

                    msg.readShort();  // head option top
                    msg.readShort();  // head option mid
                    being->setHairColor(msg.readShort());
                    msg.readShort();  // unknown
                    msg.readShort();  // head dir
                    msg.readLong();   // guild
                    msg.readLong();   // emblem
                    msg.readShort();  // manner
                    msg.readByte();   // karma
                    msg.readByte();   // sex

                    if (msg.getId() == SMSG_PLAYER_MOVE)
                    {
                        unsigned short srcX, srcY, dstX, dstY;
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

                    msg.readByte();   // unknown
                    msg.readByte();   // unknown

                    if (msg.getId() == SMSG_PLAYER_UPDATE_1)
                    {
                        if (msg.readByte() == 2)
                        {
                            being->action = Being::SIT;
                        }
                    }
                    else if (msg.getId() == SMSG_PLAYER_MOVE)
                    {
                        msg.readByte(); // unknown
                    }

                    msg.readByte();   // Lv
                    msg.readByte();   // unknown

                    being->walk_time = tick_time;
                    being->frame = 0;
                }
                break;

            case SMSG_NPC_MESSAGE:
                msg.readShort();  // length
                current_npc = msg.readLong();
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

                if (tradeWindow->isVisible() == true || requestTradeDialogOpen)
                {
                    MessageOut outMsg;
                    outMsg.writeShort(CMSG_TRADE_RESPONSE);
                    outMsg.writeByte(4);
                    break;
                }

                requestTradeDialogOpen = true;
                tradePartnerName = msg.readString(24);
                new RequestTradeDialog(tradePartnerName);
                break;

            case SMSG_TRADE_RESPONSE:
                switch (msg.readByte())
                {
                    case 0: // Too far away
                        chatWindow->chat_log("Trading isn't possible. "
                                             "Trade partner is too far away.",
                                             BY_SERVER);
                        break;
                    case 1: // Character doesn't exist
                        chatWindow->chat_log("Trading isn't possible. "
                                             "Character doesn't exist.",
                                             BY_SERVER);
                        break;
                    case 2: // Invite request check failed...
                        chatWindow->chat_log("Trade cancelled due to an "
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
                        chatWindow->chat_log("Trade cancelled.", BY_SERVER);
                        tradeWindow->setVisible(false);
                        break;
                    default: // Shouldn't happen as well, but to be sure
                        chatWindow->chat_log("Unhandled trade cancel packet",
                                             BY_SERVER);
                        break;
                }
                break;

            case SMSG_TRADE_ITEM_ADD:
                {
                    long amount = msg.readLong();
                    short type = msg.readShort();
                    msg.readByte();  // identified flag
                    msg.readByte();  // attribute
                    msg.readByte();  // refine
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
                    Item *item = inventory->getItem(msg.readShort());
                    short quantity = msg.readShort();

                    switch (msg.readByte())
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
                            chatWindow->chat_log("Failed adding item. Trade "
                                                 "partner is over weighted.",
                                                 BY_SERVER);
                            break;
                        default:
                            chatWindow->chat_log("Failed adding item for "
                                                 "unknown reason.", BY_SERVER);
                            break;
                    }
                }
                break;

            case SMSG_TRADE_OK:
                // 0 means ok from myself, 1 means ok from other;
                tradeWindow->receivedOk(msg.readByte() == 0);
                break;

            case SMSG_TRADE_CANCEL:
                chatWindow->chat_log("Trade canceled.", BY_SERVER);
                tradeWindow->setVisible(false);
                tradeWindow->reset();
                break;

            case SMSG_TRADE_COMPLETE:
                chatWindow->chat_log("Trade completed.", BY_SERVER);
                tradeWindow->setVisible(false);
                tradeWindow->reset();
                break;

            case SMSG_PLAYER_INVENTORY:
                {
                    // Only called on map load / warp. First reset all items
                    // to not load them twice on map change.
                    inventory->resetItems();
                    msg.readShort();  // length
                    int number = (msg.getLength() - 4) / 18;

                    for (int loop = 0; loop < number; loop++)
                    {
                        short index = msg.readShort();
                        short itemId = msg.readShort();
                        msg.readByte(); // type
                        msg.readByte(); // identify flag
                        short amount = msg.readShort();
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
                    msg.readShort(); // length
                    int number = (msg.getLength() - 4) / 20;

                    for (int loop = 0; loop < number; loop++)
                    {
                        short index = msg.readShort();
                        short itemId = msg.readShort();
                        msg.readByte();  // type
                        msg.readByte();  // identify flag
                        msg.readShort(); // equip type
                        short equipPoint = msg.readShort();
                        msg.readByte();  // attribute
                        msg.readByte();  // refine
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
                    short index = msg.readShort();
                    short amount = msg.readShort();

                    if (msg.readByte() == 0) {
                        chatWindow->chat_log("Failed to use item", BY_SERVER);
                    } else {
                        inventory->getItem(index)->setQuantity(amount);
                    }
                }
                break;

            case SMSG_PLAYER_WARP:
                {
                    // Set new map path
                    map_path = "maps/" + msg.readString(16);
                    map_path= map_path.substr(0, map_path.rfind(".")) + ".tmx.gz";

                    int x = msg.readShort();
                    int y = msg.readShort();

                    logger->log("Warping to %s (%d, %d)", map_path.c_str(), x, y);

                    engine->changeMap(map_path);
                    tiledMap = engine->getCurrentMap();

                    empty_floor_items();

                    // Remove the player, so it is not deleted
                    beings.remove(player_node);

                    // Delete all beings except the local player
                    std::list<Being *>::iterator i;
                    for (i = beings.begin(); i != beings.end(); i++)
                    {
                        delete (*i);
                    }
                    beings.clear();

                    autoTarget = NULL;
                    current_npc = 0;

                    // Re-add the local player node
                    beings.push_back(player_node);

                    player_node->action = Being::STAND;
                    player_node->frame = 0;
                    player_node->x = x;
                    player_node->y = y;
                    player_node->setMap(tiledMap);

                    // Send "map loaded"
                    MessageOut outMsg;
                    outMsg.writeShort(CMSG_MAP_LOADED);
                }
                break;

            case SMSG_SKILL_FAILED:
                // Action failed (ex. sit because you have not reached the
                // right level)
                CHATSKILL action;
                action.skill   = msg.readShort();
                action.bskill  = msg.readShort();
                action.unused  = msg.readShort(); // unknown
                action.success = msg.readByte();
                action.reason  = msg.readByte();
                if (action.success != SKILL_FAILED &&
                    action.bskill == BSKILL_EMOTE)
                {
                    printf("Action: %d/%d", action.bskill, action.success);
                }
                chatWindow->chat_log(action);
                break;

            case SMSG_PLAYER_STAT_UPDATE_1:
                switch (msg.readShort())
                {
                    //case 0x0000:
                    //    player_node->speed = msg.readLong();
                    //    break;
                    case 0x0005:
                        player_info->hp = msg.readLong();
                        break;
                    case 0x0006:
                        player_info->maxHp = msg.readLong();
                        break;
                    case 0x0007:
                        player_info->mp = msg.readLong();
                        break;
                    case 0x0008:
                        player_info->maxMp = msg.readLong();
                        break;
                    case 0x000b:
                        player_info->lvl = msg.readLong();
                        break;
                    case 0x000c:
                        player_info->skillPoint = msg.readLong();
                        skillDialog->setPoints(player_info->skillPoint);
                        break;
                    case 0x0018:
                        player_info->totalWeight = msg.readLong();
                        if (player_info->totalWeight >= player_info->maxWeight)
                        {
                            weightNotice = new OkDialog("Message",
                            "You are carrying more then half your weight. You are unable to regain health.",
                            &weightNoticeListener);
                            weightNotice->releaseModalFocus();
                        }
                        break;
                    case 0x0019:
                        player_info->maxWeight = msg.readLong();
                        break;
                    case 0x0037:
                        player_info->jobLvl = msg.readLong();
                        break;
                    case 0x0009:
                        player_info->statsPointsToAttribute = msg.readLong();
                        break;
                    case 0x0029:
                        player_info->ATK = msg.readLong();
                        break;
                    case 0x002b:
                        player_info->MATK = msg.readLong();
                        break;
                    case 0x002d:
                        player_info->DEF = msg.readLong();
                        break;
                    case 0x002f:
                        player_info->MDEF = msg.readLong();
                        break;
                    case 0x0031:
                        player_info->HIT = msg.readLong();
                        break;
                    case 0x0032:
                        player_info->FLEE = msg.readLong();
                        break;
                    case 0x0035:
                        player_node->aspd = msg.readLong();
                        break;
                }

                if (player_info->hp == 0 && deathNotice == NULL)
                {
                    deathNotice = new OkDialog("Message",
                            "You're now dead, press ok to restart",
                            &deathNoticeListener);
                    deathNotice->releaseModalFocus();
                    player_node->action = Being::DEAD;
                }
                break;

                // Stop walking
                // case 0x0088:  // Disabled because giving some problems
                //if (being = findNode(readLong(2))) {
                //    if (being->getId() != player_node->getId()) {
                //        being->action = STAND;
                //        being->frame = 0;
                //        set_coordinates(being->coordinates,
                //                        readWord(6), readWord(8),
                //                        get_direction(being->coordinates));
                //    }
                //}
                //break;

            case SMSG_BEING_ACTION:
                {
                    Being *srcBeing = findNode(msg.readLong());
                    Being *dstBeing = findNode(msg.readLong());
//                    msg.readLong();   // server tick
//                    msg.readLong();   // src speed
//                    msg.readLong();   // dst speed
                    msg.skip(12);
                    short param1 = msg.readShort();
                    msg.skip(2);  // param 2
                    char type = msg.readByte();
                    msg.skip(2);  // param 3

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
                                srcBeing->frame = 0;
                                srcBeing->walk_time = tick_time;
                            }
                            break;

                        case 2: // Sit
                            if (srcBeing == NULL) break;
                            srcBeing->frame = 0;
                            srcBeing->action = Being::SIT;
                            break;

                        case 3: // Stand up
                            if (srcBeing == NULL) break;
                            srcBeing->frame = 0;
                            srcBeing->action = Being::STAND;
                            break;
                    }
                }
                break;

            case SMSG_PLAYER_STAT_UPDATE_2:
                switch (msg.readShort()) {
                    case 0x0001:
                        player_info->xp = msg.readLong();
                        break;
                    case 0x0002:
                        player_info->jobXp = msg.readLong();
                        break;
                    case 0x0014:
                        player_info->gp = msg.readLong();
                        break;
                    case 0x0016:
                        player_info->xpForNextLevel = msg.readLong();
                        break;
                    case 0x0017:
                        player_info->jobXpForNextLevel = msg.readLong();
                        break;
                }
                break;

            case SMSG_BEING_LEVELUP:
                if ((unsigned long)msg.readLong() == player_node->getId()) {
                    logger->log("Level up");
                    sound.playSfx("sfx/levelup.ogg");
                } else {
                    logger->log("Someone else went level up");
                }
                msg.readLong();  // type
                break;

            case SMSG_BEING_EMOTION:
                {
                    Being *being = findNode(msg.readLong());
                    if (being == NULL) break;

                    being->emotion = msg.readByte();
                    being->emotion_time = EMOTION_TIME;
                }
                break;

            case SMSG_PLAYER_STAT_UPDATE_3:
                {
                    long type = msg.readLong();
                    long base = msg.readLong();
                    long bonus = msg.readLong();
                    long total = base + bonus;

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
                current_npc = msg.readLong();
                break;

            case SMSG_NPC_BUY:
                msg.readShort();  // length
                n_items = (msg.getLength() - 4) / 11;
                buyDialog->reset();
                buyDialog->setMoney(player_info->gp);
                buyDialog->setVisible(true);

                for (int k = 0; k < n_items; k++)
                {
                    long value = msg.readLong();
                    msg.readLong();  // DCvalue
                    msg.readByte();  // type
                    short itemId = msg.readShort();
                    buyDialog->addItem(itemId, value);
                }
                break;

            case SMSG_NPC_SELL:
                msg.readShort();  // length
                n_items = (msg.getLength() - 4) / 10;
                if (n_items > 0) {
                    sellDialog->reset();
                    sellDialog->setVisible(true);

                    for (int k = 0; k < n_items; k++)
                    {
                        short index = msg.readShort();
                        long value = msg.readLong();
                        msg.readLong();  // OCvalue

                        Item *item = inventory->getItem(index);
                        if (item && !(item->isEquipped())) {
                            sellDialog->addItem(item, value);
                        }
                    }
                }
                else {
                    chatWindow->chat_log("Nothing to sell", BY_SERVER);
                    current_npc = 0;
                }
                break;

            case SMSG_NPC_BUY_RESPONSE:
                if (msg.readByte() == 0) {
                    chatWindow->chat_log("Thanks for buying", BY_SERVER);
                } else {
                    chatWindow->chat_log("Unable to buy", BY_SERVER);
                }
                break;

            case SMSG_NPC_SELL_RESPONSE:
                if (msg.readByte() == 0) {
                    chatWindow->chat_log("Thanks for selling", BY_SERVER);
                } else {
                    chatWindow->chat_log("Unable to sell", BY_SERVER);
                }
                break;

            case SMSG_PLAYER_INVENTORY_ADD:
                {
                    short index = msg.readShort();
                    short amount = msg.readShort();
                    short itemId = msg.readShort();
                    msg.readByte();  // identify flag
                    msg.readByte();  // attribute
                    msg.readByte();  // refine
                    msg.skip(8);     // card
                    short equipType = msg.readShort();
                    msg.readByte();  // type
                    char fail = msg.readByte();

                    if (fail > 0) {
                        chatWindow->chat_log("Unable to pick up item",
                                             BY_SERVER);
                    } else {
                        inventory->addItem(index, itemId, amount,
                                           equipType != 0);
                    }
                }
                break;

            case SMSG_PLAYER_INVENTORY_REMOVE:
                {
                    short index = msg.readShort();
                    short amount = msg.readShort();
                    inventory->getItem(index)->increaseQuantity(-amount);
                }
                break;

            case SMSG_PLAYER_INVENTORY_USE:
                {
                    short index = msg.readShort();
                    msg.readShort(); // item id
                    msg.readLong();  // id
                    short amountLeft = msg.readShort();
                    msg.readByte();  // type

                    inventory->getItem(index)->setQuantity(amountLeft);
                }
                break;

            case SMSG_PLAYER_SKILLS:
                msg.readShort();  // length
                n_items = (msg.getLength() - 4) / 37;
                skillDialog->cleanList();

                for (int k = 0; k < n_items; k++)
                {
                    short skillId = msg.readShort();
                    msg.readShort();  // target type
                    msg.readShort();  // unknown
                    short level = msg.readShort();
                    short sp = msg.readShort();
                    msg.readShort();  // range
                    std::string skillName = msg.readString(24);
                    char up = msg.readByte();

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
                msg.readLong(); // id
                chatWindow->chat_log("MVP player", BY_SERVER);
                break;

            case SMSG_ITEM_VISIBLE:
            case SMSG_ITEM_DROPPED:
                {
                    long id = msg.readLong();
                    short itemId = msg.readShort();
                    msg.readByte();  // identify flag
                    short x = msg.readShort();
                    short y = msg.readShort();
                    msg.skip(4);     // amount,subX,subY / subX,subY,amount

                    add_floor_item(new FloorItem(id, itemId, x, y));
                }
                break;

            case SMSG_ITEM_REMOVE:
                remove_floor_item(msg.readLong());
                break;

            case SMSG_NPC_CHOICE:
                msg.readShort();  // length
                current_npc = msg.readLong();
                npcListDialog->parseItems(msg.readString(msg.getLength() - 8));
                npcListDialog->setVisible(true);
                break;

            case SMSG_BEING_CHANGE_LOOKS:
                {
                    Being *being = findNode(msg.readLong());

                    if (being)
                    {
                        switch (msg.readByte()) {
                            case 1:
                                being->setHairStyle(msg.readByte());
                                break;
                            case 2:
                                being->setWeapon(msg.readByte());
                                break;
                            case 6:
                                being->setHairColor(msg.readByte());
                                break;
                            default:
                                msg.readByte(); // unsupported
                                break;
                        }
                    }
                }
                break;

            case SMSG_PLAYER_EQUIP:
                {
                    short index = msg.readShort();
                    short equipPoint = msg.readShort();
                    char type = msg.readByte();

                    logger->log("Equipping: %i %i %i",
                                index, equipPoint, type);

                    if (type == 0) {
                        chatWindow->chat_log("Unable to equip.", BY_SERVER);
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
                    Being *being = findNode(msg.readLong());
                    msg.readByte();  // equip point
                    short itemId1 = msg.readShort();
                    msg.readShort(); // item id 2

                    if (being != NULL)
                    {
                        being->setWeaponById(itemId1);
                    }
                }
                break;

            case SMSG_PLAYER_UNEQUIP:
                {
                    short index = msg.readShort();
                    short equipPoint = msg.readShort();
                    char type = msg.readByte();

                    if (type == 0) {
                        chatWindow->chat_log("Unable to unequip.", BY_SERVER);
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
                    short id = msg.readShort();

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
                    short type = msg.readShort();

                    switch (type) {
                        case 0:
                            chatWindow->chat_log("Equip arrows first",
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
                    short type = msg.readShort();
                    char fail = msg.readByte();
                    char value = msg.readByte();

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
                player_info->statsPointsToAttribute = msg.readShort();
                player_info->STR   = msg.readByte();
                player_info->STRUp = msg.readByte();
                player_info->AGI   = msg.readByte();
                player_info->AGIUp = msg.readByte();
                player_info->VIT   = msg.readByte();
                player_info->VITUp = msg.readByte();
                player_info->INT   = msg.readByte();
                player_info->INTUp = msg.readByte();
                player_info->DEX   = msg.readByte();
                player_info->DEXUp = msg.readByte();
                player_info->LUK   = msg.readByte();
                player_info->LUKUp = msg.readByte();
                player_info->ATK       = msg.readShort();  // ATK
                player_info->ATKBonus  = msg.readShort();  // ATK bonus
                player_info->MATK      = msg.readShort();  // MATK max
                player_info->MATKBonus = msg.readShort();  // MATK min
                player_info->DEF       = msg.readShort();  // DEF
                player_info->DEFBonus  = msg.readShort();  // DEF bonus
                player_info->MDEF      = msg.readShort();  // MDEF
                player_info->MDEFBonus = msg.readShort();  // MDEF bonus
                player_info->HIT       = msg.readShort();  // HIT
                player_info->FLEE      = msg.readShort();  // FLEE
                player_info->FLEEBonus = msg.readShort();  // FLEE bonus
                msg.readShort();  // critical
                msg.readShort();  // unknown
                break;

            case SMSG_PLAYER_STAT_UPDATE_6:
                switch (msg.readShort()) {
                    case 0x0020: player_info->STRUp = msg.readByte(); break;
                    case 0x0021: player_info->AGIUp = msg.readByte(); break;
                    case 0x0022: player_info->VITUp = msg.readByte(); break;
                    case 0x0023: player_info->INTUp = msg.readByte(); break;
                    case 0x0024: player_info->DEXUp = msg.readByte(); break;
                    case 0x0025: player_info->LUKUp = msg.readByte(); break;
                }
                break;

            case SMSG_BEING_NAME_RESPONSE:
                {
                    Being *being = findNode(msg.readLong());

                    if (being) {
                        being->setName(msg.readString(24));
                    }
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
