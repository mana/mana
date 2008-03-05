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

#include <guichan/exception.hpp>

#include "beingmanager.h"
#include "channelmanager.h"
#include "configuration.h"
#include "engine.h"
#include "flooritemmanager.h"
#include "graphics.h"
#include "itemshortcut.h"
#include "joystick.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "log.h"
#include "npc.h"
#include "particle.h"

#include "gui/buy.h"
#include "gui/buysell.h"
//#include "gui/chargedialog.h"
#include "gui/chat.h"
#include "gui/confirm_dialog.h"
#include "gui/debugwindow.h"
#include "gui/equipmentwindow.h"
#include "gui/gui.h"
#include "gui/guildwindow.h"
#include "gui/help.h"
#include "gui/inventorywindow.h"
#include "gui/itemshortcutwindow.h"
#include "gui/menuwindow.h"
#include "gui/minimap.h"
#include "gui/ministatus.h"
#include "gui/npclistdialog.h"
#include "gui/npc_text.h"
#include "gui/ok_dialog.h"
#include "gui/sdlinput.h"
#include "gui/sell.h"
#include "gui/setup.h"
#include "gui/skill.h"
#include "gui/status.h"
#include "gui/trade.h"
#include "gui/viewport.h"
#include "gui/quitdialog.h"

#include "net/beinghandler.h"
#include "net/buysellhandler.h"
#include "net/chathandler.h"
#include "net/guildhandler.h"
#include "net/inventoryhandler.h"
#include "net/itemhandler.h"
#include "net/network.h"
#include "net/npchandler.h"
#include "net/playerhandler.h"
#include "net/tradehandler.h"

#include "resources/imagewriter.h"

extern Graphics *graphics;

class Map;

std::string map_path;

bool done = false;
volatile int tick_time;
volatile int fps = 0, frame = 0;
Engine *engine = NULL;
Joystick *joystick = NULL;

extern Window *weightNotice;
extern Window *deathNotice;
QuitDialog *quitDialog = NULL;
OkDialog *disconnectedDialog = NULL;

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
//ChargeDialog *chargeDialog;
TradeWindow *tradeWindow;
//BuddyWindow *buddyWindow;
GuildWindow *guildWindow;
HelpWindow *helpWindow;
DebugWindow *debugWindow;
ItemShortcutWindow *itemShortcutWindow;

BeingManager *beingManager = NULL;
FloorItemManager *floorItemManager = NULL;
ChannelManager *channelManager = NULL;

Particle *particleEngine = NULL;

const int MAX_TIME = 10000;

/**
 * Listener used for exitting handling.
 */
namespace {
    struct ExitListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            if (event.getId() == "yes" || event.getId() == "ok") {
                done = true;
            }
        }
    } exitListener;
}

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
 * Updates fps.
 */
Uint32 nextSecond(Uint32 interval, void *param)
{
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
    chatWindow = new ChatWindow;
    menuWindow = new MenuWindow();
    statusWindow = new StatusWindow(player_node);
    miniStatusWindow = new MiniStatusWindow();
    buyDialog = new BuyDialog;
    sellDialog = new SellDialog;
    buySellDialog = new BuySellDialog();
    inventoryWindow = new InventoryWindow();
    npcTextDialog = new NpcTextDialog();
    npcListDialog = new NpcListDialog();
    skillDialog = new SkillDialog();
    //newSkillWindow = new NewSkillDialog();
    setupWindow = new Setup();
    minimap = new Minimap();
    equipmentWindow = new EquipmentWindow(player_node->mEquipment.get());
    //chargeDialog = new ChargeDialog();
    tradeWindow = new TradeWindow;
    //buddyWindow = new BuddyWindow();
    guildWindow = new GuildWindow(player_node);
    helpWindow = new HelpWindow();
    debugWindow = new DebugWindow();
    itemShortcutWindow = new ItemShortcutWindow();

    // Initialize window positions
    //chargeDialog->setPosition(
    //        graphics->getWidth() - 5 - chargeDialog->getWidth(),
    //        graphics->getHeight() - chargeDialog->getHeight() - 15);

    //buddyWindow->setPosition(10, minimap->getHeight() + 30);

    // Set initial window visibility
    chatWindow->setVisible(true);
    miniStatusWindow->setVisible(true);
    menuWindow->setVisible(true);
    itemShortcutWindow->setVisible(true);

    if (config.getValue("logToChat", 0))
    {
        logger->setChatWindow(chatWindow);
    }
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
    //delete chargeDialog;
    //delete newSkillWindow;
    delete tradeWindow;
    //delete buddyWindow;
    delete guildWindow;
    delete helpWindow;
    delete debugWindow;
    delete itemShortcutWindow;
}

