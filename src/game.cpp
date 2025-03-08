/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "actorsprite.h"
#include "actorspritemanager.h"
#include "channelmanager.h"
#include "client.h"
#include "commandhandler.h"
#include "configuration.h"
#include "effectmanager.h"
#include "emoteshortcut.h"
#include "event.h"
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

#include "gui/abilitieswindow.h"
#include "gui/chatwindow.h"
#include "gui/debugwindow.h"
#include "gui/equipmentwindow.h"
#include "gui/gui.h"
#include "gui/helpwindow.h"
#include "gui/inventorywindow.h"
#include "gui/minimap.h"
#include "gui/ministatuswindow.h"
#include "gui/npcdialog.h"
#include "gui/okdialog.h"
#include "gui/outfitwindow.h"
#include "gui/questswindow.h"
#include "gui/quitdialog.h"
#include "gui/setup.h"
#include "gui/shortcutwindow.h"
#include "gui/skilldialog.h"
#include "gui/socialwindow.h"
#include "gui/statuswindow.h"
#include "gui/textdialog.h"
#include "gui/tradewindow.h"
#include "gui/viewport.h"

#include "gui/widgets/chattab.h"
#include "gui/widgets/emoteshortcutcontainer.h"
#include "gui/widgets/itemshortcutcontainer.h"

#include "net/gamehandler.h"
#include "net/net.h"
#include "net/playerhandler.h"

#include "resources/imagewriter.h"
#include "resources/mapreader.h"

#include "utils/filesystem.h"
#include "utils/gettext.h"
#include "utils/mkdir.h"

#include <guichan/focushandler.hpp>

#include <fstream>
#include <sstream>

Joystick *joystick;

OkDialog *weightNotice;
OkDialog *deathNotice;
QuitDialog *quitDialog;

ChatWindow *chatWindow;
StatusWindow *statusWindow;
MiniStatusWindow *miniStatusWindow;
InventoryWindow *inventoryWindow;
SkillDialog *skillDialog;
QuestsWindow *questsWindow;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
TradeWindow *tradeWindow;
HelpWindow *helpWindow;
DebugWindow *debugWindow;
ShortcutWindow *itemShortcutWindow;
ShortcutWindow *emoteShortcutWindow;
OutfitWindow *outfitWindow;
AbilitiesWindow *abilitiesWindow;
SocialWindow *socialWindow;

ActorSpriteManager *actorSpriteManager;
ChannelManager *channelManager;
CommandHandler *commandHandler;
Particle *particleEngine;
EffectManager *effectManager;
Viewport *viewport;                     /**< Viewport on the map. */

ChatTab *localChatTab;

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

    particleEngine = new Particle(nullptr);
    Particle::setupEngine();

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
    equipmentWindow = new EquipmentWindow(PlayerInfo::getEquipment());
    statusWindow = new StatusWindow;
    inventoryWindow = new InventoryWindow(PlayerInfo::getInventory());
    skillDialog = new SkillDialog;
    questsWindow = new QuestsWindow;
    helpWindow = new HelpWindow;
    debugWindow = new DebugWindow;
    itemShortcutWindow = new ShortcutWindow("ItemShortcut",
                                            new ItemShortcutContainer);
    emoteShortcutWindow = new ShortcutWindow("EmoteShortcut",
                                             new EmoteShortcutContainer);
    outfitWindow = new OutfitWindow();
    abilitiesWindow = new AbilitiesWindow();
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
    del_0(questsWindow)
    del_0(minimap)
    del_0(equipmentWindow)
    del_0(tradeWindow)
    del_0(helpWindow)
    del_0(debugWindow)
    del_0(itemShortcutWindow)
    del_0(emoteShortcutWindow)
    del_0(outfitWindow)
    del_0(abilitiesWindow)
    del_0(socialWindow)

    Event::trigger(Event::NpcChannel, Event::CloseAll); // Cleanup remaining NPC dialogs

    Event::trigger(Event::GameChannel, Event::GuiWindowsUnloaded);
}

Game *Game::mInstance = nullptr;

