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

#include "gui/viewport.h"

#include "actorspritemanager.h"
#include "client.h"
#include "configuration.h"
#include "graphics.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "map.h"
#include "playerinfo.h"
#include "textmanager.h"

#include "gui/gui.h"
#include "gui/ministatuswindow.h"
#include "gui/popupmenu.h"
#include "gui/beingpopup.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "resources/resourcemanager.h"

#include "utils/stringutils.h"

#include <cmath>

extern volatile int tick_time;

Viewport::Viewport():
    mMap(0),
    mMouseX(0),
    mMouseY(0),
    mPixelViewX(0.0f),
    mPixelViewY(0.0f),
    mDebugFlags(0),
    mPlayerFollowMouse(false),
    mLocalWalkTime(-1),
    mHoverBeing(0),
    mHoverItem(0)
{
    setOpaque(false);
    addMouseListener(this);

    mScrollLaziness = config.getIntValue("ScrollLaziness");
    mScrollRadius = config.getIntValue("ScrollRadius");
    mScrollCenterOffsetX = config.getIntValue("ScrollCenterOffsetX");
    mScrollCenterOffsetY = config.getIntValue("ScrollCenterOffsetY");

    mPopupMenu = new PopupMenu;
    mBeingPopup = new BeingPopup;

    setFocusable(true);

    listen(Event::ConfigChannel);
    listen(Event::ActorSpriteChannel);
}

Viewport::~Viewport()
{
    delete mPopupMenu;
    delete mBeingPopup;
}

void Viewport::setMap(Map *map)
{
    if (mMap && map)
    {
        map->setDebugFlags(mMap->getDebugFlags());
    }
    mMap = map;
}

extern MiniStatusWindow *miniStatusWindow;

