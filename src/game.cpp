/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "game.h"

#include "actorspritemanager.h"
#include "actorsprite.h"
#include "channelmanager.h"
#include "client.h"
#include "commandhandler.h"
#include "configuration.h"
#include "effectmanager.h"
#include "event.h"
#include "emoteshortcut.h"
#include "graphics.h"
#include "itemshortcut.h"
#include "joystick.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "playerrelations.h"
#include "sound.h"

#include "gui/chat.h"
#include "gui/confirmdialog.h"
#include "gui/debugwindow.h"
#include "gui/equipmentwindow.h"
#include "gui/gui.h"
#include "gui/help.h"
#include "gui/inventorywindow.h"
#include "gui/shortcutwindow.h"
#include "gui/minimap.h"
#include "gui/ministatus.h"
#include "gui/npcdialog.h"
#include "gui/okdialog.h"
#include "gui/outfitwindow.h"
#include "gui/quitdialog.h"
#include "gui/sdlinput.h"
#include "gui/setup.h"
#include "gui/socialwindow.h"
#include "gui/specialswindow.h"
#include "gui/skilldialog.h"
#include "gui/statuswindow.h"
#include "gui/textdialog.h"
#include "gui/trade.h"
#include "gui/viewport.h"
#include "gui/windowmenu.h"

#include "gui/widgets/chattab.h"
#include "gui/widgets/emoteshortcutcontainer.h"
#include "gui/widgets/itemshortcutcontainer.h"

#include "net/gamehandler.h"
#include "net/generalhandler.h"
#include "net/net.h"
#include "net/playerhandler.h"

#include "resources/imagewriter.h"
#include "resources/mapreader.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/mkdir.h"

#include <guichan/exception.hpp>
#include <guichan/focushandler.hpp>

#include <physfs.h>

#include <fstream>
#include <sstream>
#include <string>

Joystick *joystick = NULL;

OkDialog *weightNotice = NULL;
OkDialog *deathNotice = NULL;
QuitDialog *quitDialog = NULL;
OkDialog *disconnectedDialog = NULL;

ChatWindow *chatWindow;
StatusWindow *statusWindow;
MiniStatusWindow *miniStatusWindow;
InventoryWindow *inventoryWindow;
SkillDialog *skillDialog;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
TradeWindow *tradeWindow;
HelpWindow *helpWindow;
DebugWindow *debugWindow;
ShortcutWindow *itemShortcutWindow;
ShortcutWindow *emoteShortcutWindow;
OutfitWindow *outfitWindow;
SpecialsWindow *specialsWindow;
SocialWindow *socialWindow;

ActorSpriteManager *actorSpriteManager = NULL;
ChannelManager *channelManager = NULL;
CommandHandler *commandHandler = NULL;
Particle *particleEngine = NULL;
EffectManager *effectManager = NULL;
Viewport *viewport = NULL;                    /**< Viewport on the map. */

ChatTab *localChatTab = NULL;

/**
 * Initialize every game sub-engines in the right order
 */
static void initEngines()
{
    Event::trigger(Event::GameChannel, Event::EnginesInitializing);

    actorSpriteManager = new ActorSpriteManager;
    commandHandler = new CommandHandler;
    channelManager = new ChannelManager;
    effectManager = new EffectManager;

    particleEngine = new Particle(NULL);
    particleEngine->setupEngine();

    Event::trigger(Event::GameChannel, Event::EnginesInitialized);
}

/**
 * Create all the various globally accessible gui windows
 */
