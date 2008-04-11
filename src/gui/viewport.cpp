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

#include "viewport.h"

#include <guichan/sdl/sdlinput.hpp>

#include "gui.h"
#include "popupmenu.h"

#include "../simpleanimation.h"
#include "../beingmanager.h"
#include "../configuration.h"
#include "../flooritemmanager.h"
#include "../graphics.h"
#include "../localplayer.h"
#include "../map.h"
#include "../monster.h"
#include "../npc.h"

#include "../resources/animation.h"
#include "../resources/monsterinfo.h"
#include "../resources/resourcemanager.h"
#include "../resources/image.h"
#include "../resources/imageset.h"

#include "../utils/tostring.h"

#include <cassert>

Viewport::Viewport():
    mMap(0),
    mPixelViewX(0.0f),
    mPixelViewY(0.0f),
    mTileViewX(0),
    mTileViewY(0),
    mShowDebugPath(false),
    mPlayerFollowMouse(false),
    mWalkTime(0)
{
    setOpaque(false);
    addMouseListener(this);

    mScrollLaziness = (int) config.getValue("ScrollLaziness", 32);
    mScrollRadius = (int) config.getValue("ScrollRadius", 32);
    mScrollCenterOffsetX = (int) config.getValue("ScrollCenterOffsetX", 0);
    mScrollCenterOffsetY = (int) config.getValue("ScrollCenterOffsetY", 0);

    config.addListener("ScrollLaziness", this);
    config.addListener("ScrollRadius", this);

    mPopupMenu = new PopupMenu();

    // Load target cursors
    loadTargetCursor("graphics/gui/target-cursor-blue-s.png", 44, 35,
                     false, Being::TC_SMALL);
    loadTargetCursor("graphics/gui/target-cursor-red-s.png", 44, 35,
                     true, Being::TC_SMALL);
    loadTargetCursor("graphics/gui/target-cursor-blue-m.png", 62, 44,
                     false, Being::TC_MEDIUM);
    loadTargetCursor("graphics/gui/target-cursor-red-m.png", 62, 44,
                     true, Being::TC_MEDIUM);
    loadTargetCursor("graphics/gui/target-cursor-blue-l.png", 82, 60,
                     false, Being::TC_LARGE);
    loadTargetCursor("graphics/gui/target-cursor-red-l.png", 82, 60,
                     true, Being::TC_LARGE);
}

void
Viewport::loadTargetCursor(std::string filename, int width, int height,
                           bool outRange, Being::TargetCursorSize size)
{
    assert(size > -1);
    assert(size < 3);

    ImageSet* currentImageSet;
    SimpleAnimation* currentCursor;

    ResourceManager *resman = ResourceManager::getInstance();

    currentImageSet = resman->getImageSet(filename, width, height);
    Animation *anim = new Animation();
    for (unsigned int i = 0; i < currentImageSet->size(); ++i)
    {
        anim->addFrame(currentImageSet->get(i), 75, 0, 0);
    }
    currentCursor = new SimpleAnimation(anim);

    if (outRange)
    {
        mOutRangeImages[size] = currentImageSet;
        mTargetCursorOutRange[size] = currentCursor;
    }
    else {
        mInRangeImages[size] = currentImageSet;
        mTargetCursorInRange[size] = currentCursor;
    }
}

Viewport::~Viewport()
{
    delete mPopupMenu;

    for (int i = Being::TC_SMALL; i < Being::NUM_TC; i++)
    {
        delete mTargetCursorInRange[i];
        delete mTargetCursorOutRange[i];
        mInRangeImages[i]->decRef();
        mOutRangeImages[i]->decRef();
    }
}

void
Viewport::setMap(Map *map)
{
    mMap = map;
}

