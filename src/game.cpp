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

#include "beingmanager.h"
#include "configuration.h"
#include "engine.h"
#include "floor_item.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"
#include "npc.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/chargedialog.h"
#include "gui/chat.h"
#include "gui/confirm_dialog.h"
#include "gui/equipmentwindow.h"
#include "gui/help.h"
#include "gui/inventorywindow.h"
#include "gui/minimap.h"
#include "gui/npclistdialog.h"
#include "gui/npc_text.h"
#include "gui/sell.h"
#include "gui/setup.h"
#include "gui/skill.h"
#include "gui/menuwindow.h"
#include "gui/status.h"
#include "gui/ministatus.h"
#include "gui/trade.h"
#include "gui/debugwindow.h"

#include "net/beinghandler.h"
#include "net/buysellhandler.h"
#include "net/chathandler.h"
#include "net/equipmenthandler.h"
#include "net/inventoryhandler.h"
#include "net/itemhandler.h"
#include "net/network.h"
#include "net/npchandler.h"
#include "net/playerhandler.h"
#include "net/skillhandler.h"
#include "net/tradehandler.h"

#include "resources/imagewriter.h"

enum {
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT,
    JOY_BTN0,
    JOY_BTN1,
    JOY_BTN2,
    JOY_BTN3,
    JOY_BTN4,
    JOY_BTN5,
    JOY_BTN6,
    JOY_BTN7,
    JOY_BTN8,
    JOY_BTN9,
    JOY_BTN10,
    JOY_BTN11
};

extern Graphics *graphics;
extern gcn::SDLInput *guiInput;

class Map;

std::string map_path;

bool done = false;
volatile int tick_time;
volatile bool action_time = false;
volatile int fps = 0, frame = 0;
Engine *engine = NULL;
SDL_Joystick *joypad = NULL;       /**< Joypad object */

extern Window *weightNotice;
extern Window *deathNotice;
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

BeingManager *beingManager = NULL;

const int MAX_TIME = 10000;

/**
 * Listener used for exitting handling.
 */
class ExitListener : public gcn::ActionListener {
    void action(const std::string &eventId) {
        if (eventId == "yes") {
            done = true;
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
void createGuiWindows(Network *network)
{
    // Create dialogs
    chatWindow = new ChatWindow(
    config.getValue("homeDir", "") + std::string("/chatlog.txt"), network);
    menuWindow = new MenuWindow();
    statusWindow = new StatusWindow(player_node);
    miniStatusWindow = new MiniStatusWindow();
    buyDialog = new BuyDialog(network);
    sellDialog = new SellDialog(network);
    buySellDialog = new BuySellDialog();
    inventoryWindow = new InventoryWindow();
    npcTextDialog = new NpcTextDialog();
    npcListDialog = new NpcListDialog();
    skillDialog = new SkillDialog();
    //newSkillWindow = new NewSkillDialog();
    setupWindow = new Setup();
    minimap = new Minimap();
    equipmentWindow = new EquipmentWindow(player_node->mEquipment);
    chargeDialog = new ChargeDialog();
    tradeWindow = new TradeWindow(network);
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

Game::Game(Network *network):
    mNetwork(network)
{
    createGuiWindows(network);
    engine = new Engine(network);

    beingManager = new BeingManager(network);

    // Initialize timers
    tick_time = 0;
    SDL_AddTimer(10, nextTick, NULL);                     // Logic counter
    SDL_AddTimer(1000, nextSecond, NULL);                 // Seconds counter

    // Initialize beings
    beingManager->setPlayer(player_node);
    player_node->setNetwork(network);
    engine->changeMap(map_path);

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

    mBeingHandler = new BeingHandler();
    mBuySellHandler = new BuySellHandler();
    mChatHandler = new ChatHandler();
    mEquipmentHandler = new EquipmentHandler();
    mInventoryHandler = new InventoryHandler();
    mItemHandler = new ItemHandler();
    mNpcHandler = new NPCHandler();
    mPlayerHandler = new PlayerHandler();
    mSkillHandler = new SkillHandler();
    mTradeHandler = new TradeHandler();

    network->registerHandler(mBeingHandler);
    network->registerHandler(mBuySellHandler);
    network->registerHandler(mChatHandler);
    network->registerHandler(mEquipmentHandler);
    network->registerHandler(mInventoryHandler);
    network->registerHandler(mItemHandler);
    network->registerHandler(mNpcHandler);
    network->registerHandler(mPlayerHandler);
    network->registerHandler(mSkillHandler);
    network->registerHandler(mTradeHandler);
}

Game::~Game()
{
    delete mBeingHandler;
    delete mBuySellHandler;
    delete mChatHandler;
    delete mEquipmentHandler;
    delete mInventoryHandler;
    delete mItemHandler;
    delete mNpcHandler;
    delete mPlayerHandler;
    delete mSkillHandler;
    delete mTradeHandler;

    delete engine;
    delete player_node;
    destroyGuiWindows();

    if (joypad != NULL)
    {
        SDL_JoystickClose(joypad);
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
#if (defined __USE_UNIX98 || defined __FreeBSD__)
        filename << PHYSFS_getUserDir() << "/";
#endif
        filename << "TMW_Screenshot_" << screenshotCount << ".png";
        testExists.open(filename.str().c_str(), std::ios::in);
        found = !testExists.is_open();
        testExists.close();
    } while (!found);

    return ImageWriter::writePNG(screenshot, filename.str());
}

void Game::logic()
{
    int gameTime = tick_time;

    while (!done)
    {
        // Handle all necessary game logic
        while (get_elapsed_time(gameTime) > 0)
        {
            handleInput();
            engine->logic();
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

        // Handle network stuff
        mNetwork->flush();
        mNetwork->dispatchMessages();
    }
}

void Game::handleInput()
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
        int lowerTolerance = (int)config.getValue("leftTolerance", -100);
        int upperTolerance = (int)config.getValue("rightTolerance", 100);
        SDL_JoystickUpdate();
        int position = SDL_JoystickGetAxis(joypad, 0);
        if (position >= upperTolerance)
        {
            joy[JOY_RIGHT] = true;
        }
        else if (position <= lowerTolerance)
        {
            joy[JOY_LEFT] = true;
        }
        lowerTolerance = (int)config.getValue("upTolerance", -100);
        upperTolerance = (int)config.getValue("downTolerance", 100);
        position = SDL_JoystickGetAxis(joypad, 1);
        if (position <= lowerTolerance)
        {
            joy[JOY_UP] = true;
        }
        else if (position >= upperTolerance)
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
                        done = true;
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
                        FloorItem *item = find_floor_item_by_cor(
                                player_node->x, player_node->y);

                        // If none below the player, try the tile in front of
                        // the player
                        if (!item) {
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
                            item = find_floor_item_by_cor(x, y);
                        }

                        if (item)
                            player_node->pickUp(item);

                        used = true;
                    }
                    break;

                    // Quitting confirmation dialog
                case SDLK_ESCAPE:
                    if (!exitConfirm) {
                        exitConfirm = new ConfirmDialog(
                                "Quit", "Are you sure you want to quit?");
                        exitConfirm->addActionListener(&exitListener);
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

                        player_node->toggleSit();
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
                        engine->toggleDebugPath();
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
                        player_node->emote(emotion);
                        action_time = false;
                        used = true;
                    }
                }
            }
        }