static void createGuiWindows()
{
    Event::trigger(Event::GameChannel, Event::GuiWindowsLoading);

    setupWindow->clearWindowsForReset();

    // Create dialogs
    miniStatusWindow = new MiniStatusWindow;
    minimap = new Minimap;
    chatWindow = new ChatWindow;
    tradeWindow = new TradeWindow;
    switch (Net::getNetworkType())
    {
      case ServerInfo::TMWATHENA:
        equipmentWindow = new TmwAthena::TaEquipmentWindow(
                                                    PlayerInfo::getEquipment());
        break;
      case ServerInfo::MANASERV:
      default:
        equipmentWindow = new EquipmentWindow(PlayerInfo::getEquipment());
        break;
    }
    statusWindow = new StatusWindow;
    inventoryWindow = new InventoryWindow(PlayerInfo::getInventory());
    skillDialog = new SkillDialog;
    helpWindow = new HelpWindow;
    debugWindow = new DebugWindow;
    itemShortcutWindow = new ShortcutWindow("ItemShortcut",
                                            new ItemShortcutContainer);
    emoteShortcutWindow = new ShortcutWindow("EmoteShortcut",
                                             new EmoteShortcutContainer);
    outfitWindow = new OutfitWindow();
    specialsWindow = new SpecialsWindow();
    socialWindow = new SocialWindow();

    localChatTab = new ChatTab(_("General"));

    NpcDialog::setup();

    Event::trigger(Event::GameChannel, Event::GuiWindowsLoaded);
}

#define del_0(X) { delete X; X = 0; }

/**
 * Destroy all the globally accessible gui windows
 */
static void destroyGuiWindows()
{
    Event::trigger(Event::GameChannel, Event::GuiWindowsUnloading);

    del_0(localChatTab) // Need to do this first, so it can remove itself
    del_0(chatWindow)
    del_0(statusWindow)
    del_0(miniStatusWindow)
    del_0(inventoryWindow)
    del_0(skillDialog)
    del_0(minimap)
    del_0(equipmentWindow)
    del_0(tradeWindow)
    del_0(helpWindow)
    del_0(debugWindow)
    del_0(itemShortcutWindow)
    del_0(emoteShortcutWindow)
    del_0(outfitWindow)
    del_0(specialsWindow)
    del_0(socialWindow)

    Event::trigger(Event::NpcChannel, Event::CloseAll); // Cleanup remaining NPC dialogs

    Event::trigger(Event::GameChannel, Event::GuiWindowsUnloaded);
}

Game *Game::mInstance = 0;

Game::Game():
    mLastTarget(ActorSprite::UNKNOWN),
    mCurrentMap(0), mMapName("")
{
    assert(!mInstance);
    mInstance = this;

    disconnectedDialog = NULL;

    // Create the viewport
    viewport = new Viewport;
    viewport->setDimension(gcn::Rectangle(0, 0, graphics->getWidth(),
                                          graphics->getHeight()));

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());
    top->add(viewport);
    viewport->requestMoveToBottom();

    createGuiWindows();

    mWindowMenu = new WindowMenu;
    windowContainer->add(mWindowMenu);

    initEngines();

    // Initialize beings
    actorSpriteManager->setPlayer(player_node);

    /*
     * To prevent the server from sending data before the client
     * has initialized, I've modified it to wait for a "ping"
     * from the client to complete its initialization
     *
     * Note: This only affects the latest eAthena version.  This
     * packet is handled by the older version, but its response
     * is ignored by the client
     */
    Net::getGameHandler()->ping(tick_time);

    Joystick::init();
    // TODO: The user should be able to choose which one to use
    // Open the first device
    if (Joystick::getNumberOfJoysticks() > 0)
        joystick = new Joystick(0);

    setupWindow->setInGame(true);

    Event::trigger(Event::GameChannel, Event::Constructed);
}

Game::~Game()
{
    Event::trigger(Event::GameChannel, Event::Destructing);

    delete mWindowMenu;

    destroyGuiWindows();

    del_0(actorSpriteManager)
    if (Client::getState() != STATE_CHANGE_MAP)
        del_0(player_node)
    del_0(channelManager)
    del_0(commandHandler)
    del_0(joystick)
    del_0(particleEngine)
    del_0(viewport)
    del_0(mCurrentMap)

    mInstance = 0;

    Event::trigger(Event::GameChannel, Event::Destructed);
}