void Viewport::draw(gcn::Graphics *gcnGraphics)
{
    static int lastTick = tick_time;

    // Check whether map was successfully loaded since
    // the rest of this function relies on it
    if (!mMap || !local_player)
    {
        // Render unicolor background to avoid
        // rendering issues
        gcnGraphics->setColor(gcn::Color(64, 64, 64));
        gcnGraphics->fillRectangle(
                gcn::Rectangle(0, 0, getWidth(), getHeight()));
        return;
    }

    Graphics *graphics = static_cast<Graphics*>(gcnGraphics);

    // Avoid freaking out when tick_time overflows
    if (tick_time < lastTick)
    {
        lastTick = tick_time;
    }

    // Calculate viewpoint
    int midTileX = (graphics->getWidth() + mScrollCenterOffsetX) / 2;
    int midTileY = (graphics->getHeight() + mScrollCenterOffsetX) / 2;

    const Vector &playerPos = local_player->getPosition();
    const int player_x = (int) playerPos.x - midTileX;
    const int player_y = (int) playerPos.y - midTileY;

    if (mScrollLaziness < 1)
        mScrollLaziness = 1; // Avoids division by zero

    while (lastTick < tick_time)
    {
        // Apply lazy scrolling
        if (player_x > mPixelViewX + mScrollRadius)
        {
            mPixelViewX += (player_x - mPixelViewX - mScrollRadius) /
                            mScrollLaziness;
        }
        if (player_x < mPixelViewX - mScrollRadius)
        {
            mPixelViewX += (player_x - mPixelViewX + mScrollRadius) /
                            mScrollLaziness;
        }
        if (player_y > mPixelViewY + mScrollRadius)
        {
            mPixelViewY += (player_y - mPixelViewY - mScrollRadius) /
                            mScrollLaziness;
        }
        if (player_y < mPixelViewY - mScrollRadius)
        {
            mPixelViewY += (player_y - mPixelViewY + mScrollRadius) /
                            mScrollLaziness;
        }

        // manage shake effect
        for (ShakeEffects::iterator i = mShakeEffects.begin();
             i != mShakeEffects.end();
             i++)
        {
            // apply the effect to viewport
            mPixelViewX += i->x *= -i->decay;
            mPixelViewY += i->y *= -i->decay;
            // check death conditions
            if (abs(i->x) + abs(i->y) < 1.0f ||
                (i->duration > 0 && --i->duration == 0))
            {
                i = mShakeEffects.erase(i);
            }
        }
        lastTick++;
    }

    // Auto center when player is off screen
    if (        player_x - mPixelViewX > graphics->getWidth() / 2
            ||  mPixelViewX - player_x > graphics->getWidth() / 2
            ||  mPixelViewY - player_y > graphics->getHeight() / 2
            ||  player_y - mPixelViewY > graphics->getHeight() / 2
        )
    {
        mPixelViewX = player_x;
        mPixelViewY = player_y;
    }

    // Don't move camera so that the end of the map is on screen
    const int mapWidthPixels = mMap->getWidth() * mMap->getTileWidth();
    const int mapHeightPixels = mMap->getHeight() * mMap->getTileHeight();
    const int viewXmax = mapWidthPixels - graphics->getWidth();
    const int viewYmax = mapHeightPixels - graphics->getHeight();
    if (mPixelViewX < 0)
        mPixelViewX = 0;
    if (mPixelViewY < 0)
        mPixelViewY = 0;
    if (mPixelViewX > viewXmax)
        mPixelViewX = viewXmax;
    if (mPixelViewY > viewYmax)
        mPixelViewY = viewYmax;

    // Center camera on map if the map is smaller than the screen
    if (mapWidthPixels < graphics->getWidth())
        mPixelViewX = (mapWidthPixels - graphics->getWidth()) / 2;
    if (mapHeightPixels < graphics->getHeight())
        mPixelViewY = (mapHeightPixels - graphics->getHeight()) / 2;

    // Draw black background if map is smaller than the screen
    if (        mapWidthPixels < graphics->getWidth()
            ||  mapHeightPixels < graphics->getHeight())
    {
        gcnGraphics->setColor(gcn::Color(0, 0, 0));
        gcnGraphics->fillRectangle(
                gcn::Rectangle(0, 0, getWidth(), getHeight()));

    }

    // Draw tiles and sprites
    mMap->draw(graphics, (int) mPixelViewX, (int) mPixelViewY);

    if (mDebugFlags)
    {
        if (mDebugFlags & (Map::DEBUG_GRID | Map::DEBUG_COLLISION_TILES))
        {
            mMap->drawCollision(graphics, (int) mPixelViewX,
                                (int) mPixelViewY, mDebugFlags);
        }

        _drawDebugPath(graphics);
    }

    if (local_player->getCheckNameSetting())
    {
        local_player->setCheckNameSetting(false);
        local_player->setName(local_player->getName());
    }

    // Draw text
    if (textManager)
    {
        textManager->draw(graphics, (int) mPixelViewX, (int) mPixelViewY);
    }

    // Draw player names, speech, and emotion sprite as needed
    const ActorSprites &actors = actorSpriteManager->getAll();
    for (ActorSpritesConstIterator it = actors.begin(), it_end = actors.end();
         it != it_end; it++)
    {
        if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        Being *b = static_cast<Being*>(*it);
        b->drawSpeech((int) mPixelViewX, (int) mPixelViewY);
    }

    if (mDebugFlags & Map::DEBUG_BEING_IDS)
    {
        graphics->setColor(gcn::Color(255, 0, 255, 255));
        ActorSpritesConstIterator it, it_end;
        const ActorSprites &actors = actorSpriteManager->getAll();
        for (it = actors.begin(), it_end = actors.end(); it != it_end; ++it)
        {
            Being *being = dynamic_cast<Being*>(*it);
            if (!being)
                continue;

            const Vector &beingPos = being->getPosition();
            std::string idString = toString(being->getId());
            graphics->drawText(idString,
                               beingPos.x - mPixelViewX,
                               beingPos.y - mPixelViewY,
                               gcn::Graphics::CENTER);
        }
    }

    // Draw contained widgets
    WindowContainer::draw(gcnGraphics);
}