void
Viewport::draw(gcn::Graphics *gcnGraphics)
{
    static int lastTick = tick_time;

    if (!mMap || !player_node)
        return;

    Graphics *graphics = static_cast<Graphics*>(gcnGraphics);

    // Avoid freaking out when tick_time overflows
    if (tick_time < lastTick)
    {
        lastTick = tick_time;
    }

    // Calculate viewpoint
    int midTileX = (graphics->getWidth() + mScrollCenterOffsetX) / 32 / 2;
    int midTileY = (graphics->getHeight() + mScrollCenterOffsetY) / 32 / 2;

    int player_x = (player_node->mX - midTileX) * 32 +
                    player_node->getXOffset();
    int player_y = (player_node->mY - midTileY) * 32 +
                    player_node->getYOffset();

    if (mScrollLaziness < 1)
        mScrollLaziness = 1; // Avoids division by zero

    // Apply lazy scrolling
    while (lastTick < tick_time)
    {
        if (player_x > mPixelViewX + mScrollRadius)
        {
            mPixelViewX += (player_x - mPixelViewX - mScrollRadius) / mScrollLaziness;
        }
        if (player_x < mPixelViewX - mScrollRadius)
        {
            mPixelViewX += (player_x - mPixelViewX + mScrollRadius) / mScrollLaziness;
        }
        if (player_y > mPixelViewY + mScrollRadius)
        {
            mPixelViewY += (player_y - mPixelViewY - mScrollRadius) / mScrollLaziness;
        }
        if (player_y < mPixelViewY - mScrollRadius)
        {
            mPixelViewY += (player_y - mPixelViewY + mScrollRadius) / mScrollLaziness;
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
    int viewXmax = (mMap->getWidth() * 32) - graphics->getWidth();
    int viewYmax = (mMap->getHeight() * 32) - graphics->getHeight();
    if (mMap)
    {
        if (mPixelViewX < 0) {
            mPixelViewX = 0;
        }
        if (mPixelViewY < 0) {
            mPixelViewY = 0;
        }
        if (mPixelViewX > viewXmax) {
            mPixelViewX = viewXmax;
        }
        if (mPixelViewY > viewYmax) {
            mPixelViewY = viewYmax;
        }
    }

    mTileViewX = (int) (mPixelViewX + 16) / 32;
    mTileViewY = (int) (mPixelViewY + 16) / 32;

    // Draw tiles and sprites
    if (mMap)
    {
        mMap->draw(graphics, (int) mPixelViewX, (int) mPixelViewY, 0);
        drawTargetCursor(graphics);
        mMap->draw(graphics, (int) mPixelViewX, (int) mPixelViewY, 1);
        mMap->draw(graphics, (int) mPixelViewX, (int) mPixelViewY, 2);
        mMap->drawOverlay(graphics, mPixelViewX, mPixelViewY,
                (int) config.getValue("OverlayDetail", 2));
        drawTargetName(graphics);
    }

    // Find a path from the player to the mouse, and draw it. This is for debug
    // purposes.
    if (mShowDebugPath && mMap)
    {
        // Get the current mouse position
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        int mouseTileX = mouseX / 32 + mTileViewX;
        int mouseTileY = mouseY / 32 + mTileViewY;

        Path debugPath = mMap->findPath(
                player_node->mX, player_node->mY,
                mouseTileX, mouseTileY);

        graphics->setColor(gcn::Color(255, 0, 0));
        for (PathIterator i = debugPath.begin(); i != debugPath.end(); i++)
        {
            int squareX = i->x * 32 - (int) mPixelViewX + 12;
            int squareY = i->y * 32 - (int) mPixelViewY + 12;

            graphics->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));
            graphics->drawText(
                    toString(mMap->getMetaTile(i->x, i->y)->Gcost),
                    squareX + 4, squareY + 12, gcn::Graphics::CENTER);
        }
    }

    // Draw player nickname, speech, and emotion sprite as needed
    Beings &beings = beingManager->getAll();
    for (BeingIterator i = beings.begin(); i != beings.end(); i++)
    {
        (*i)->drawSpeech(graphics, -(int) mPixelViewX, -(int) mPixelViewY);
        (*i)->drawName(graphics, -(int) mPixelViewX, -(int) mPixelViewY);
        (*i)->drawEmotion(graphics, -(int) mPixelViewX, -(int) mPixelViewY);
    }

    // Draw contained widgets
    WindowContainer::draw(gcnGraphics);
}

void
Viewport::logic()
{
    WindowContainer::logic();

    if (!mMap || !player_node)
        return;

    int mouseX, mouseY;
    Uint8 button = SDL_GetMouseState(&mouseX, &mouseY);

    if (mPlayerFollowMouse && button & SDL_BUTTON(1) &&
            mWalkTime != player_node->mWalkTime)
    {
        player_node->setDestination(mouseX / 32 + mTileViewX,
                                    mouseY / 32 + mTileViewY);
        mWalkTime = player_node->mWalkTime;
    }

    for (int i = 0; i < 3; i++)
    {
        mTargetCursorInRange[i]->update(10);
        mTargetCursorOutRange[i]->update(10);
    }
}