Game::Game():
    mLastTarget(ActorSprite::UNKNOWN)
{
    assert(!mInstance);
    mInstance = this;

    mParticleEngineTimer.set();

    // Create the viewport
    viewport = new Viewport;
    viewport->setSize(graphics->getWidth(), graphics->getHeight());

    auto *top = static_cast<gcn::Container*>(gui->getTop());
    top->add(viewport);
    viewport->requestMoveToBottom();

    createGuiWindows();

    mWindowMenu = new WindowMenu;
    windowContainer->add(mWindowMenu);

    initEngines();

    // Initialize beings
    actorSpriteManager->setPlayer(local_player);

    Joystick::init();
    // TODO: The user should be able to choose which one to use
    // Open the first device
    if (Joystick::getNumberOfJoysticks() > 0)
        joystick = new Joystick(0);

    Event::trigger(Event::GameChannel, Event::Constructed);
}

Game::~Game()
{
    Event::trigger(Event::GameChannel, Event::Destructing);

    delete mWindowMenu;

    destroyGuiWindows();

    del_0(actorSpriteManager)
    if (Client::getState() != STATE_CHANGE_MAP)
        del_0(local_player)
    del_0(effectManager)
    del_0(channelManager)
    del_0(commandHandler)
    del_0(joystick)
    del_0(particleEngine)
    del_0(viewport)
    del_0(mCurrentMap)

    mInstance = nullptr;

    Event::trigger(Event::GameChannel, Event::Destructed);
}

static bool saveScreenshot()
{
    static unsigned int screenshotCount = 0;

    // We don't want to show IP addresses in screenshots
    const bool showip = local_player->getShowIp();
    if (showip)
    {
        local_player->setShowIp(false);
        actorSpriteManager->updatePlayerNames();
        gui->draw();
    }

    SDL_Surface *screenshot = graphics->getScreenshot();

    if (showip)
    {
        local_player->setShowIp(true);
        actorSpriteManager->updatePlayerNames();
    }

    if (!screenshot)
    {
        serverNotice(_("Could not take screenshot!"));
        Log::error("Could not take screenshot.");
        return false;
    }

    // Search for an unused screenshot name
    std::ostringstream filenameSuffix;
    std::ostringstream filename;
    std::ifstream testExists;
    std::string screenshotDirectory = Client::getScreenshotDirectory();
    bool found = false;

    if (mkdir_r(screenshotDirectory.c_str()) != 0)
    {
        Log::info("Directory %s doesn't exist and can't be created! "
                  "Setting screenshot directory to home.",
                  screenshotDirectory.c_str());
        screenshotDirectory = FS::getUserDir();
    }

    do
    {
        screenshotCount++;
        filenameSuffix.str(std::string());
        filenameSuffix << branding.getValue("appShort", "Mana")
                       << "_Screenshot_" << screenshotCount << ".png";
        filename.str(std::string());
        filename << screenshotDirectory << "/" << filenameSuffix.str();
        testExists.open(filename.str());
        found = !testExists.is_open();
        testExists.close();
    }
    while (!found);

    const bool success = ImageWriter::writePNG(screenshot, filename.str());

    if (success)
    {
        std::string screenshotLink;
#if SDL_VERSION_ATLEAST(2, 0, 14)
        screenshotLink = strprintf("@@screenshot:%s|%s@@",
                                   filenameSuffix.str().c_str(),
                                   filenameSuffix.str().c_str());
#else
        screenshotLink = filenameSuffix.str();
#endif
        serverNotice(strprintf(_("Screenshot saved as %s"),
                               screenshotLink.c_str()));
    }
    else
    {
        serverNotice(_("Saving screenshot failed!"));
        Log::error("Could not save screenshot.");
    }

    SDL_FreeSurface(screenshot);

    return success;
}

void Game::logic()
{
    // Handle all necessary game logic
    actorSpriteManager->logic();

    // todo: make Particle::update work with variable time steps
    while (mParticleEngineTimer.passed())
    {
        particleEngine->update();
        mParticleEngineTimer.extend(MILLISECONDS_IN_A_TICK);
    }

    if (mCurrentMap)
        mCurrentMap->update(Time::deltaTimeMs());

    // Handle network stuff
    if (!Net::getGameHandler()->isConnected() && !mDisconnected)
    {
        if (Client::getState() == STATE_CHANGE_MAP)
            return; // Not a problem here

        if (Client::getState() == STATE_ERROR)
            return; // Disconnect gets handled by STATE_ERROR

        errorMessage = _("The connection to the server was lost.");
        Client::instance()->showOkDialog(_("Network Error"),
                                         errorMessage,
                                         STATE_CHOOSE_SERVER);
        mDisconnected = true;
    }
}

/**
 * handle item pick up case.
 */
static void handleItemPickUp()
{
    int x = local_player->getTileX();
    int y = local_player->getTileY();

    // Let's look for items around until you find one.
    FloorItem *item = actorSpriteManager->findItem(x, y, 1);
    if (!item)
        return;

    local_player->pickUp(item);
}