void Viewport::shakeScreen(int intensity)
{
    float direction = rand()%628 / 100.0f; // random value between 0 and 2PI
    float x = std::sin(direction) * intensity;
    float y = std::cos(direction) * intensity;
    shakeScreen(x, y);
}

void Viewport::shakeScreen(float x, float y, float decay, unsigned duration)
{
    ShakeEffect effect;
    effect.x = x;
    effect.y = y;
    effect.decay = decay;
    effect.duration = duration;
    mShakeEffects.push_back(effect);
}

void Viewport::logic()
{
    WindowContainer::logic();

    // Make the player follow the mouse position
    // if the mouse is dragged elsewhere than in a window.
    _followMouse();
}

void Viewport::_followMouse()
{
    Uint8 button = SDL_GetMouseState(&mMouseX, &mMouseY);

    mMouseX /= graphics->getScale();
    mMouseY /= graphics->getScale();

    // If the left button is dragged
    if (mPlayerFollowMouse && button & SDL_BUTTON(1))
    {
        // We create a mouse event and send it to mouseDragged.
        Uint8 *keys = SDL_GetKeyState(NULL);
        gcn::MouseEvent mouseEvent(NULL,
                      (keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]),
                      false,
                      false,
                      false,
                      gcn::MouseEvent::DRAGGED,
                      gcn::MouseEvent::LEFT,
                      mMouseX,
                      mMouseY,
                      0);

        mouseDragged(mouseEvent);
    }
}

void Viewport::_drawDebugPath(Graphics *graphics)
{
    if (mDebugFlags & Map::DEBUG_MOUSE_PATH)
    {
        // Get the current mouse position
        SDL_GetMouseState(&mMouseX, &mMouseY);

        mMouseX /= graphics->getScale();
        mMouseY /= graphics->getScale();

        // Prepare the walkmask corresponding to the protocol
        unsigned char walkMask;
        switch (Net::getNetworkType())
        {
          case ServerInfo::TMWATHENA:
            walkMask = Map::BLOCKMASK_WALL | Map::BLOCKMASK_CHARACTER;
            break;
          case ServerInfo::MANASERV:
          default:
            walkMask = Map::BLOCKMASK_WALL;
            break;
        }

        static Path debugPath;
        static Vector lastMouseDestination = Vector(0.0f, 0.0f);
        Vector mouseDestination(mMouseX + (int) mPixelViewX,
                                mMouseY + (int) mPixelViewY);

        if (mouseDestination.x != lastMouseDestination.x
            || mouseDestination.y != lastMouseDestination.y)
        {
            const Vector &playerPos = local_player->getPosition();

            // Adapt the path finding to the precision requested
            if (Net::getPlayerHandler()->usePixelPrecision())
            {
                debugPath = mMap->findPixelPath((int) playerPos.x,
                                                (int) playerPos.y,
                                                mouseDestination.x,
                                                mouseDestination.y,
                                                local_player->getCollisionRadius(),
                                                walkMask);
            }
            else
            {
                debugPath = mMap->findTilePath((int) playerPos.x,
                                               (int) playerPos.y,
                                               mouseDestination.x,
                                               mouseDestination.y,
                                               walkMask);
            }

            lastMouseDestination = mouseDestination;
        }

        _drawPath(graphics, debugPath, gcn::Color(128, 0, 128, 150));
    }

    // Draw the path debug information for every beings.
    ActorSpritesConstIterator it, it_end;
    const ActorSprites &actors = actorSpriteManager->getAll();
    for (it = actors.begin(), it_end = actors.end(); it != it_end; it++)
    {
        Being *being = dynamic_cast<Being*>(*it);
        if (!being)
            continue;

        const Vector &beingPos = being->getPosition();
        graphics->setColor(gcn::Color(128, 128, 0, 150));

        if (mDebugFlags & Map::DEBUG_BEING_COLLISION_RADIUS)
        {
            const int radius = being->getCollisionRadius();
            graphics->fillRectangle(gcn::Rectangle(
                                        (int) beingPos.x
                                        - (int) mPixelViewX - radius,
                                        (int) beingPos.y - (int) mPixelViewY
                                        - radius,
                                        radius * 2, radius * 2));
        }

        if (mDebugFlags & Map::DEBUG_BEING_PATH)
            _drawPath(graphics, being->getPath(), gcn::Color(0, 0, 255, 150));

        if (mDebugFlags & Map::DEBUG_BEING_POSITION)
        {
            // Draw the absolute x, y position using a cross.
            graphics->setColor(gcn::Color(0, 0, 255, 255));
            graphics->drawLine((int) beingPos.x - (int) mPixelViewX - 4,
                               (int) beingPos.y - (int) mPixelViewY - 4,
                               (int) beingPos.x - (int) mPixelViewX + 4,
                               (int) beingPos.y - (int) mPixelViewY + 4);
            graphics->drawLine((int) beingPos.x - (int) mPixelViewX + 4,
                               (int) beingPos.y - (int) mPixelViewY - 4,
                               (int) beingPos.x - (int) mPixelViewX - 4,
                               (int) beingPos.y - (int) mPixelViewY + 4);
        }
    }
}