static bool saveScreenshot()
{
    static unsigned int screenshotCount = 0;

    // We don't want to show IP addresses in screenshots
    const bool showip = player_node->getShowIp();
    if (showip)
    {
        player_node->setShowIp(false);
        actorSpriteManager->updatePlayerNames();
        gui->draw();
    }

    SDL_Surface *screenshot = graphics->getScreenshot();

    if (showip)
    {
        player_node->setShowIp(true);
        actorSpriteManager->updatePlayerNames();
    }

    // Search for an unused screenshot name
    std::stringstream filenameSuffix;
    std::stringstream filename;
    std::fstream testExists;
    std::string screenshotDirectory = Client::getScreenshotDirectory();
    bool found = false;

    if (mkdir_r(screenshotDirectory.c_str()) != 0)
    {
        logger->log("Directory %s doesn't exist and can't be created! "
                    "Setting screenshot directory to home.",
                    screenshotDirectory.c_str());
        screenshotDirectory = std::string(PHYSFS_getUserDir());
    }

    do
    {
        screenshotCount++;
        filenameSuffix.str("");
        filename.str("");
        filename << screenshotDirectory << "/";
        filenameSuffix << branding.getValue("appShort", "Mana")
                       << "_Screenshot_" << screenshotCount << ".png";
        filename << filenameSuffix.str();
        testExists.open(filename.str().c_str(), std::ios::in);
        found = !testExists.is_open();
        testExists.close();
    }
    while (!found);

    const bool success = ImageWriter::writePNG(screenshot, filename.str());

    if (success)
    {
        std::stringstream chatlogentry;
        // TODO: Make it one complete gettext string below
        chatlogentry << _("Screenshot saved as ") << filenameSuffix.str();
        SERVER_NOTICE(chatlogentry.str())
    }
    else
    {
        SERVER_NOTICE(_("Saving screenshot failed!"))
        logger->log("Error: could not save screenshot.");
    }

    SDL_FreeSurface(screenshot);

    return success;
}

void Game::logic()
{
    handleInput();

    // Handle all necessary game logic
    ActorSprite::actorLogic();
    actorSpriteManager->logic();
    particleEngine->update();
    if (mCurrentMap)
        mCurrentMap->update();

    cur_time = time(NULL);

    // Handle network stuff
    if (!Net::getGameHandler()->isConnected())
    {
        if (Client::getState() == STATE_CHANGE_MAP)
            return; // Not a problem here

        if (Client::getState() == STATE_ERROR)
            return; // Disconnect gets handled by STATE_ERROR

        errorMessage = _("The connection to the server was lost.");

        if (!disconnectedDialog)
        {
            disconnectedDialog = new OkDialog(_("Network Error"),
                                              errorMessage);
            disconnectedDialog->addActionListener(&errorListener);
            disconnectedDialog->requestMoveToTop();
        }
    }
}

