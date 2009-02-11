/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <fstream>
#include <physfs.h>
#include <sstream>
#include <string>

#include <guichan/exception.hpp>

#include "beingmanager.h"
#include "configuration.h"
#include "effectmanager.h"
#include "emoteshortcut.h"
#include "engine.h"
#include "flooritemmanager.h"
#include "game.h"
#include "graphics.h"
#include "itemshortcut.h"
#include "joystick.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "log.h"
#include "npc.h"
#include "particle.h"
#include "player_relations.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/chat.h"
#include "gui/confirm_dialog.h"
#include "gui/debugwindow.h"
#include "gui/emoteshortcutcontainer.h"
#include "gui/emotewindow.h"
#include "gui/equipmentwindow.h"
#include "gui/gui.h"
#include "gui/help.h"
#include "gui/inventorywindow.h"
#include "gui/shortcutwindow.h"
#include "gui/itemshortcutcontainer.h"
#include "gui/menuwindow.h"
#include "gui/minimap.h"
#include "gui/ministatus.h"
#include "gui/npcintegerdialog.h"
#include "gui/npclistdialog.h"
#include "gui/npcstringdialog.h"
#include "gui/npc_text.h"
#include "gui/ok_dialog.h"
#include "gui/sdlinput.h"
#include "gui/sell.h"
#include "gui/setup.h"
#include "gui/skill.h"
#include "gui/status.h"
#include "gui/trade.h"
#include "gui/viewport.h"

#include "net/beinghandler.h"
#include "net/buysellhandler.h"
#include "net/chathandler.h"
#include "net/equipmenthandler.h"
#include "net/inventoryhandler.h"
#include "net/itemhandler.h"
#include "net/messageout.h"
#include "net/network.h"
#include "net/npchandler.h"
#include "net/playerhandler.h"
#include "net/protocol.h"
#include "net/skillhandler.h"
#include "net/tradehandler.h"

#include "resources/imagewriter.h"

#include "utils/gettext.h"

class Map;

std::string map_path;

bool done = false;
volatile int tick_time;
volatile int fps = 0, frame = 0;

Engine *engine = NULL;
Joystick *joystick = NULL;

extern Window *weightNotice;
extern Window *deathNotice;
ConfirmDialog *exitConfirm = NULL;
OkDialog *disconnectedDialog = NULL;

ChatWindow *chatWindow;
MenuWindow *menuWindow;
StatusWindow *statusWindow;
MiniStatusWindow *miniStatusWindow;
BuyDialog *buyDialog;
SellDialog *sellDialog;
BuySellDialog *buySellDialog;
InventoryWindow *inventoryWindow;
EmoteWindow *emoteWindow;
NpcIntegerDialog *npcIntegerDialog;
NpcListDialog *npcListDialog;
NpcTextDialog *npcTextDialog;
NpcStringDialog *npcStringDialog;
SkillDialog *skillDialog;
Setup* setupWindow;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
TradeWindow *tradeWindow;
HelpWindow *helpWindow;
DebugWindow *debugWindow;
ShortcutWindow *itemShortcutWindow;
ShortcutWindow *emoteShortcutWindow;

BeingManager *beingManager = NULL;
FloorItemManager *floorItemManager = NULL;
Particle* particleEngine = NULL;
EffectManager *effectManager = NULL;

const int MAX_TIME = 10000;

/**
 * Listener used for exiting handling.
 */
namespace {
    struct ExitListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            if (event.getId() == "yes" || event.getId() == "ok") {
                done = true;
            }
            exitConfirm = NULL;
            disconnectedDialog = NULL;
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
    if (start_time <= tick_time)
    {
        return (tick_time - start_time) * 10;
    }
    else
    {
        return (tick_time + (MAX_TIME - start_time)) * 10;
    }
}

/**
 * Create all the various globally accessible gui windows
 */