void Viewport::_drawPath(Graphics *graphics, const Path &path,
                         gcn::Color color)
{
    graphics->setColor(color);

    for (Path::const_iterator i = path.begin(); i != path.end(); ++i)
    {
        int squareX = i->x - (int) mPixelViewX;
        int squareY = i->y - (int) mPixelViewY;

        graphics->fillRectangle(gcn::Rectangle(squareX - 4, squareY - 4,
                                                8, 8));
        graphics->drawText(
                toString(mMap->getMetaTile(i->x / mMap->getTileWidth(),
                                           i->y / mMap->getTileHeight())->Gcost),
                squareX + 4, squareY + 12, gcn::Graphics::CENTER);
    }
}

void Viewport::mousePressed(gcn::MouseEvent &event)
{
    if (event.getSource() != this)
        return;

    // Check if we are alive and kickin'
    if (!mMap || !local_player || !local_player->isAlive())
        return;

    // Check if we are busy
    if (PlayerInfo::isTalking())
        return;

    mPlayerFollowMouse = false;
    mBeingPopup->setVisible(false);

    const int pixelX = event.getX() + (int) mPixelViewX;
    const int pixelY = event.getY() + (int) mPixelViewY;

    mHoverBeing = actorSpriteManager->findBeingByPixel(pixelX, pixelY);
    mHoverItem = actorSpriteManager->findItem(pixelX / mMap->getTileWidth(),
                                              pixelY / mMap->getTileHeight());

    updateCursorType();

    // Right click might open a popup
    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        if (mHoverBeing && mHoverBeing != local_player)
        {
            mPopupMenu->showPopup(event.getX(), event.getY(), mHoverBeing);
            return;
        }
        else if (mHoverItem)
        {
            mPopupMenu->showPopup(event.getX(), event.getY(), mHoverItem);
            return;
        }
    }

    // If a popup is active, just remove it
    if (mPopupMenu->isVisible())
    {
        mPopupMenu->setVisible(false);
        return;
    }

    // Left click can cause different actions
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        // Interact with some being
        if (mHoverBeing)
        {
            if (mHoverBeing->canTalk())
                mHoverBeing->talkTo();
            else
            {
                // Ignore it if its dead
                if (mHoverBeing->isAlive())
                {
                    if (local_player->withinRange(mHoverBeing,
                                                 local_player->getAttackRange())
                        || keyboard.isKeyActive(keyboard.KEY_ATTACK))
                    {
                        local_player->attack(mHoverBeing,
                            !keyboard.isKeyActive(keyboard.KEY_TARGET));
                    }
                    else
                    {
                        local_player->setGotoTarget(mHoverBeing);
                    }
                }
             }
        // Picks up a item if we clicked on one
        }
        else if (mHoverItem)
        {
            local_player->pickUp(mHoverItem);
        }
        else if (local_player->getCurrentAction() == Being::SIT)
        {
            return;
        }
        // Just walk around
        else
        {
            local_player->stopAttack();
            mPlayerFollowMouse = true;

            // Make the player go to the mouse position
            _followMouse();
        }
    }
    else if (event.getButton() == gcn::MouseEvent::MIDDLE)
    {
        // Find the being nearest to the clicked position
        Being *target = actorSpriteManager->findNearestLivingBeing(
                pixelX, pixelY, 20, ActorSprite::MONSTER);

        if (target)
             local_player->setTarget(target);
    }
}