Game::Game():
    mBeingHandler(new BeingHandler()),
    mBuySellHandler(new BuySellHandler()),
    mChatHandler(new ChatHandler()),
    mGuildHandler(new GuildHandler()),
    mInventoryHandler(new InventoryHandler()),
    mItemHandler(new ItemHandler()),
    mNpcHandler(new NPCHandler()),
    mPlayerHandler(new PlayerHandler()),
    mTradeHandler(new TradeHandler()),
    mLogicCounterId(0), mSecondsCounterId(0)
{
    done = false;

    createGuiWindows();
    engine = new Engine;

    beingManager = new BeingManager;
    floorItemManager = new FloorItemManager();
    channelManager = new ChannelManager();

    particleEngine = new Particle(NULL);
    particleEngine->setupEngine();

    // Initialize logic and seconds counters
    tick_time = 0;
    mLogicCounterId = SDL_AddTimer(10, nextTick, NULL);
    mSecondsCounterId = SDL_AddTimer(1000, nextSecond, NULL);

    // Initialize frame limiting
    config.addListener("fpslimit", this);
    optionChanged("fpslimit");

    // Initialize beings
    beingManager->setPlayer(player_node);

    Joystick::init();
    // TODO: The user should be able to choose which one to use
    // Open the first device
    if (Joystick::getNumberOfJoysticks() > 0)
    {
        joystick = new Joystick(0);
    }

    Net::registerHandler(mBeingHandler.get());
    Net::registerHandler(mBuySellHandler.get());
    Net::registerHandler(mChatHandler.get());
    Net::registerHandler(mGuildHandler.get());
    Net::registerHandler(mInventoryHandler.get());
    Net::registerHandler(mItemHandler.get());
    Net::registerHandler(mNpcHandler.get());
    Net::registerHandler(mPlayerHandler.get());
    Net::registerHandler(mTradeHandler.get());
}

Game::~Game()
{
    Net::clearHandlers();

    delete player_node;
    destroyGuiWindows();

    delete beingManager;
    delete floorItemManager;
    delete channelManager;
    delete joystick;
    delete particleEngine;
    delete engine;

    viewport->setMap(NULL);
    player_node = NULL;
    beingManager = NULL;
    floorItemManager = NULL;
    joystick = NULL;

    SDL_RemoveTimer(mLogicCounterId);
    SDL_RemoveTimer(mSecondsCounterId);
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
        filename << PHYSFS_getUserDir() << ".tmw/";
#elif defined __APPLE__
        filename << PHYSFS_getUserDir() << "Desktop/";
#endif
        filename << "TMW_Screenshot_" << screenshotCount << ".png";
        testExists.open(filename.str().c_str(), std::ios::in);
        found = !testExists.is_open();
        testExists.close();
    } while (!found);

    if (ImageWriter::writePNG(screenshot, filename.str()))
    {
        std::stringstream chatlogentry;
        chatlogentry << "Screenshot saved to " << filename.str().c_str();
        chatWindow->chatLog(chatlogentry.str(), BY_SERVER);
        return true;
    }
    else
    {
        chatWindow->chatLog("Saving screenshot failed!", BY_SERVER);
        return false;
    }
}

void Game::optionChanged(const std::string &name)
{
    int fpsLimit = (int) config.getValue("fpslimit", 0);

    // Calculate new minimum frame time
    mMinFrameTime = fpsLimit ? 1000 / fpsLimit : 0;

    // Reset draw time to current time
    mDrawTime = tick_time * 10;
}

