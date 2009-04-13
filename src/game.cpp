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
#include "channelmanager.h"
#include "commandhandler.h"
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

#include "gui/widgets/chattab.h"
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
#include "gui/partywindow.h"
#ifdef TMWSERV_SUPPORT
#include "gui/buddywindow.h"
#include "gui/guildwindow.h"
#include "gui/magic.h"
#include "gui/quitdialog.h"
#endif
#include "gui/npcpostdialog.h"
#include "gui/storagewindow.h"

#include "net/generalhandler.h"
#include "net/maphandler.h"
#include "net/net.h"

#include "net/tmwserv/inventoryhandler.h"
#include "net/ea/inventoryhandler.h"

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
#ifdef TMWSERV_SUPPORT
QuitDialog *quitDialog = NULL;
#else
ConfirmDialog *exitConfirm = NULL;
#endif
OkDialog *disconnectedDialog = NULL;

ChatWindow *chatWindow;
MenuWindow *menuWindow;
StatusWindow *statusWindow;
MiniStatusWindow *miniStatusWindow;
BuyDialog *buyDialog;
SellDialog *sellDialog;
#ifdef EATHENA_SUPPORT
BuySellDialog *buySellDialog;
#endif
InventoryWindow *inventoryWindow;
EmoteWindow *emoteWindow;
NpcIntegerDialog *npcIntegerDialog;
NpcListDialog *npcListDialog;
NpcTextDialog *npcTextDialog;
NpcStringDialog *npcStringDialog;
SkillDialog *skillDialog;
PartyWindow *partyWindow;
#ifdef TMWSERV_SUPPORT
BuddyWindow *buddyWindow;
GuildWindow *guildWindow;
MagicDialog *magicDialog;
#endif
NpcPostDialog *npcPostDialog;
StorageWindow *storageWindow;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
TradeWindow *tradeWindow;
HelpWindow *helpWindow;
DebugWindow *debugWindow;
ShortcutWindow *itemShortcutWindow;
ShortcutWindow *emoteShortcutWindow;

BeingManager *beingManager = NULL;
FloorItemManager *floorItemManager = NULL;
ChannelManager *channelManager = NULL;
CommandHandler *commandHandler = NULL;
Particle *particleEngine = NULL;
EffectManager *effectManager = NULL;

ChatTab *localChatTab = NULL;

const int MAX_TIME = 10000;

/**
 * Listener used for exiting handling.
 */
namespace {
    struct ExitListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            if (event.getId() == "yes" || event.getId() == "ok")
                done = true;

#ifdef EATHENA_SUPPORT
            exitConfirm = NULL;
#endif
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
        return (tick_time - start_time) * 10;
    else
        return (tick_time + (MAX_TIME - start_time)) * 10;
}

/**
 * Create all the various globally accessible gui windows
 */