void Viewport::mouseDragged(gcn::MouseEvent &event)
{
    if (!mMap || !local_player)
        return;

    if (mPlayerFollowMouse && !event.isShiftPressed())
    {
        if (get_elapsed_time(mLocalWalkTime) >= walkingMouseDelay)
        {
            mLocalWalkTime = tick_time;
            local_player->setDestination(event.getX() + (int) mPixelViewX,
                                        event.getY() + (int) mPixelViewY);
            local_player->pathSetByMouse();
        }
    }
}

void Viewport::mouseReleased(gcn::MouseEvent &event)
{
    mPlayerFollowMouse = false;
}

void Viewport::showPopup(Window *parent, int x, int y, Item *item,
                         bool isInventory, bool canDrop)
{
    mPopupMenu->showPopup(parent, x, y, item, isInventory, canDrop);
}

void Viewport::closePopupMenu()
{
    mPopupMenu->handleLink("cancel");
}

void Viewport::mouseMoved(gcn::MouseEvent &event)
{
    // Check if we are on the map
    if (!mMap || !local_player)
        return;

    const int x = (event.getX() + (int) mPixelViewX);
    const int y = (event.getY() + (int) mPixelViewY);

    mHoverBeing = actorSpriteManager->findBeingByPixel(x, y);
    mBeingPopup->show(getMouseX(), getMouseY(), mHoverBeing);

    mHoverItem = actorSpriteManager->findItem(x / mMap->getTileWidth(),
                                              y / mMap->getTileHeight());

    updateCursorType();
}

void Viewport::updateCursorType()
{
    if (mHoverBeing)
    {
        switch (mHoverBeing->getType())
        {
            // NPCs
            case ActorSprite::NPC:
                gui->setCursorType(Gui::CURSOR_TALK);
                break;

            // Monsters
            case ActorSprite::MONSTER:
                gui->setCursorType(Gui::CURSOR_FIGHT);
                break;
            default:
                gui->setCursorType(Gui::CURSOR_POINTER);
                break;
         }
    // Item mouseover
    }
    else if (mHoverItem)
    {
        gui->setCursorType(Gui::CURSOR_PICKUP);
    }
    else
    {
        gui->setCursorType(Gui::CURSOR_POINTER);
    }
}

void Viewport::setShowDebugPath(int debugFlags)
{
    mDebugFlags = debugFlags;
    if (mMap)
        mMap->setDebugFlags(debugFlags);
}

void Viewport::hideBeingPopup()
{
    mBeingPopup->setVisible(false);
}

void Viewport::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::ActorSpriteChannel
            && event.getType() == Event::Destroyed)
    {
        ActorSprite *actor = event.getActor("source");

        if (mHoverBeing == actor)
            mHoverBeing = 0;

        if (mHoverItem == actor)
            mHoverItem = 0;
    }
    else if (channel == Event::ConfigChannel &&
             event.getType() == Event::ConfigOptionChanged)
    {
        const std::string option = event.getString("option");
        if (option == "ScrollLaziness" || option == "ScrollRadius")
        {
            mScrollLaziness = config.getIntValue("ScrollLaziness");
            mScrollRadius = config.getIntValue("ScrollRadius");
        }
    }
}