void Game::logic()
{
    // mDrawTime has a higher granularity than gameTime in order to be able to
    // work with minimum frame durations in milliseconds.
    int gameTime = tick_time;
    mDrawTime = tick_time * 10;

    while (!done)
    {
        // Handle all necessary game logic
        while (get_elapsed_time(gameTime) > 0)
        {
            handleInput();
            engine->logic();
            gameTime++;
        }

        // This is done because at some point tick_time will wrap.
        gameTime = tick_time;

        // Update the screen when application is active, delay otherwise.
        if (SDL_GetAppState() & SDL_APPACTIVE)
        {
            // Draw a frame if either frames are not limited or enough time has
            // passed since the last frame.
            if (!mMinFrameTime ||
                    get_elapsed_time(mDrawTime / 10) > mMinFrameTime)
            {
                frame++;
                gui->draw();
                graphics->updateScreen();
                mDrawTime += mMinFrameTime;

                // Make sure to wrap mDrawTime, since tick_time will wrap.
                if (mDrawTime > MAX_TIME * 10)
                    mDrawTime -= MAX_TIME * 10;
            }
            else
            {
                SDL_Delay(10);
            }
        }
        else
        {
            SDL_Delay(10);
            mDrawTime = tick_time * 10;
        }

        // Handle network stuff
        Net::flush();

        // TODO: Fix notification when the connection is lost
        if (false /*!mNetwork->isConnected() */)
        {
            if (!disconnectedDialog)
            {
                disconnectedDialog = new
                    OkDialog("Network Error",
                    "The connection to the server was lost, the program will now quit");
                disconnectedDialog->addActionListener(&exitListener);
            }

            disconnectedDialog->requestMoveToTop();
        }
    }
}