static void createGuiWindows()
{
    // Create dialogs
    chatWindow = new ChatWindow;
    buyDialog = new BuyDialog;
    sellDialog = new SellDialog;
    tradeWindow = new TradeWindow;
    npcTextDialog = new NpcTextDialog;
    npcIntegerDialog = new NpcIntegerDialog;
    npcListDialog = new NpcListDialog;
    npcStringDialog = new NpcStringDialog;
    partyWindow = new PartyWindow;
#ifdef TMWSERV_SUPPORT
    magicDialog = new MagicDialog;
    equipmentWindow = new EquipmentWindow(player_node->mEquipment.get());
    buddyWindow = new BuddyWindow;
    guildWindow = new GuildWindow;
#else
    buySellDialog = new BuySellDialog;
    equipmentWindow = new EquipmentWindow;
#endif
    npcPostDialog = new NpcPostDialog;
    storageWindow = new StorageWindow;
    menuWindow = new MenuWindow;
    statusWindow = new StatusWindow(player_node);
    miniStatusWindow = new MiniStatusWindow;
    inventoryWindow = new InventoryWindow;
    emoteWindow = new EmoteWindow;
    skillDialog = new SkillDialog;
    minimap = new Minimap;
    helpWindow = new HelpWindow;
    debugWindow = new DebugWindow;
    itemShortcutWindow = new ShortcutWindow("ItemShortcut",
                                            new ItemShortcutContainer);
    emoteShortcutWindow = new ShortcutWindow("emoteShortcut",
                                             new EmoteShortcutContainer);

    localChatTab = new ChatTab(_("General"));

    // Set initial window visibility
    chatWindow->setVisible((bool) config.getValue(
        chatWindow->getWindowName() + "Visible", true));
    miniStatusWindow->setVisible((bool) config.getValue(
        miniStatusWindow->getPopupName() + "Visible", true));
    buyDialog->setVisible(false);
    sellDialog->setVisible(false);
    minimap->setVisible((bool) config.getValue(
        minimap->getWindowName() + "Visible", true));
    tradeWindow->setVisible(false);
    menuWindow->setVisible((bool) config.getValue(
        menuWindow->getPopupName() + "Visible", true));
    itemShortcutWindow->setVisible((bool) config.getValue(
        itemShortcutWindow->getWindowName() + "Visible", true));
    emoteShortcutWindow->setVisible((bool) config.getValue(
        emoteShortcutWindow->getWindowName() + "Visible", true));
    minimap->setVisible((bool) config.getValue(
        minimap->getWindowName() + "Visible", true));
#ifdef EATHENA_SUPPORT
    buySellDialog->setVisible(false);
#endif
    npcTextDialog->setVisible(false);
    npcIntegerDialog->setVisible(false);
    npcListDialog->setVisible(false);
    npcStringDialog->setVisible(false);
#ifdef EATHENA_SUPPORT
    storageWindow->setVisible(false);
#endif

    if (config.getValue("logToChat", 0))
    {
        logger->setChatWindow(chatWindow);
    }

    Net::getGeneralHandler()->guiWindowsLoaded();
}

/**
 * Destroy all the globally accessible gui windows
 */
static void destroyGuiWindows()
{
    Net::getGeneralHandler()->guiWindowsUnloaded();
    logger->setChatWindow(NULL);
    delete localChatTab; // Need to do this first, so it can remove itself
    delete chatWindow;
    delete statusWindow;
    delete miniStatusWindow;
    delete menuWindow;
    delete buyDialog;
    delete sellDialog;
#ifdef EATHENA_SUPPORT
    delete buySellDialog;
#endif
    delete inventoryWindow;
    delete emoteWindow;
    delete npcIntegerDialog;
    delete npcListDialog;
    delete npcTextDialog;
    delete npcStringDialog;
    delete partyWindow;
    delete npcPostDialog;
#ifdef TMWSERV_SUPPORT
    delete magicDialog;
    delete buddyWindow;
    delete guildWindow;
#endif
    delete skillDialog;
    delete minimap;
    delete equipmentWindow;
    delete tradeWindow;
    delete helpWindow;
    delete debugWindow;
    delete itemShortcutWindow;
    delete emoteShortcutWindow;
    delete storageWindow;
}

Game::Game():
    mLastTarget(Being::UNKNOWN),
    mLogicCounterId(0), mSecondsCounterId(0)
{
    done = false;

    createGuiWindows();
    engine = new Engine;

    beingManager = new BeingManager;
    commandHandler = new CommandHandler;
    floorItemManager = new FloorItemManager;
    channelManager = new ChannelManager;
    effectManager = new EffectManager;

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
        joystick = new Joystick(0);

#ifdef EATHENA_SUPPORT
    map_path = map_path.substr(0, map_path.rfind("."));
    engine->changeMap(map_path);
#endif

    setupWindow->setInGame(true);

    /*
     * To prevent the server from sending data before the client
     * has initialized, I've modified it to wait for a "ping"
     * from the client to complete its initialization
     *
     * Note: This only affects the latest eAthena version.  This
     * packet is handled by the older version, but its response
     * is ignored by the client
     */
    Net::getMapHandler()->ping(tick_time);
}