void createGuiWindows(Network *network)
{
    // Create dialogs
    chatWindow = new ChatWindow(network);
    menuWindow = new MenuWindow();
    statusWindow = new StatusWindow(player_node);
    miniStatusWindow = new MiniStatusWindow();
    buyDialog = new BuyDialog(network);
    sellDialog = new SellDialog(network);
    buySellDialog = new BuySellDialog();
    inventoryWindow = new InventoryWindow();
    emoteWindow = new EmoteWindow();
    npcTextDialog = new NpcTextDialog();
    npcIntegerDialog = new NpcIntegerDialog();
    npcListDialog = new NpcListDialog();
    npcStringDialog = new NpcStringDialog();
    skillDialog = new SkillDialog();
    setupWindow = new Setup();
    minimap = new Minimap();
    equipmentWindow = new EquipmentWindow();
    tradeWindow = new TradeWindow(network);
    helpWindow = new HelpWindow();
    debugWindow = new DebugWindow();
    itemShortcutWindow = new ShortcutWindow("ItemShortcut",new ItemShortcutContainer);
    emoteShortcutWindow = new ShortcutWindow("emoteShortcut",new EmoteShortcutContainer);

    // Set initial window visibility
    chatWindow->setVisible((bool) config.getValue(
        chatWindow->getWindowName() + "Visible", true));
    miniStatusWindow->setVisible((bool) config.getValue(
        miniStatusWindow->getWindowName() + "Visible", true));
    buyDialog->setVisible(false);
    sellDialog->setVisible(false);
    minimap->setVisible((bool) config.getValue(
        minimap->getWindowName() + "Visible", true));
    tradeWindow->setVisible(false);
    menuWindow->setVisible((bool) config.getValue(
        menuWindow->getWindowName() + "Visible", true));
    itemShortcutWindow->setVisible((bool) config.getValue(
        itemShortcutWindow->getWindowName() + "Visible", true));
    emoteShortcutWindow->setVisible((bool) config.getValue(
        emoteShortcutWindow->getWindowName() + "Visible", true));
    minimap->setVisible((bool) config.getValue(
        minimap->getWindowName() + "Visible", true));
}

/**
 * Destroy all the globally accessible gui windows
 */
void destroyGuiWindows()
{
    logger->setChatWindow(NULL);
    delete chatWindow;
    delete statusWindow;
    delete miniStatusWindow;
    delete menuWindow;
    delete buyDialog;
    delete sellDialog;
    delete buySellDialog;
    delete inventoryWindow;
    delete emoteWindow;
    delete npcIntegerDialog;
    delete npcListDialog;
    delete npcTextDialog;
    delete npcStringDialog;
    delete skillDialog;
    delete setupWindow;
    delete minimap;
    delete equipmentWindow;
    delete tradeWindow;
    delete helpWindow;
    delete debugWindow;
    delete itemShortcutWindow;
    delete emoteShortcutWindow;
}

Game::Game(Network *network):
    mNetwork(network),
    mBeingHandler(new BeingHandler(config.getValue("EnableSync", 0) == 1)),
    mBuySellHandler(new BuySellHandler()),
    mChatHandler(new ChatHandler()),
    mEquipmentHandler(new EquipmentHandler()),
    mInventoryHandler(new InventoryHandler()),
    mItemHandler(new ItemHandler()),
    mNpcHandler(new NPCHandler()),
    mPlayerHandler(new PlayerHandler()),
    mSkillHandler(new SkillHandler()),
    mTradeHandler(new TradeHandler())
{
    createGuiWindows(network);
    engine = new Engine(network);

    beingManager = new BeingManager(network);
    floorItemManager = new FloorItemManager();
    effectManager = new EffectManager();

    particleEngine = new Particle(NULL);
    particleEngine->setupEngine();

    // Initialize timers
    tick_time = 0;
    SDL_AddTimer(10, nextTick, NULL);                     // Logic counter
    SDL_AddTimer(1000, nextSecond, NULL);                 // Seconds counter

    // Initialize frame limiting
    config.addListener("fpslimit", this);
    optionChanged("fpslimit");

    // Initialize beings
    beingManager->setPlayer(player_node);
    player_node->setNetwork(network);

    Joystick::init();
    // TODO: The user should be able to choose which one to use
    // Open the first device
    if (Joystick::getNumberOfJoysticks() > 0)
        joystick = new Joystick(0);

    network->registerHandler(mBeingHandler.get());
    network->registerHandler(mBuySellHandler.get());
    network->registerHandler(mChatHandler.get());
    network->registerHandler(mEquipmentHandler.get());
    network->registerHandler(mInventoryHandler.get());
    network->registerHandler(mItemHandler.get());
    network->registerHandler(mNpcHandler.get());
    network->registerHandler(mPlayerHandler.get());
    network->registerHandler(mSkillHandler.get());
    network->registerHandler(mTradeHandler.get());

    /*
     * To prevent the server from sending data before the client
     * has initialized, I've modified it to wait for a "ping"
     * from the client to complete its initialization
     *
     * Note: This only affects the latest eAthena version.  This
     * packet is handled by the older version, but its response
     * is ignored by the client
     */
    MessageOut msg(mNetwork);
    msg.writeInt16(CMSG_CLIENT_PING);
    msg.writeInt32(tick_time);

    engine->changeMap(map_path);
}

