/*
 *  The Mana World
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "gui/viewport.h"

#include "beingmanager.h"
#include "configuration.h"
#include "flooritemmanager.h"
#include "game.h"
#include "graphics.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "map.h"
#include "monster.h"
#include "npc.h"
#include "textmanager.h"

#include "gui/gui.h"
#include "gui/ministatus.h"
#include "gui/popupmenu.h"

#include "net/net.h"

#include "resources/monsterinfo.h"
#include "resources/resourcemanager.h"

#include "utils/stringutils.h"

extern volatile int tick_time;

Viewport::Viewport():
    mMap(0),
    mMouseX(0),
    mMouseY(0),
    mPixelViewX(0.0f),
    mPixelViewY(0.0f),
    mTileViewX(0),
    mTileViewY(0),
    mShowDebugPath(false),
    mVisibleNames(false),
    mPlayerFollowMouse(false),
    mLocalWalkTime(-1)
{
    setOpaque(false);
    addMouseListener(this);

    mScrollLaziness = (int) config.getValue("ScrollLaziness", 16);
    mScrollRadius = (int) config.getValue("ScrollRadius", 0);
    mScrollCenterOffsetX = (int) config.getValue("ScrollCenterOffsetX", 0);
    mScrollCenterOffsetY = (int) config.getValue("ScrollCenterOffsetY", 0);
    mVisibleNames = config.getValue("visiblenames", 1);

    config.addListener("ScrollLaziness", this);
    config.addListener("ScrollRadius", this);
    config.addListener("visiblenames", this);

    mPopupMenu = new PopupMenu;
}

Viewport::~Viewport()
{
    delete mPopupMenu;

    config.removeListener("visiblenames", this);
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

    if (!mMap || !player_node)
    {
        gcnGraphics->setColor(gcn::Color(64, 64, 64));
        gcnGraphics->fillRectangle(
                gcn::Rectangle(0, 0, getWidth(), getHeight()));
        return;
    }

    Graphics *graphics = static_cast<Graphics*>(gcnGraphics);

    // Ensure the client doesn't freak out if a feature localplayer uses
    // is dependent on a map.
    player_node->setMapInitialized(true);

    // Avoid freaking out when tick_time overflows
    if (tick_time < lastTick)
    {
        lastTick = tick_time;
    }

    // Calculate viewpoint
    int midTileX = (graphics->getWidth() + mScrollCenterOffsetX) / 2;
    int midTileY = (graphics->getHeight() + mScrollCenterOffsetX) / 2;

    const Vector &playerPos = player_node->getPosition();
    const int player_x = (int) playerPos.x - midTileX;
    const int player_y = (int) playerPos.y - midTileY;

    if (mScrollLaziness < 1)
        mScrollLaziness = 1; // Avoids division by zero

    // Apply lazy scrolling
    while (lastTick < tick_time)
    {
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
    };

    // Don't move camera so that the end of the map is on screen
    const int viewXmax =
        mMap->getWidth() * mMap->getTileWidth() - graphics->getWidth();
    const int viewYmax =
        mMap->getHeight() * mMap->getTileHeight() - graphics->getHeight();
    if (mMap)
    {
        if (mPixelViewX < 0)
            mPixelViewX = 0;
        if (mPixelViewY < 0)
            mPixelViewY = 0;
        if (mPixelViewX > viewXmax)
            mPixelViewX = viewXmax;
        if (mPixelViewY > viewYmax)
            mPixelViewY = viewYmax;
    }

    mTileViewX = (int) (mPixelViewX + 16) / 32;
    mTileViewY = (int) (mPixelViewY + 16) / 32;

    // Draw tiles and sprites
    if (mMap)
    {
        mMap->draw(graphics, (int) mPixelViewX, (int) mPixelViewY);

        if (mShowDebugPath)
        {
            mMap->drawCollision(graphics,
                                (int) mPixelViewX,
                                (int) mPixelViewY,
                                mShowDebugPath);
            if (mShowDebugPath == Map::MAP_DEBUG)
                _drawDebugPath(graphics);
        }
    }

    if (player_node->getCheckNameSetting())
    {
        player_node->setCheckNameSetting(false);
        player_node->setName(player_node->getName());
    }

    // Draw text
    if (textManager)
    {
        textManager->draw(graphics, (int) mPixelViewX, (int) mPixelViewY);
    }

    // Draw player names, speech, and emotion sprite as needed
    const Beings &beings = beingManager->getAll();
    for (Beings::const_iterator i = beings.begin(), i_end = beings.end();
         i != i_end; ++i)
    {
        (*i)->drawSpeech((int) mPixelViewX, (int) mPixelViewY);
        (*i)->drawEmotion(graphics, (int) mPixelViewX, (int) mPixelViewY);
    }

    if (miniStatusWindow)
        miniStatusWindow->drawIcons(graphics);

    // Draw contained widgets
    WindowContainer::draw(gcnGraphics);
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
    // Get the current mouse position
    SDL_GetMouseState(&mMouseX, &mMouseY);

    const int mouseTileX = (mMouseX + (int) mPixelViewX) / 32;
    const int mouseTileY = (mMouseY + (int) mPixelViewY) / 32;
    const Vector &playerPos = player_node->getPosition();

    Path debugPath = mMap->findPath(
            (int) (playerPos.x - 16) / 32,
            (int) (playerPos.y - 32) / 32,
            mouseTileX, mouseTileY, 0xFF);

    _drawPath(graphics, debugPath);
}

void Viewport::_drawPath(Graphics *graphics, const Path &path)
{
    graphics->setColor(gcn::Color(255, 0, 0));
    for (Path::const_iterator i = path.begin(); i != path.end(); ++i)
    {
        int squareX = i->x * 32 - (int) mPixelViewX + 12;
        int squareY = i->y * 32 - (int) mPixelViewY + 12;

        graphics->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));
        graphics->drawText(
                toString(mMap->getMetaTile(i->x, i->y)->Gcost),
                squareX + 4, squareY + 12, gcn::Graphics::CENTER);
    }
}

void Viewport::mousePressed(gcn::MouseEvent &event)
{
    // Check if we are alive and kickin'
    if (!mMap || !player_node || !player_node->isAlive())
        return;

    // Check if we are busy
    if (current_npc)
        return;

    mPlayerFollowMouse = false;

    const int pixelX = event.getX() + (int) mPixelViewX;
    const int pixelY = event.getY() + (int) mPixelViewY;
    const int tileX = pixelX / mMap->getTileWidth();
    const int tileY = pixelY / mMap->getTileHeight();

    // Right click might open a popup
    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Being *being;
        FloorItem *floorItem;

        if ((being = beingManager->findBeingByPixel(pixelX, pixelY)) &&
             being != player_node)
        {
            mPopupMenu->showPopup(event.getX(), event.getY(), being);
            return;
        }
        else if ((floorItem = floorItemManager->findByCoordinates(tileX,
                                                                  tileY)))
        {
            mPopupMenu->showPopup(event.getX(), event.getY(), floorItem);
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
        FloorItem *item;
        Being *being;

        // Interact with some being
        if ((being = beingManager->findBeingByPixel(pixelX, pixelY)))
        {
            switch (being->getType())
            {
                // Talk to NPCs
                case Being::NPC:
                    dynamic_cast<NPC*>(being)->talk();
                    break;

                // Attack or walk to monsters or players
                case Being::MONSTER:
                case Being::PLAYER:
                    // Ignore it if its dead
                    if (!being->isAlive())
                        break;

                    if (player_node->withinAttackRange(being) ||
                        keyboard.isKeyActive(keyboard.KEY_ATTACK))
                    {
                        player_node->attack(being,
                            !keyboard.isKeyActive(keyboard.KEY_TARGET));
                    }
                    else
                    {
                        player_node->setGotoTarget(being);
                    }
                    break;
                default:
                    break;
             }
        // Picks up a item if we clicked on one
        }
        else if ((item = floorItemManager->findByCoordinates(tileX, tileY)))
        {
            player_node->pickUp(item);
        }
        else if (player_node->getCurrentAction() == Being::SIT)
        {
            return;
        }
        // Just walk around
        else
        {
            player_node->stopAttack();
            player_node->cancelFollow();
            mPlayerFollowMouse = true;

            // Make the player go to the mouse position
            _followMouse();
        }
    }
    else if (event.getButton() == gcn::MouseEvent::MIDDLE)
    {
        // Find the being nearest to the clicked position
        Being *target = beingManager->findNearestLivingBeing(
                pixelX, pixelY, 20, Being::MONSTER);

        if (target)
             player_node->setTarget(target);
    }
}

void Viewport::mouseDragged(gcn::MouseEvent &event)
{
    if (!mMap || !player_node)
        return;

    if (mPlayerFollowMouse && !event.isShiftPressed())
    {
        if (Net::getNetworkType() == ServerInfo::MANASERV)
        {
          if (get_elapsed_time(mLocalWalkTime) >= walkingMouseDelay)
          {
              mLocalWalkTime = tick_time;
              player_node->setDestination(event.getX() + (int) mPixelViewX,
                                          event.getY() + (int) mPixelViewY);
              player_node->pathSetByMouse();
          }
        }
        else
        {
          if (mLocalWalkTime != player_node->getWalkTime())
          {
              mLocalWalkTime = player_node->getWalkTime();
              int destX = event.getX() / 32 + mTileViewX;
              int destY = event.getY() / 32 + mTileViewY;
              player_node->setDestination(destX, destY);
          }
        }
    }
}

void Viewport::mouseReleased(gcn::MouseEvent &event)
{
    mPlayerFollowMouse = false;

    // Only useful for eAthena but doesn't hurt under ManaServ
    mLocalWalkTime = -1;
}

void Viewport::showPopup(int x, int y, Item *item, bool isInventory)
{
    mPopupMenu->showPopup(x, y, item, isInventory);
}

void Viewport::closePopupMenu()
{
    mPopupMenu->handleLink("cancel");
}

void Viewport::optionChanged(const std::string &name)
{
    mScrollLaziness = (int) config.getValue("ScrollLaziness", 32);
    mScrollRadius = (int) config.getValue("ScrollRadius", 32);

    if (name == "visiblenames")
        mVisibleNames = config.getValue("visiblenames", 1);
}

void Viewport::mouseMoved(gcn::MouseEvent &event)
{
    // Check if we are on the map
    if (!mMap || !player_node)
        return;

    const int tilex = (event.getX() + (int) mPixelViewX) / 32;
    const int tiley = (event.getY() + (int) mPixelViewY) / 32;

    mSelectedBeing = beingManager->findBeing(tilex, tiley);
}

void Viewport::toggleDebugPath()
{
    mShowDebugPath++;
    if (mShowDebugPath > Map::MAP_SPECIAL)
        mShowDebugPath = Map::MAP_NORMAL;
    if (mMap)
    {
        mMap->setDebugFlags(mShowDebugPath);
    }
}