Game::~Game()
{
    Net::getGeneralHandler()->unload();

    destroyGuiWindows();

    delete beingManager;
    delete player_node;
    delete floorItemManager;
    delete channelManager;
    delete commandHandler;
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
        localChatTab->chatLog(chatlogentry.str(), BY_SERVER);
    }
    else
    {
        localChatTab->chatLog(_("Saving screenshot failed!"), BY_SERVER);
        logger->log("Error: could not save screenshot.");
    }

    SDL_FreeSurface(screenshot);

    return success;
}

void Game::optionChanged(const std::string &name)
{
    int fpsLimit = (int) config.getValue("fpslimit", 0);

    // Calculate new minimum frame time. If one isn't set, use 60 FPS.
    // (1000 / 60 is 16.66) Since the client can go well above the refresh
    // rates for monitors now in OpenGL mode, this cutoff is done to help
    // conserve on CPU time.
    mMinFrameTime = fpsLimit ? 1000 / fpsLimit : 16;

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
        Net::getGeneralHandler()->flushNetwork();
        if (!Net::getGeneralHandler()->isNetworkConnected())
        {
            if (!disconnectedDialog)
            {
                disconnectedDialog = new OkDialog(_("Network Error"),
                                                  _("The connection to the "
                                                    "server was lost, the "
                                                    "program will now quit"));
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

            // send straight to gui for certain windows
            if (npcPostDialog->isVisible())
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

            if (!(chatWindow->isInputFocused() || deathNotice || weightNotice))
                if (keyboard.isKeyActive(keyboard.KEY_OK))
                {
#ifdef TMWSERV_SUPPORT
                    // Don not focus chat input when quit dialog is active
                    if (quitDialog != NULL && quitDialog->isVisible())
                        continue;
#else
                    if (exitConfirm &&
                        keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT))
                        done = true;
#endif
                    // Close the Browser if opened
                    else if (helpWindow->isVisible() &&
                                keyboard.isKeyActive(keyboard.KEY_OK))
                        helpWindow->setVisible(false);
                    // Close the config window, cancelling changes if opened
                    else if (setupWindow->isVisible() &&
                                keyboard.isKeyActive(keyboard.KEY_OK))
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
                    /*
                    else if (guildWindow->isVisible())
                    {
                        // TODO: Check if a dialog is open and close it if so
                    }
                    */
                }
                if (keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT))
                {
                    if (chatWindow->requestChatFocus())
                        used = true;
                }


            if (!chatWindow->isInputFocused() || (event.key.keysym.mod & KMOD_ALT))
            {
                if (keyboard.isKeyActive(keyboard.KEY_PREV_CHAT_TAB))
                {
                    chatWindow->prevTab();
                    return;
                }
                else if (keyboard.isKeyActive(keyboard.KEY_NEXT_CHAT_TAB))
                {
                    chatWindow->nextTab();
                    return;
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
#ifdef TMWSERV_SUPPORT
                    if (!quitDialog)
                    {
                        quitDialog = new QuitDialog(&done, &quitDialog);
                        quitDialog->requestMoveToTop();
                    }
                    else
                    {
                        quitDialog->action(gcn::ActionEvent(NULL, "cancel"));
                    }
#else
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
#endif
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
#ifdef TMWSERV_SUPPORT
                            const Vector &pos = player_node->getPosition();
                            Uint16 x = (int) pos.x / 32;
                            Uint16 y = (int) pos.y / 32;
#else
                            Uint16 x = player_node->mX;
                            Uint16 y = player_node->mY;
#endif
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
#ifdef TMWSERV_SUPPORT
                            guildWindow->setVisible(false);
                            buddyWindow->setVisible(false);
#endif
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
                            localChatTab->chatLog(
                                        _("Ignoring incoming trade requests"),
                                        BY_SERVER);
                            deflt &= ~PlayerRelation::TRADE;
                        }
                        else
                        {
                            localChatTab->chatLog(
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
    if (player_node->mAction != Being::DEAD && current_npc == 0 &&
        !chatWindow->isInputFocused())
    {
        // Get the state of the keyboard keys
        keyboard.refreshActiveKeys();

        // Ignore input if either "ignore" key is pressed
        // Stops the character moving about if the user's window manager
        // uses "ignore+arrow key" to switch virtual desktops.
        if (keyboard.isKeyActive(keyboard.KEY_IGNORE_INPUT_1) ||
            keyboard.isKeyActive(keyboard.KEY_IGNORE_INPUT_2))
        {
            return;
        }

#ifdef TMWSERV_SUPPORT
        const Vector &pos = player_node->getPosition();
        const Uint16 x = (int) pos.x / 32;
        const Uint16 y = (int) pos.y / 32;
#else
        const Uint16 x = player_node->mX;
        const Uint16 y = player_node->mY;
#endif
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

#ifdef TMWSERV_SUPPORT
        // First if player is pressing key for the direction he is already
        // going
        if (direction == player_node->getWalkingDir())
        {
            player_node->setWalkingDir(direction);
        }
        // Else if he is pressing a key, and its different from what he has
        // been pressing, stop (do not send this stop to the server) and
        // start in the new direction
        else if (direction && direction != player_node->getWalkingDir())
        {
            player_node->stopWalking(false);
            player_node->setWalkingDir(direction);
        }
        // Else, he is not pressing a key, stop (sending to server)
        else
        {
            player_node->stopWalking(true);
        }
#else
        player_node->setWalkingDir(direction);

        // Attacking monsters
        if (keyboard.isKeyActive(keyboard.KEY_ATTACK) ||
           (joystick && joystick->buttonPressed(0)))
        {
            Being *target = NULL;

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

                // Only auto target Monsters
                target = beingManager->findNearestLivingBeing(targetX, targetY,
                         20, Being::MONSTER);
            }

            player_node->attack(target, newTarget);
        }
#endif

        // Target the nearest player/monster/npc
        if ((keyboard.isKeyActive(keyboard.KEY_TARGET_PLAYER) ||
            keyboard.isKeyActive(keyboard.KEY_TARGET_CLOSEST) ||
            keyboard.isKeyActive(keyboard.KEY_TARGET_NPC) ||
                    (joystick && joystick->buttonPressed(3))) &&
                !keyboard.isKeyActive(keyboard.KEY_TARGET))
        {
            Being::Type currentTarget = Being::UNKNOWN;
            if (keyboard.isKeyActive(keyboard.KEY_TARGET_CLOSEST) ||
                    (joystick && joystick->buttonPressed(3)))
                currentTarget = Being::MONSTER;
            else if (keyboard.isKeyActive(keyboard.KEY_TARGET_PLAYER))
                currentTarget = Being::PLAYER;
            else if (keyboard.isKeyActive(keyboard.KEY_TARGET_NPC))
                currentTarget = Being::NPC;

            Being *target = beingManager->findNearestLivingBeing(player_node,
                                                    20, currentTarget);

            if (target && (target != player_node->getTarget() ||
                    currentTarget != mLastTarget))
            {
                player_node->setTarget(target);
                mLastTarget = currentTarget;
            }
        } else mLastTarget = Being::UNKNOWN; // Reset last target

        // Talk to the nearest NPC if 't' pressed
        if ( keyboard.isKeyActive(keyboard.KEY_TALK) )
        {
            if (!npcTextDialog->isVisible()   && !npcListDialog->isVisible() &&
                !npcStringDialog->isVisible() && !npcIntegerDialog->isVisible())
            {
                Being *target = player_node->getTarget();

                if (target)
                {
                    if (target->getType() == Being::NPC)
                        dynamic_cast<NPC*>(target)->talk();
                }
            }
        }

        // Stop attacking if the right key is pressed
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