Game::~Game()
{
    delete player_node;
    destroyGuiWindows();

    delete beingManager;
    delete floorItemManager;
    delete joystick;
    delete particleEngine;
    delete engine;

    beingManager = NULL;
    floorItemManager = NULL;
    joystick = NULL;
}

static bool saveScreenshot()
{
    static unsigned int screenshotCount = 0;

    SDL_Surface *screenshot = graphics->getScreenshot();

    // Search for an unused screenshot name
    std::stringstream filenameSuffix;
    std::stringstream filename;
    std::fstream testExists;
    bool found = false;

    do {
        screenshotCount++;
        filename.str("");
        filenameSuffix.str("");
        filename << PHYSFS_getUserDir();
#if (defined __USE_UNIX98 || defined __FreeBSD__)
        filenameSuffix << ".tmw/";
#elif defined __APPLE__
        filenameSuffix << "Desktop/";
#endif
        filenameSuffix << "TMW_Screenshot_" << screenshotCount << ".png";
        filename << filenameSuffix.str();
        testExists.open(filename.str().c_str(), std::ios::in);
        found = !testExists.is_open();
        testExists.close();
    } while (!found);

    const bool success = ImageWriter::writePNG(screenshot, filename.str());

    if (success)
    {
        std::stringstream chatlogentry;
        chatlogentry << _("Screenshot saved to ~/") << filenameSuffix.str();
        chatWindow->chatLog(chatlogentry.str(), BY_SERVER);
    }
    else
    {
        chatWindow->chatLog(_("Saving screenshot failed!"), BY_SERVER);
        logger->log("Error: could not save screenshot.");
    }

    SDL_FreeSurface(screenshot);

    return success;
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
        mNetwork->flush();
        mNetwork->dispatchMessages();

        if (!mNetwork->isConnected())
        {
            if (!disconnectedDialog)
            {
                disconnectedDialog = new OkDialog(_("Network Error"),
                        _("The connection to the server was lost, "
                          "the program will now quit"));
                disconnectedDialog->addActionListener(&exitListener);
                disconnectedDialog->requestMoveToTop();
            }
        }
    }
}