void Game::handleInput()
{
    if (joystick != NULL)
    {
        joystick->update();
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

            if (setupWindow->isVisible() &&
                keyboard.getNewKeyIndex() > keyboard.KEY_NO_VALUE)
            {
                keyboard.setNewKey((int) event.key.keysym.sym);
                keyboard.callbackNewKey();
                keyboard.setNewKeyIndex(keyboard.KEY_NO_VALUE);
                return;
            }
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

                    // Don not focus chat input when quit dialog is active
		    if(quitDialog != NULL && quitDialog->isVisible())
			break;

                    // Close the Browser if opened
                    if (helpWindow->isVisible())
                    {
                        helpWindow->setVisible(false);
                    }
                    // Close the config window, cancelling changes if opened
                    else if (setupWindow->isVisible())
                    {
                        setupWindow->action(gcn::ActionEvent(NULL, "cancel"));
                    }
                    else if (guildWindow->isVisible())
                    {
                        // TODO: Check if a dialog is open and close it if so
                    }
                    // Else, open the chat edit box
                    else
                    {
                        chatWindow->requestChatFocus();
                        used = true;
                    }
                    break;
                   // Quitting confirmation dialog
                case SDLK_ESCAPE:
                    if (!quitDialog)
                    {
                        quitDialog = new QuitDialog(&done, &quitDialog);
                    }
                    else
                    {
                        quitDialog->requestMoveToTop();
                    }
                    break;

                default:
                    break;
            }

            if (keyboard.isEnabled() && !chatWindow->isFocused() && !guildWindow->isFocused())
            {
                const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);
                // Checks if any item shortcut is pressed.
                for (int i = KeyboardConfig::KEY_SHORTCUT_0;
                    i <= KeyboardConfig::KEY_SHORTCUT_9;
                    i++)
                {
                    if (tKey == i) {
                        itemShortcut->useItem(
                                i - KeyboardConfig::KEY_SHORTCUT_0);
                        break;
                    }
                }
                switch (tKey) {
                    case KeyboardConfig::KEY_PICKUP:
                        {
                            Uint16 x = player_node->mX / 32;
                            Uint16 y = player_node->mY / 32;
                            FloorItem *item =
                                floorItemManager->findByCoordinates(x, y);

                            // If none below the player, try the tile in front
                            // of the player
                            if (!item)
                            {
                                // Temporary until tile-based picking is
                                // removed.
                                switch (player_node->getSpriteDirection())
                                {
                                    case DIRECTION_UP   : --y; break;
                                    case DIRECTION_DOWN : ++y; break;
                                    case DIRECTION_LEFT : --x; break;
                                    case DIRECTION_RIGHT: ++x; break;
                                    default: break;
                                }

                                item =
                                    floorItemManager->findByCoordinates(x, y);
                            }

                            if (item)
                                player_node->pickUp(item);

                            used = true;
                        }
                        break;

                    case KeyboardConfig::KEY_ATTACK:
                        player_node->attack();
                        used = true;
                        break;

                    case KeyboardConfig::KEY_SIT:
                        // Player sit action
                        player_node->toggleSit();
                        used = true;
                        break;

                    case KeyboardConfig::KEY_HIDE_WINDOWS:
                         // Hide certain windows
                        if (!chatWindow->isFocused())
                        {
                            statusWindow->setVisible(false);
                            inventoryWindow->setVisible(false);
                            skillDialog->setVisible(false);
                            setupWindow->setVisible(false);
                            equipmentWindow->setVisible(false);
                            helpWindow->setVisible(false);
                            debugWindow->setVisible(false);
                            guildWindow->setVisible(false);
                        }
                        break;
                }
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
                    case SDLK_p:
                        // Screenshot (picture, hence the p)
                        {
                            SDL_Surface *screenshot = graphics->getScreenshot();
                            if (!saveScreenshot(screenshot))
                            {
                                logger->log(
                                        "Error: could not save Screenshot.");
                            }
                            SDL_FreeSurface(screenshot);
                        }
                        used = true;
                        break;

                    default:
                        break;

                    case SDLK_f:
                        // Find path to mouse (debug purpose)
                        viewport->toggleDebugPath();
                        used = true;
                        break;

                    case SDLK_t:
                        // Toggle accepting of incoming trade requests
                        {
                            TradeHandler *th = static_cast<TradeHandler*>(
                                    mTradeHandler.get());
                            th->setAcceptTradeRequests(
                                    !th->acceptTradeRequests());
                        }
                        used = true;
                        break;
                }

                // Emotions
                Uint8 emotion;
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
                    case SDLK_MINUS: emotion = 11; break;
                    case SDLK_EQUALS: emotion = 12; break;
                    default: emotion = 0; break;
                }

                if (emotion)
                {
                    player_node->emote(emotion);
                    used = true;
                }
            }
        }

        // Quit event
        else if (event.type == SDL_QUIT)
        {
            done = true;
        }

        // Push input to GUI when not used
        if (!used)
        {
            try
            {
                guiInput->pushInput(event);
            }
            catch (gcn::Exception e)
            {
                const char* err = e.getMessage().c_str();
                logger->log("Warning: guichan input exception: %s", err);
            }
        }

    } // End while
    // If the user is configuring the keys then don't respond.
    if (!keyboard.isEnabled())
    {
       return;
    }
    // Moving player around
    if (player_node->mAction != Being::DEAD &&
        !chatWindow->isFocused())
    {
        // Get the state of the keyboard keys
        keyboard.refreshActiveKeys();

        Uint16 x = player_node->mX / 32, y = player_node->mY / 32;

        unsigned char direction = 0;

        // Translate pressed keys to movement and direction
        if (keyboard.isKeyActive(keyboard.KEY_MOVE_UP) ||
            (joystick && joystick->isUp()))
        {
            direction |= Being::UP;
        }
        else if (keyboard.isKeyActive(keyboard.KEY_MOVE_DOWN) ||
                 (joystick && joystick->isDown()))
        {
            direction |= Being::DOWN;
        }

        if (keyboard.isKeyActive(keyboard.KEY_MOVE_LEFT) ||
            (joystick && joystick->isLeft()))
        {
            direction |= Being::LEFT;
        }
        else if (keyboard.isKeyActive(keyboard.KEY_MOVE_RIGHT) ||
                 (joystick && joystick->isRight()))
        {
            direction |= Being::RIGHT;
        }

        player_node->setWalkingDir(direction);

        // Target the nearest monster if 'a' pressed
        if ( keyboard.isKeyActive(keyboard.KEY_TARGET_CLOSEST) )
        //if (keys[SDLK_a])
        {
            Being *target =
                beingManager->findNearestLivingBeing(x, y, 20, Being::MONSTER);

            if (target)
            {
                player_node->setTarget(target);
            }
        }

        if (joystick)
        {
            if (joystick->buttonPressed(1))
            {
                FloorItem *item = floorItemManager->findByCoordinates(x, y);

                if (item)
                    player_node->pickUp(item);
            }
            else if (joystick->buttonPressed(2))
            {
                player_node->toggleSit();
            }
        }
    }
}