        // Quit event
        else if (event.type == SDL_QUIT)
        {
            done = true;
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
        Being::Direction Direction = Being::DIR_NONE;

        // Translate pressed keys to movement and direction
        if (keys[SDLK_UP] || keys[SDLK_KP8] || joy[JOY_UP])
        {
            Direction = Being::NORTH;
        }
        if (keys[SDLK_DOWN] || keys[SDLK_KP2] || joy[JOY_DOWN])
        {
            Direction = Being::SOUTH;
        }
        if (keys[SDLK_LEFT] || keys[SDLK_KP4] || joy[JOY_LEFT])
        {
            // Allow diagonal walking
            // TODO: Make this nicer, once we got a bitfield for directions
            if (Direction == Being::NORTH)
                Direction = Being::NW;
            else if (Direction == Being::SOUTH)
                Direction = Being::SW;
            else
                Direction = Being::WEST;
        }
        if (keys[SDLK_RIGHT] || keys[SDLK_KP6] || joy[JOY_RIGHT])
        {
            // Allow diagonal walking
            // TODO: Make this nicer, once we got a bitfield for directions
            if (Direction == Being::NORTH)
                Direction = Being::NE;
            else if (Direction == Being::SOUTH)
                Direction = Being::SE;
            else
                Direction = Being::EAST;
        }
        if (keys[SDLK_KP1]) // Bottom Left
        {
            Direction = Being::SW;
        }
        if (keys[SDLK_KP3]) // Bottom Right
        {
            Direction = Being::SE;
        }
        if (keys[SDLK_KP7]) // Top Left
        {
            Direction = Being::NW;
        }
        if (keys[SDLK_KP9]) // Top Right
        {
            Direction = Being::NE;
        }

        player_node->walk(Direction);

        // Attacking monsters
        if (keys[SDLK_LCTRL] || keys[SDLK_RCTRL] || joy[JOY_BTN0])
        {
            Being *target = NULL;
            bool newTarget = keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT];

            // A set target has highest priority
            if (newTarget || !player_node->getTarget())
            {
                Uint16 targetX = x, targetY = y;

                switch (player_node->direction)
                {
                    case Being::SOUTH:
                        targetY++;
                        break;

                    case Being::WEST:
                        targetX--;
                        break;

                    case Being::NORTH:
                        targetY--;
                        break;

                    case Being::EAST:
                        targetX++;
                        break;
                }

                // Attack priorioty is: Monster, Player, auto target
                target = beingManager->findBeing(
                        targetX, targetY, Being::MONSTER);
                if (!target)
                    target = beingManager->findBeing(
                            targetX, targetY, Being::PLAYER);
            }

            player_node->attack(target, newTarget);
        }

        if (joy[JOY_BTN1])
        {
            FloorItem *item = find_floor_item_by_cor(
                    player_node->x, player_node->y);

            if (item)
                player_node->pickUp(item);
        }
        else if (joy[JOY_BTN2] && action_time)
        {
            player_node->toggleSit();
            action_time = false;
        }
    }
}