/**
 * Handles an SDL_KEYDOWN event and returns whether it was consumed.
 */
bool Game::keyDownEvent(SDL_KeyboardEvent &event)
{
    gcn::Window *requestedWindow = nullptr;

    // send straight to gui for certain windows
    if (quitDialog || TextDialog::isActive() ||
        PlayerInfo::getNPCPostCount() > 0)
    {
        return false;
    }

    if (keyboard.isKeyActive(KeyboardConfig::KEY_EMOTE))
    {
        int emotion = keyboard.getKeyEmoteOffset(event.keysym.sym);
        if (emotion != -1)
        {
            emoteShortcut->useEmote(emotion);
            return true;
        }
    }

    if (!chatWindow->isInputFocused()
        && !gui->getFocusHandler()->getModalFocused())
    {
        NpcDialog *dialog = NpcDialog::getActive();
        if (keyboard.isKeyActive(KeyboardConfig::KEY_OK)
            && (!dialog || !dialog->isTextInputFocused()))
        {
            // Close the Browser if opened
            if (helpWindow->isVisible())
                helpWindow->setVisible(false);
            // Close the config window, cancelling changes if opened
            else if (setupWindow->isVisible())
                setupWindow->action(gcn::ActionEvent(nullptr, "cancel"));
            else if (dialog)
                dialog->action(gcn::ActionEvent(nullptr, "ok"));
        }
        if (keyboard.isKeyActive(KeyboardConfig::KEY_TOGGLE_CHAT))
        {
            if (chatWindow->requestChatFocus())
                return true;
        }
        if (dialog)
        {
            if (keyboard.isKeyActive(KeyboardConfig::KEY_MOVE_UP))
                dialog->move(1);
            else if (keyboard.isKeyActive(KeyboardConfig::KEY_MOVE_DOWN))
                dialog->move(-1);
        }
    }

    if (!chatWindow->isInputFocused() || (event.keysym.mod & KMOD_ALT))
    {
        if (keyboard.isKeyActive(KeyboardConfig::KEY_PREV_CHAT_TAB))
        {
            chatWindow->prevTab();
            return true;
        }
        if (keyboard.isKeyActive(KeyboardConfig::KEY_NEXT_CHAT_TAB))
        {
            chatWindow->nextTab();
            return true;
        }
    }

    if (!chatWindow->isInputFocused())
    {
        bool wearOutfit = false;
        bool copyOutfit = false;

        if (keyboard.isKeyActive(KeyboardConfig::KEY_WEAR_OUTFIT))
            wearOutfit = true;

        if (keyboard.isKeyActive(KeyboardConfig::KEY_COPY_OUTFIT))
            copyOutfit = true;

        if (wearOutfit || copyOutfit)
        {
            int outfitNum = -1;
            switch (event.keysym.sym)
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
                outfitNum = event.keysym.sym - SDLK_1;
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
                if (wearOutfit)
                    outfitWindow->wearOutfit(outfitNum);
                else if (copyOutfit)
                    outfitWindow->copyOutfit(outfitNum);
                return true;
            }
        }
    }

    const int tKey = keyboard.getKeyIndex(event.keysym.sym);
    switch (tKey)
    {
    case KeyboardConfig::KEY_SCROLL_CHAT_UP:
        if (chatWindow->isVisible())
        {
            chatWindow->scroll(-DEFAULT_CHAT_WINDOW_SCROLL);
            return true;
        }
        break;
    case KeyboardConfig::KEY_SCROLL_CHAT_DOWN:
        if (chatWindow->isVisible())
        {
            chatWindow->scroll(DEFAULT_CHAT_WINDOW_SCROLL);
            return true;
        }
        break;
    case KeyboardConfig::KEY_WINDOW_HELP:
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
        return true;

    case KeyboardConfig::KEY_QUIT:
    {
        // Close possible stuck NPC dialogs.
        NpcDialog *npcDialog = NpcDialog::getActive();
        if (npcDialog && npcDialog->isWaitingForTheServer())
        {
            npcDialog->close();
            return true;
        }

        // Otherwise, show the quit confirmation dialog.
        quitDialog = new QuitDialog(&quitDialog);
        quitDialog->requestMoveToTop();
        return true;
    }
    default:
        break;
    }

    if (keyboard.isEnabled() && !chatWindow->isInputFocused() &&
        !NpcDialog::isAnyInputFocused() && !InventoryWindow::isAnyInputFocused())
    {
        // Do not activate shortcuts if tradewindow is visible
        if (!tradeWindow->isVisible() && !setupWindow->isVisible())
        {
            // Checks if any item shortcut is pressed.
            for (int i = KeyboardConfig::KEY_SHORTCUT_1;
                 i <= KeyboardConfig::KEY_SHORTCUT_12;
                 i++)
            {
                if (tKey == i)
                {
                    itemShortcut->useItem(
                        i - KeyboardConfig::KEY_SHORTCUT_1);
                    return true;
                }
            }
        }

        switch (tKey)
        {
        case KeyboardConfig::KEY_PICKUP:
            handleItemPickUp();
            return true;

        case KeyboardConfig::KEY_SIT:
            local_player->toggleSit();
            return true;

        case KeyboardConfig::KEY_HIDE_WINDOWS:
            // Hide certain windows
            statusWindow->setVisible(false);
            inventoryWindow->setVisible(false);
            skillDialog->setVisible(false);
            questsWindow->setVisible(false);
            setupWindow->setVisible(false);
            equipmentWindow->setVisible(false);
            helpWindow->setVisible(false);
            debugWindow->setVisible(false);
            socialWindow->setVisible(false);
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
        case KeyboardConfig::KEY_WINDOW_QUESTS:
            requestedWindow = questsWindow;
            break;
        case KeyboardConfig::KEY_WINDOW_MINIMAP:
            minimap->toggle();
            return true;
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
            saveScreenshot();
            return true;

        case KeyboardConfig::KEY_TRADE:
        {
            // Toggle accepting of incoming trade requests
            unsigned int deflt = player_relations.getDefault();
            if (deflt & PlayerPermissions::TRADE)
            {
                serverNotice(_("Ignoring incoming trade requests"));
                deflt &= ~PlayerPermissions::TRADE;
            }
            else
            {
                serverNotice(_("Accepting incoming trade requests"));
                deflt |= PlayerPermissions::TRADE;
            }

            player_relations.setDefault(deflt);

            return true;
        }

        case KeyboardConfig::KEY_TALK:
            if (Being *target = local_player->getTarget())
                if (target->canTalk())
                    target->talkTo();
            return true;
        }
    }

    if (requestedWindow)
    {
        requestedWindow->setVisible(!requestedWindow->isVisible());
        if (requestedWindow->isVisible())
            requestedWindow->requestMoveToTop();
        return true;
    }

    return false;
}