void
Viewport::drawTargetCursor(Graphics *graphics)
{
    // Draw target marker if needed
    Being *target = player_node->getTarget();
    if (target)
    {
        // Calculate target circle position

        // Find whether target is in range
        int rangeX = abs(target->mX - player_node->mX);
        int rangeY = abs(target->mY - player_node->mY);
        int attackRange = player_node->getAttackRange();

        // Get the correct target cursors graphic
        Being::TargetCursorSize cursorSize = target->getTargetCursorSize();
        Image* targetCursor;
        if (rangeX > attackRange || rangeY > attackRange)
        {
            targetCursor = mTargetCursorOutRange[cursorSize]->getCurrentImage();
        }
        else {
            targetCursor = mTargetCursorInRange[cursorSize]->getCurrentImage();
        }

        // Draw the target cursor at the correct position
        int posX = target->getPixelX() + 16 - targetCursor->getWidth() / 2 - (int) mPixelViewX;
        int posY = target->getPixelY() + 16 - targetCursor->getHeight() / 2 - (int) mPixelViewY;

        graphics->drawImage(targetCursor, posX, posY);
    }
}

void
Viewport::drawTargetName(Graphics *graphics)
{
    // Draw target marker if needed
    Being *target = player_node->getTarget();
    if (target && target->getType() == Being::MONSTER)
    {
        graphics->setFont(speechFont);
        graphics->setColor(gcn::Color(255, 32, 32));

        const MonsterInfo &mi = static_cast<Monster*>(target)->getInfo();
        int posX = target->getPixelX() + 16 - (int)mPixelViewX;
        int posY = target->getPixelY() + 16 - target->getHeight() - (int)mPixelViewY;

        graphics->drawText(mi.getName(), posX, posY, gcn::Graphics::CENTER);
    }
}

void
Viewport::mousePressed(gcn::MouseEvent &event)
{
    // Check if we are alive and kickin'
    if (!mMap || !player_node || player_node->mAction == Being::DEAD)
        return;

    // Check if we are busy
    if (current_npc)
        return;

    mPlayerFollowMouse = false;

    int tilex = event.getX() / 32 + mTileViewX;
    int tiley = event.getY() / 32 + mTileViewY;

    // Right click might open a popup
    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Being *being;
        FloorItem *floorItem;

        if ((being = beingManager->findBeing(tilex, tiley)) &&
            being != player_node)
        {
            mPopupMenu->showPopup(event.getX(), event.getY(), being);
            return;
        }
        else if((floorItem = floorItemManager->findByCoordinates(tilex, tiley)))
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
        Being *being;
        FloorItem *item;

        // Interact with some being
//      if ((being = beingManager->findBeing(tilex, tiley))
        int x = event.getX() + mPixelViewX;
        int y = event.getY() + mPixelViewY;
        if ((being = beingManager->findBeingByPixel(x, y)))
        {
            switch (being->getType())
            {
                case Being::NPC:
                    dynamic_cast<NPC*>(being)->talk();
                    break;

                case Being::MONSTER:
                case Being::PLAYER:
                    if (being->mAction == Being::DEAD)
                        break;
                    if (being == player_node)
                        break;

                    if (player_node->withinAttackRange(being))
                    {
                        player_node->attack(being, true);
                    }
                    else
                    {
                        Uint8 *keys = SDL_GetKeyState(NULL);
                        if (!(keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]))
                        {
                            player_node->stopAttack();
                            player_node->setGotoTarget(being);
                        }
                    }
                    break;

                default:
                    break;
            }
        }
        // Pick up some item
        else if ((item = floorItemManager->findByCoordinates(tilex, tiley)))
        {
                player_node->pickUp(item);
        }
        // Just walk around
        else
        {
            // XXX XXX XXX REALLY UGLY!
            Uint8 *keys = SDL_GetKeyState(NULL);
            if (!(keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]))
            {
                player_node->setDestination(tilex, tiley);
                player_node->stopAttack();
            }
            mPlayerFollowMouse = true;
        }
    }
    else if (event.getButton() == gcn::MouseEvent::MIDDLE)
    {
        // Find the being nearest to the clicked position
        Being *target = beingManager->findNearestLivingBeing(
                tilex, tiley,
                20, Being::MONSTER);

        if (target)
        {
            player_node->setTarget(target);
        }
    }
}

void
Viewport::mouseDragged(gcn::MouseEvent &event)
{
    if (!mMap || !player_node)
        return;

    if (mPlayerFollowMouse && mWalkTime == player_node->mWalkTime)
    {
        int destX = event.getX() / 32 + mTileViewX;
        int destY = event.getY() / 32 + mTileViewY;
        player_node->setDestination(destX, destY);
    }
}

void
Viewport::mouseReleased(gcn::MouseEvent &event)
{
    mPlayerFollowMouse = false;
}

void
Viewport::showPopup(int x, int y, Item *item)
{
    mPopupMenu->showPopup(x, y, item);
}

void
Viewport::optionChanged(const std::string &name)
{
    mScrollLaziness = (int) config.getValue("ScrollLaziness", 32);
    mScrollRadius = (int) config.getValue("ScrollRadius", 32);
}