/**
 * The huge input handling method.
 */
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
            if (quitDialog || TextDialog::isActive() ||
                    PlayerInfo::getNPCPostCount() > 0)
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

            if (!chatWindow->isInputFocused()
                && !gui->getFocusHandler()->getModalFocused())
            {
                NpcDialog *dialog = NpcDialog::getActive();
                if (keyboard.isKeyActive(keyboard.KEY_OK)
                    && (!dialog || !dialog->isTextInputFocused()))
                {
                    // Close the Browser if opened
                    if (helpWindow->isVisible())
                        helpWindow->setVisible(false);
                    // Close the config window, cancelling changes if opened
                    else if (setupWindow->isVisible())
                        setupWindow->action(gcn::ActionEvent(NULL, "cancel"));
                    else if (dialog)
                        dialog->action(gcn::ActionEvent(NULL, "ok"));
                }
                if (keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT))
                {
                    if (chatWindow->requestChatFocus())
                        used = true;
                }
                if (dialog)
                {
                    if (keyboard.isKeyActive(keyboard.KEY_MOVE_UP))
                        dialog->move(1);
                    else if (keyboard.isKeyActive(keyboard.KEY_MOVE_DOWN))
                        dialog->move(-1);
                }
            }


            if (!chatWindow->isInputFocused() || (event.key.keysym.mod &
                                                  KMOD_ALT))
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

            if (!chatWindow->isInputFocused())
            {
                bool wearOutfit = false;
                bool copyOutfit = false;

                if (keyboard.isKeyActive(keyboard.KEY_WEAR_OUTFIT))
                    wearOutfit = true;

                if (keyboard.isKeyActive(keyboard.KEY_COPY_OUTFIT))
                    copyOutfit = true;

                if (wearOutfit || copyOutfit)
                {
                    int outfitNum = -1;
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_1:
                        case SDLK_2:
                        case SDLK_3:
                        case SDLK_4:
                        case SDLK_5:
                        case SDLK_6:
                        case SDLK_7:
                        case SDLK_8:
                        case SDLK_9:
                            outfitNum = event.key.keysym.sym - SDLK_1;
                            break;

                        case SDLK_0:
                            outfitNum = 9;
                            break;

                        case SDLK_MINUS:
                            outfitNum = 10;
                            break;

                        case SDLK_EQUALS:
                            outfitNum = 11;
                            break;

                        case SDLK_BACKSPACE:
                            outfitNum = 12;
                            break;

                        case SDLK_INSERT:
                            outfitNum = 13;
                            break;

                        case SDLK_HOME:
                            outfitNum = 14;
                            break;

                        default:
                            break;
                    }
                    if (outfitNum >= 0)
                    {
                        used = true;
                        if (wearOutfit)
                            outfitWindow->wearOutfit(outfitNum);
                        else if (copyOutfit)
                            outfitWindow->copyOutfit(outfitNum);
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
                    quitDialog = new QuitDialog(&quitDialog);
                    quitDialog->requestMoveToTop();
                    return;
                default:
                    break;
            }
            if (keyboard.isEnabled() && !chatWindow->isInputFocused() &&
                !NpcDialog::isAnyInputFocused() && !InventoryWindow::isAnyInputFocused())
            {
                const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);

                // Do not activate shortcuts if tradewindow is visible
                if (!tradeWindow->isVisible() && !setupWindow->isVisible())
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
                            int x = player_node->getTileX();
                            int y = player_node->getTileY();

                            FloorItem *item =
                                    actorSpriteManager->findItem(x, y);

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

                                item = actorSpriteManager->findItem(x, y);
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
                            skillDialog->setVisible(false);
                            setupWindow->setVisible(false);
                            equipmentWindow->setVisible(false);
                            helpWindow->setVisible(false);
                            debugWindow->setVisible(false);
                            socialWindow->setVisible(false);
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
                    case KeyboardConfig::KEY_WINDOW_SOCIAL:
                        requestedWindow = socialWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_EMOTE_SHORTCUT:
                        requestedWindow = emoteShortcutWindow;
                        break;
                    case KeyboardConfig::KEY_WINDOW_OUTFIT:
                        requestedWindow = outfitWindow;
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
                            SERVER_NOTICE(_("Ignoring incoming trade requests"))
                            deflt &= ~PlayerRelation::TRADE;
                        }
                        else
                        {
                            SERVER_NOTICE(_("Accepting incoming trade requests"))
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
            Client::setState(STATE_EXIT);
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
                const char *err = e.getMessage().c_str();
                logger->log("Warning: guichan input exception: %s", err);
            }
        }

    } // End while

    // If the user is configuring the keys then don't respond.
    if (!keyboard.isEnabled())
        return;

    // Moving player around
    if (player_node->isAlive() && !PlayerInfo::isTalking() &&
        !chatWindow->isInputFocused() && !quitDialog && !TextDialog::isActive())
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

        if (keyboard.isKeyActive(keyboard.KEY_EMOTE) && direction != 0)
        {
            if (player_node->getDirection() != direction)
            {
                player_node->setDirection(direction);
                Net::getPlayerHandler()->setDirection(direction);
            }
            direction = 0;
        }
        else
        {
            player_node->setWalkingDir(direction);
        }

        // Attacking monsters
        if (keyboard.isKeyActive(keyboard.KEY_ATTACK) ||
           (joystick && joystick->buttonPressed(0)))
        {
            if (player_node->getTarget())
                player_node->attack(player_node->getTarget(), true);
        }

        if (keyboard.isKeyActive(keyboard.KEY_TARGET_ATTACK))
        {
            Being *target = 0;

            bool newTarget = !keyboard.isKeyActive(keyboard.KEY_TARGET);
            // A set target has highest priority
            if (!player_node->getTarget())
            {
                // Only auto target Monsters
                target = actorSpriteManager->findNearestLivingBeing(player_node,
                                                    20, ActorSprite::MONSTER);
            }
            player_node->attack(target, newTarget);
        }

        // Target the nearest player/monster/npc
        if ((keyboard.isKeyActive(keyboard.KEY_TARGET_PLAYER) ||
            keyboard.isKeyActive(keyboard.KEY_TARGET_CLOSEST) ||
            keyboard.isKeyActive(keyboard.KEY_TARGET_NPC) ||
                    (joystick && joystick->buttonPressed(3))) &&
                !keyboard.isKeyActive(keyboard.KEY_TARGET))
        {
            ActorSprite::Type currentTarget = ActorSprite::UNKNOWN;
            if (keyboard.isKeyActive(keyboard.KEY_TARGET_CLOSEST) ||
                    (joystick && joystick->buttonPressed(3)))
                currentTarget = ActorSprite::MONSTER;
            else if (keyboard.isKeyActive(keyboard.KEY_TARGET_PLAYER))
                currentTarget = ActorSprite::PLAYER;
            else if (keyboard.isKeyActive(keyboard.KEY_TARGET_NPC))
                currentTarget = ActorSprite::NPC;

            Being *target = actorSpriteManager->findNearestLivingBeing(player_node,
                                                    20, currentTarget);

            if (target && (target != player_node->getTarget() ||
                    currentTarget != mLastTarget))
            {
                player_node->setTarget(target);
                mLastTarget = currentTarget;
            }
        }
        else
        {
            mLastTarget = ActorSprite::UNKNOWN; // Reset last target
        }

        // Talk to the nearest NPC if 't' pressed
        if (event.type == SDL_KEYDOWN &&
            keyboard.getKeyIndex(event.key.keysym.sym) == KeyboardConfig::KEY_TALK)
        {
            Being *target = player_node->getTarget();

            if (target)
            {
                if (target->canTalk())
                    target->talkTo();
            }
        }

        // Stop attacking if the right key is pressed
        if (!keyboard.isKeyActive(keyboard.KEY_ATTACK)
            && keyboard.isKeyActive(keyboard.KEY_TARGET))
        {
            player_node->stopAttack();
        }

        if (joystick)
        {
            if (joystick->buttonPressed(1))
            {
                const int x = player_node->getTileX();
                const int y = player_node->getTileY();

                FloorItem *item = actorSpriteManager->findItem(x, y);

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

/**
 * Changes the currently active map. Should only be called while the game is
 * running.
 */
void Game::changeMap(const std::string &mapPath)
{
    // Clean up floor items, beings and particles
    actorSpriteManager->clear();

    // Close the popup menu on map change so that invalid options can't be
    // executed.
    viewport->closePopupMenu();

    // Unset the map of the player so that its particles are cleared before
    // being deleted in the next step
    if (player_node)
        player_node->setMap(0);

    particleEngine->clear();

    mMapName = mapPath;

    std::string fullMap = paths.getValue("maps", "maps/")
                          + mMapName + ".tmx";
    ResourceManager *resman = ResourceManager::getInstance();
    if (!resman->exists(fullMap))
        fullMap += ".gz";

    // Attempt to load the new map
    Map *newMap = MapReader::readMap(fullMap);

    if (!newMap)
    {
        logger->log("Error while loading %s", fullMap.c_str());
        new OkDialog(_("Could Not Load Map"),
                     strprintf(_("Error while loading %s"), fullMap.c_str()));
    }

    // Notify the minimap and beingManager about the map change
    minimap->setMap(newMap);
    actorSpriteManager->setMap(newMap);
    particleEngine->setMap(newMap);
    viewport->setMap(newMap);

    // Initialize map-based particle effects
    if (newMap)
        newMap->initializeParticleEffects(particleEngine);

    // Start playing new music file when necessary
    std::string oldMusic = mCurrentMap ? mCurrentMap->getMusicFile() : "";
    std::string newMusic = newMap ? newMap->getMusicFile() : "";
    if (newMusic != oldMusic)
    {
        if (newMusic.empty())
            sound.stopMusic();
        else
            sound.playMusic(newMusic);
    }

    delete mCurrentMap;
    mCurrentMap = newMap;

    Event event(Event::MapLoaded);
    event.setString("mapPath", mapPath);
    event.trigger(Event::GameChannel);
}
