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
#include "gui/popupmenu.h"
#include "gui/beingpopup.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "utils/stringutils.h"

#include <algorithm>
#include <cmath>

Viewport::Viewport()
{
    setOpaque(false);
    addMouseListener(this);

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

void Viewport::draw(gcn::Graphics *gcnGraphics)
{
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

    auto *graphics = static_cast<Graphics*>(gcnGraphics);

    // Calculate viewpoint
    int midTileX = (graphics->getWidth() + config.scrollCenterOffsetX) / 2;
    int midTileY = (graphics->getHeight() + config.scrollCenterOffsetY) / 2;

    const Vector &playerPos = local_player->getPosition();
    const int player_x = (int) playerPos.x - midTileX;
    const int player_y = (int) playerPos.y - midTileY;

    const float ticks = Time::deltaTimeMs() / static_cast<float>(MILLISECONDS_IN_A_TICK);
    float scrollFraction = 1.0f;

    if (config.scrollLaziness > 1)
    {
        // settings.ScrollLaziness defines the fraction of the desired camera movement
        // that is applied every 10ms. To make this work independently of the
        // frame duration, we calculate the actual scroll fraction based on the
        // time delta.
        scrollFraction = 1.0f - std::pow(1.0f - 1.0f / config.scrollLaziness, ticks);
    }

    // Apply lazy scrolling
    if (player_x > mPixelViewX + config.scrollRadius)
    {
        mPixelViewX += (player_x - mPixelViewX - config.scrollRadius) *
                        scrollFraction;
    }
    if (player_x < mPixelViewX - config.scrollRadius)
    {
        mPixelViewX += (player_x - mPixelViewX + config.scrollRadius) *
                        scrollFraction;
    }
    if (player_y > mPixelViewY + config.scrollRadius)
    {
        mPixelViewY += (player_y - mPixelViewY - config.scrollRadius) *
                        scrollFraction;
    }
    if (player_y < mPixelViewY - config.scrollRadius)
    {
        mPixelViewY += (player_y - mPixelViewY + config.scrollRadius) *
                        scrollFraction;
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
    };

    // Don't move camera so that the end of the map is on screen
    // Center camera on map if the map is smaller than the screen
    const int mapWidthPixels = mMap->getWidth() * mMap->getTileWidth();
    const int mapHeightPixels = mMap->getHeight() * mMap->getTileHeight();
    const int viewXmax = mapWidthPixels - graphics->getWidth();
    const int viewYmax = mapHeightPixels - graphics->getHeight();

    if (viewXmax > 0)
        mPixelViewX = std::clamp<float>(mPixelViewX, 0, viewXmax);
    else
        mPixelViewX = viewXmax / 2;

    if (viewYmax > 0)
        mPixelViewY = std::clamp<float>(mPixelViewY, 0, viewYmax);
    else
        mPixelViewY = viewYmax / 2;

    // Draw black background if map is smaller than the screen
    if (        mapWidthPixels < graphics->getWidth()
            ||  mapHeightPixels < graphics->getHeight())
    {
        gcnGraphics->setColor(gcn::Color(0, 0, 0));
        gcnGraphics->fillRectangle(
                gcn::Rectangle(0, 0, getWidth(), getHeight()));

    }

    int scrollX = static_cast<int>(mPixelViewX);
    int scrollY = static_cast<int>(mPixelViewY);

    // manage shake effect
    for (auto i = mShakeEffects.begin(); i != mShakeEffects.end(); )
    {
        ShakeEffect &effect = *i;
        effect.age += ticks;

        // The decay defines the reduction in amplitude per 10ms. Here
        // we calculate the strength left over based on the age in ticks.
        const float strength = std::pow(effect.decay, effect.age);
        const float phase = std::sin(effect.age);

        // apply the effect to viewport
        scrollX += strength * phase * effect.x;
        scrollY += strength * phase * effect.y;

        // check death conditions
        if (strength < 0.01f || (effect.timer.isSet() && effect.timer.passed()))
            i = mShakeEffects.erase(i);
        else
            ++i;
    }

    // Draw tiles and sprites
    mMap->draw(graphics, scrollX, scrollY);

    if (mDebugFlags)
    {
        if (mDebugFlags & (Map::DEBUG_GRID | Map::DEBUG_COLLISION_TILES))
            mMap->drawCollision(graphics, scrollX, scrollY, mDebugFlags);

        _drawDebugPath(graphics);
    }

    // Draw text
    if (textManager)
        textManager->draw(graphics, scrollX, scrollY);

    // Draw player names, speech, and emotion sprite as needed
    for (auto actor : actorSpriteManager->getAll())
    {
        if (actor->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        auto *being = static_cast<Being*>(actor);
        being->drawSpeech(scrollX, scrollY);
    }

    if (mDebugFlags & Map::DEBUG_BEING_IDS)
    {
        graphics->setColor(gcn::Color(255, 0, 255, 255));
        for (auto actor : actorSpriteManager->getAll())
        {
            auto *being = dynamic_cast<Being*>(actor);
            if (!being)
                continue;

            const Vector &beingPos = being->getPosition();
            std::string idString = toString(being->getId());
            graphics->drawText(idString,
                               beingPos.x - scrollX,
                               beingPos.y - scrollY,
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
    ShakeEffect &effect = mShakeEffects.emplace_back();
    effect.x = x;
    effect.y = y;
    effect.decay = decay;

    if (duration > 0)
        effect.timer.set(duration * MILLISECONDS_IN_A_TICK);
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
    const Uint8 button = SDL_GetMouseState(&mMouseX, &mMouseY);
    float logicalX;
    float logicalY;
    graphics->windowToLogical(mMouseX, mMouseY, logicalX, logicalY);
    mMouseX = static_cast<int>(logicalX);
    mMouseY = static_cast<int>(logicalY);

    // If the left button is dragged
    if (mPlayerFollowMouse && button & SDL_BUTTON_LMASK)
    {
        // We create a mouse event and send it to mouseDragged.
        const Uint8 *keys = SDL_GetKeyboardState(nullptr);
        gcn::MouseEvent mouseEvent(nullptr,
                      (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT]),
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
        // Prepare the walkmask corresponding to the protocol
        unsigned char walkMask;
        switch (Net::getNetworkType())
        {
        case ServerType::TmwAthena:
            walkMask = Map::BLOCKMASK_WALL | Map::BLOCKMASK_CHARACTER;
            break;
        case ServerType::ManaServ:
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
    for (auto actor : actorSpriteManager->getAll())
    {
        auto *being = dynamic_cast<Being*>(actor);
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

    for (auto pos : path)
    {
        int squareX = pos.x - (int) mPixelViewX;
        int squareY = pos.y - (int) mPixelViewY;

        graphics->fillRectangle(gcn::Rectangle(squareX - 4, squareY - 4,
                                                8, 8));
        graphics->drawText(
                toString(mMap->getMetaTile(pos.x / mMap->getTileWidth(),
                                           pos.y / mMap->getTileHeight())->Gcost),
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

        if (mHoverItem)
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
                        || keyboard.isKeyActive(KeyboardConfig::KEY_ATTACK))
                    {
                        local_player->attack(mHoverBeing,
                            !keyboard.isKeyActive(KeyboardConfig::KEY_TARGET));
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
        if (mLocalWalkTimer.passed())
        {
            mLocalWalkTimer.set(walkingMouseDelay);
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
            case ActorSprite::NPC:
            case ActorSprite::MONSTER:
                gui->setCursorType(mHoverBeing->getHoverCursor());
                break;
            default:
                gui->setCursorType(Cursor::Pointer);
                break;
         }
    // Item mouseover
    }
    else if (mHoverItem)
    {
        gui->setCursorType(Cursor::PickUp);
    }
    else
    {
        gui->setCursorType(Cursor::Pointer);
    }
}

void Viewport::setDebugFlags(int debugFlags)
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
            mHoverBeing = nullptr;

        if (mHoverItem == actor)
            mHoverItem = nullptr;
    }
}