void Game::handleInput()
{
    if (joystick)
        joystick->update();

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

            // Mode switch to emotes
            if (keyboard.isKeyActive(keyboard.KEY_EMOTE))
            {
                // Emotions
                int emotion = keyboard.getKeyEmoteOffset(event.key.keysym.sym);
                if (emotion)
                {
                    emoteShortcut->useEmote(emotion);
                    used = true;
                    return;
                }
            }

            if (keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT) ||
                keyboard.isKeyActive(keyboard.KEY_OK))
            {
                // Input chat window
                if (!(chatWindow->isInputFocused() ||
                                deathNotice ||
                                weightNotice))
                {
                    // Quit by pressing Enter if the exit confirm is there
                    if (exitConfirm &&
                        keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT))
                        done = true;
                    // Close the Browser if opened
                    else if (helpWindow->isVisible() &&
                             keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT))
                        helpWindow->setVisible(false);
                    // Close the config window, cancelling changes if opened
                    else if (setupWindow->isVisible() &&
                             keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT))
                        setupWindow->action(gcn::ActionEvent(NULL, "cancel"));
                    // Submits the text and proceeds to the next dialog
                    else if (npcStringDialog->isVisible() &&
                             keyboard.isKeyActive(keyboard.KEY_OK))
                        npcStringDialog->action(gcn::ActionEvent(NULL, "ok"));
                    // Proceed to the next dialog option, or close the window
                    else if (npcTextDialog->isVisible() &&
                             keyboard.isKeyActive(keyboard.KEY_OK))
                        npcTextDialog->action(gcn::ActionEvent(NULL, "ok"));
                    // Choose the currently highlighted dialogue option
                    else if (npcListDialog->isVisible() &&
                             keyboard.isKeyActive(keyboard.KEY_OK))
                        npcListDialog->action(gcn::ActionEvent(NULL, "ok"));
                    // Submits the text and proceeds to the next dialog
                    else if (npcIntegerDialog->isVisible() &&
                             keyboard.isKeyActive(keyboard.KEY_OK))
                        npcIntegerDialog->action(gcn::ActionEvent(NULL, "ok"));
                    else if (!(keyboard.getKeyValue(
                                   KeyboardConfig::KEY_TOGGLE_CHAT) ==
                               keyboard.getKeyValue(
                                   KeyboardConfig::KEY_OK) &&
                               (npcStringDialog->isVisible() ||
                                npcTextDialog->isVisible() ||
                                npcListDialog->isVisible() ||
                                npcIntegerDialog->isVisible())))
                    {
                        chatWindow->requestChatFocus();
                        used = true;
                    }
                }
            }

            const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);
            switch (tKey)
            {
                case KeyboardConfig::KEY_SCROLL_CHAT_UP:
                    if (chatWindow->isVisible())
                    {
                        chatWindow->scroll(-DEFAULT_CHAT_WINDOW_SCROLL);
                        used = true;
                    }
                    break;
                case KeyboardConfig::KEY_SCROLL_CHAT_DOWN:
                    if (chatWindow->isVisible())
                    {
                        chatWindow->scroll(DEFAULT_CHAT_WINDOW_SCROLL);
                        used = true;
                        return;
                    }
                    break;
                case KeyboardConfig::KEY_WINDOW_HELP:
                    // In-game Help
                    if (helpWindow->isVisible())
                        helpWindow->setVisible(false);
                    else
                    {
                        helpWindow->loadHelp("index");
                        helpWindow->requestMoveToTop();
                    }
                    used = true;
                    break;
               // Quitting confirmation dialog
               case KeyboardConfig::KEY_QUIT:
                    if (!exitConfirm)
                    {
                        exitConfirm = new ConfirmDialog(_("Quit"),
                                                        _("Are you sure you "
                                                          "want to quit?"));
                        exitConfirm->addActionListener(&exitListener);
                        exitConfirm->requestMoveToTop();
                    }
                    else
                    {
                        exitConfirm->action(gcn::ActionEvent(NULL, _("no")));
                    }
                    break;
                default:
                    break;
            }
            if (keyboard.isEnabled() && !chatWindow->isInputFocused()
                                     && !npcStringDialog->isInputFocused())
            {
                const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);

                // Do not activate shortcuts if tradewindow is visible
                if (!tradeWindow->isVisible())
                {
                    // Checks if any item shortcut is pressed.
                    for (int i = KeyboardConfig::KEY_SHORTCUT_1;
                             i <= KeyboardConfig::KEY_SHORTCUT_12;
                             i++)
                    {
                        if (tKey == i && !used)
                        {
                            itemShortcut->useItem(
                                          i - KeyboardConfig::KEY_SHORTCUT_1);
                            break;
                        }
                    }
                }

                switch (tKey)
                {
                    case KeyboardConfig::KEY_PICKUP:
                        {
                            FloorItem *item =
                                floorItemManager->findByCoordinates(
                                        player_node->mX, player_node->mY);

                            // If none below the player, try the tile in front
                            // of the player
                            if (!item)
                            {
                                Uint16 x = player_node->mX;
                                Uint16 y = player_node->mY;
                                if (player_node->getDirection() & Being::UP)
                                    y--;
                                if (player_node->getDirection() & Being::DOWN)
                                    y++;
                                if (player_node->getDirection() & Being::LEFT)
                                    x--;
                                if (player_node->getDirection() & Being::RIGHT)
                                    x++;

                                item = floorItemManager->findByCoordinates(
                                        x, y);
                            }

                            if (item)
                                player_node->pickUp(item);

                            used = true;
                        }
                        break;
                    case KeyboardConfig::KEY_SIT:
                        // Player sit action
                        player_node->toggleSit();
                        used = true;
                        break;
                    case KeyboardConfig::KEY_HIDE_WINDOWS:
                        // Hide certain windows
                        if (!chatWindow->isInputFocused())
                        {
                            statusWindow->setVisible(false);
                            inventoryWindow->setVisible(false);
                            emoteWindow->setVisible(false);
                            skillDialog->setVisible(false);
                            setupWindow->setVisible(false);
                            equipmentWindow->setVisible(false);
                            helpWindow->setVisible(false);
                            debugWindow->setVisible(false);
                        }
                        break;
                    case KeyboardConfig::KEY_WINDOW_STATUS:
                        requestedWindow = statusWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_INVENTORY:
                        requestedWindow = inventoryWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_EQUIPMENT:
                        requestedWindow = equipmentWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_SKILL:
                        requestedWindow = skillDialog;
                        break;
                    case KeyboardConfig::KEY_WINDOW_MINIMAP:
                        minimap->toggle();
                        requestedWindow = minimap;
                        break;
                    case KeyboardConfig::KEY_WINDOW_CHAT:
                        requestedWindow = chatWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_SHORTCUT:
                        requestedWindow = itemShortcutWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_SETUP:
                        requestedWindow = setupWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_DEBUG:
                        requestedWindow = debugWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_EMOTE:
                        requestedWindow = emoteWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_EMOTE_SHORTCUT:
                        requestedWindow = emoteShortcutWindow;
                        break;
                    case KeyboardConfig::KEY_SCREENSHOT:
                        // Screenshot (picture, hence the p)
                        saveScreenshot();
                        used = true;
                        break;
                    case KeyboardConfig::KEY_PATHFIND:
                        // Find path to mouse (debug purpose)
                        viewport->toggleDebugPath();
                        used = true;
                        break;
                    case KeyboardConfig::KEY_TRADE:
                        // Toggle accepting of incoming trade requests
                        unsigned int deflt = player_relations.getDefault();
                        if (deflt & PlayerRelation::TRADE)
                        {
                            chatWindow->chatLog(
                                              _("Ignoring incoming trade requests"),
                                                BY_SERVER);
                            deflt &= ~PlayerRelation::TRADE;
                        }
                        else
                        {
                            chatWindow->chatLog(
                                              _("Accepting incoming trade requests"),
                                                BY_SERVER);
                            deflt |= PlayerRelation::TRADE;
                        }

                        player_relations.setDefault(deflt);

                        used = true;
                        break;
                }
            }

            if (requestedWindow)
            {
                requestedWindow->setVisible(!requestedWindow->isVisible());
                if (requestedWindow->isVisible())
                    requestedWindow->requestMoveToTop();
                used = true;
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
        return;

    // Moving player around
    if (player_node->mAction != Being::DEAD &&
        current_npc == 0 && !chatWindow->isInputFocused())
    {
        // Get the state of the keyboard keys
        keyboard.refreshActiveKeys();

        const Uint16 x = player_node->mX;
        const Uint16 y = player_node->mY;
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

        // Attacking monsters
        if (keyboard.isKeyActive(keyboard.KEY_ATTACK) ||
           (joystick && joystick->buttonPressed(0)))
        {
            Being *target = beingManager->findNearestLivingBeing(x, y, 20,
                                                                 Being::MONSTER);

            bool newTarget = !keyboard.isKeyActive(keyboard.KEY_TARGET);
            // A set target has highest priority
            if (newTarget || !player_node->getTarget())
            {
                Uint16 targetX = x, targetY = y;

                switch (player_node->getSpriteDirection())
                {
                    case DIRECTION_UP   : --targetY; break;
                    case DIRECTION_DOWN : ++targetY; break;
                    case DIRECTION_LEFT : --targetX; break;
                    case DIRECTION_RIGHT: ++targetX; break;
                    default: break;
                }

                // Attack priorioty is: Monster, Player, auto target
                target = beingManager->findBeing(targetX, targetY, Being::MONSTER);
                if (!target)
                    target = beingManager->findBeing(targetX, targetY, Being::PLAYER);
            }

            player_node->attack(target, newTarget);
        }

        // Target the nearest player if 'q' is pressed
        if ( keyboard.isKeyActive(keyboard.KEY_TARGET_PLAYER) &&
                !keyboard.isKeyActive(keyboard.KEY_TARGET) )
        {
            Being *target = beingManager->findNearestLivingBeing(player_node, 20, Being::PLAYER);

            player_node->setTarget(target);
        }

        // Target the nearest monster if 'a' pressed
        if ((keyboard.isKeyActive(keyboard.KEY_TARGET_CLOSEST) ||
                    (joystick && joystick->buttonPressed(3))) &&
                !keyboard.isKeyActive(keyboard.KEY_TARGET))
        {
            Being *target = beingManager->findNearestLivingBeing(
                    x, y, 20, Being::MONSTER);

            player_node->setTarget(target);
        }

        // Target the nearest npc if 'n' pressed
        if ( keyboard.isKeyActive(keyboard.KEY_TARGET_NPC) &&
                !keyboard.isKeyActive(keyboard.KEY_TARGET) )
        {
            Being *target = beingManager->findNearestLivingBeing(
                    x, y, 20, Being::NPC);

            player_node->setTarget(target);
        }

        // Talk to the nearest NPC if 't' pressed
        if ( keyboard.isKeyActive(keyboard.KEY_TALK) )
        {
            if (!npcTextDialog->isVisible() && !npcListDialog->isVisible())
            {
                Being *target = player_node->getTarget();

                if (!target)
                {
                    target = beingManager->findNearestLivingBeing(
                            x, y, 20, Being::NPC);
                }

                if (target)
                {
                    if (target->getType() == Being::NPC)
                        dynamic_cast<NPC*>(target)->talk();
                }
            }
        }

        // Stop attacking if shift is pressed
        if (keyboard.isKeyActive(keyboard.KEY_TARGET))
        {
            player_node->stopAttack();
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