/**
 * Continuous input handling.
 */
void Game::handleInput()
{
    if (joystick)
        joystick->update();

    // If the user is configuring the keys then don't respond.
    if (!keyboard.isEnabled())
        return;
    if (!local_player->isAlive())
        return;
    if (PlayerInfo::isTalking())
        return;
    if (chatWindow->isInputFocused() || quitDialog || TextDialog::isActive())
        return;

    // Moving player around

    // Get the state of the keyboard keys
    keyboard.refreshActiveKeys();

    // Ignore input if either "ignore" key is pressed
    // Stops the character moving about if the user's window manager
    // uses "ignore+arrow key" to switch virtual desktops.
    if (keyboard.isKeyActive(KeyboardConfig::KEY_IGNORE_INPUT_1) ||
        keyboard.isKeyActive(KeyboardConfig::KEY_IGNORE_INPUT_2))
    {
        return;
    }

    unsigned char direction = 0;

    // Translate pressed keys to movement and direction
    if (keyboard.isKeyActive(KeyboardConfig::KEY_MOVE_UP) ||
        (joystick && joystick->isUp()))
    {
        direction |= Being::UP;
    }
    else if (keyboard.isKeyActive(KeyboardConfig::KEY_MOVE_DOWN) ||
             (joystick && joystick->isDown()))
    {
        direction |= Being::DOWN;
    }

    if (keyboard.isKeyActive(KeyboardConfig::KEY_MOVE_LEFT) ||
        (joystick && joystick->isLeft()))
    {
        direction |= Being::LEFT;
    }
    else if (keyboard.isKeyActive(KeyboardConfig::KEY_MOVE_RIGHT) ||
             (joystick && joystick->isRight()))
    {
        direction |= Being::RIGHT;
    }

    if (keyboard.isKeyActive(KeyboardConfig::KEY_EMOTE) && direction != 0)
    {
        if (local_player->getDirection() != direction)
        {
            local_player->setDirection(direction);
            Net::getPlayerHandler()->setDirection(direction);
        }
    }
    else
    {
        local_player->setWalkingDir(direction);
    }

    // Attacking monsters
    if (keyboard.isKeyActive(KeyboardConfig::KEY_ATTACK) ||
       (joystick && joystick->buttonPressed(0)))
    {
        if (local_player->getTarget())
            local_player->attack(local_player->getTarget(), true);
    }

    if (keyboard.isKeyActive(KeyboardConfig::KEY_TARGET_ATTACK))
    {
        Being *target = local_player->getTarget();

        bool newTarget = !keyboard.isKeyActive(KeyboardConfig::KEY_TARGET);
        // A set target has highest priority
        if (!target)
        {
            // Only auto target Monsters
            target = actorSpriteManager->findNearestLivingBeing(local_player,
                                                20, ActorSprite::MONSTER);
        }
        local_player->attack(target, newTarget);
    }

    // Target the nearest player/monster/npc
    if ((keyboard.isKeyActive(KeyboardConfig::KEY_TARGET_PLAYER) ||
        keyboard.isKeyActive(KeyboardConfig::KEY_TARGET_CLOSEST) ||
        keyboard.isKeyActive(KeyboardConfig::KEY_TARGET_NPC) ||
                (joystick && joystick->buttonPressed(3))) &&
            !keyboard.isKeyActive(KeyboardConfig::KEY_TARGET))
    {
        ActorSprite::Type currentTarget = ActorSprite::UNKNOWN;
        if (keyboard.isKeyActive(KeyboardConfig::KEY_TARGET_CLOSEST) ||
                (joystick && joystick->buttonPressed(3)))
            currentTarget = ActorSprite::MONSTER;
        else if (keyboard.isKeyActive(KeyboardConfig::KEY_TARGET_PLAYER))
            currentTarget = ActorSprite::PLAYER;
        else if (keyboard.isKeyActive(KeyboardConfig::KEY_TARGET_NPC))
            currentTarget = ActorSprite::NPC;

        Being *target = actorSpriteManager->findNearestLivingBeing(local_player,
                                                20, currentTarget);

        if (target && (target != local_player->getTarget() ||
                currentTarget != mLastTarget))
        {
            local_player->setTarget(target);
            mLastTarget = currentTarget;
        }
    }
    else
    {
        mLastTarget = ActorSprite::UNKNOWN; // Reset last target
    }

    // Stop attacking if the right key is pressed
    if (!keyboard.isKeyActive(KeyboardConfig::KEY_ATTACK)
        && keyboard.isKeyActive(KeyboardConfig::KEY_TARGET))
    {
        local_player->stopAttack();
    }

    if (joystick)
    {
        if (joystick->buttonPressed(1))
        {
            const int x = local_player->getTileX();
            const int y = local_player->getTileY();

            if (FloorItem *item = actorSpriteManager->findItem(x, y))
                local_player->pickUp(item);
        }
        else if (joystick->buttonPressed(2))
        {
            local_player->toggleSit();
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
    if (local_player)
        local_player->setMap(nullptr);

    particleEngine->clear();

    mMapName = mapPath;

    std::string fullMap = paths.getValue("maps", "maps/")
                          + mMapName + ".tmx";

    // Attempt to load the new map
    Map *newMap = MapReader::readMap(fullMap);

    if (!newMap)
    {
        Log::info("Error while loading %s", fullMap.c_str());
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
    std::string oldMusic = mCurrentMap ? mCurrentMap->getMusicFile() : std::string();
    std::string newMusic = newMap ? newMap->getMusicFile() : std::string();
    if (newMusic != oldMusic)
    {
        if (newMusic.empty())
            sound.fadeOutMusic();
        else
            sound.fadeOutAndPlayMusic(newMusic);
    }

    delete mCurrentMap;
    mCurrentMap = newMap;

    Event event(Event::MapLoaded);
    event.setString("mapPath", mapPath);
    event.trigger(Event::GameChannel);
}

int Game::getCurrentTileWidth() const
{
    if (mCurrentMap)
        return mCurrentMap->getTileWidth();

    return DEFAULT_TILE_LENGTH;
}

int Game::getCurrentTileHeight() const
{
    if (mCurrentMap)
        return mCurrentMap->getTileHeight();

    return DEFAULT_TILE_LENGTH;
}

void Game::videoResized(int width, int height)
{
    viewport->setSize(width, height);
    mWindowMenu->setPosition(width - 3 - mWindowMenu->getWidth(), 3);
}
