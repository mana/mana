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

extern volatile int tick_time;
extern int get_elapsed_time(int start_time);

Viewport::Viewport():
    mMap(0),
    mViewX(0.0f),
    mViewY(0.0f),
    mShowDebugPath(false),
    mPlayerFollowMouse(false),
    mLocalWalkTime(-1)
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
    assert(size >= Being::TC_SMALL);
    assert(size < Being::NUM_TC);

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
    int midTileX = (graphics->getWidth() + mScrollCenterOffsetX) / 2;
    int midTileY = (graphics->getHeight() + mScrollCenterOffsetX) / 2;

    int player_x = player_node->mX - midTileX + player_node->getXOffset();
    int player_y = player_node->mY - midTileY + player_node->getYOffset();

    if (mScrollLaziness < 1)
        mScrollLaziness = 1; // Avoids division by zero

    // Apply lazy scrolling
    while (lastTick < tick_time)
    {
        if (player_x > mViewX + mScrollRadius)
        {
            mViewX += (player_x - mViewX - mScrollRadius) / mScrollLaziness;
        }
        if (player_x < mViewX - mScrollRadius)
        {
            mViewX += (player_x - mViewX + mScrollRadius) / mScrollLaziness;
        }
        if (player_y > mViewY + mScrollRadius)
        {
            mViewY += (player_y - mViewY - mScrollRadius) / mScrollLaziness;
        }
        if (player_y < mViewY - mScrollRadius)
        {
            mViewY += (player_y - mViewY + mScrollRadius) / mScrollLaziness;
        }
        lastTick++;
    }

    // Auto center when player is off screen
    if (        player_x - mViewX > graphics->getWidth() / 2
            ||  mViewX - player_x > graphics->getWidth() / 2
            ||  mViewY - player_y > graphics->getHeight() / 2
            ||  player_y - mViewY > graphics->getHeight() / 2
        )
    {
        mViewX = player_x;
        mViewY = player_y;
    };

    // Don't move camera so that the end of the map is on screen
    int viewXmax = mMap->getWidth() * 32 - graphics->getWidth();
    int viewYmax = mMap->getHeight() * 32 - graphics->getHeight();
    if (mMap)
    {
        if (mViewX < 0) {
            mViewX = 0;
        }
        if (mViewY < 0) {
            mViewY = 0;
        }
        if (mViewX > viewXmax) {
            mViewX = viewXmax;
        }
        if (mViewY > viewYmax) {
            mViewY = viewYmax;
        }
    }

    // Draw tiles and sprites
    if (mMap)
    {
        mMap->draw(graphics, (int) mViewX, (int) mViewY);
        drawTargetCursor(graphics); // TODO: Draw the cursor with the sprite
        drawTargetName(graphics);

        if (mShowDebugPath) {
            mMap->drawCollision(graphics, (int) mViewX, (int) mViewY);
            drawDebugPath(graphics);
        }
    }

    // Draw player names, speech, and emotion sprite as needed
    Beings &beings = beingManager->getAll();
    for (BeingIterator i = beings.begin(); i != beings.end(); i++)
    {
        (*i)->drawSpeech(graphics, -(int) mViewX, -(int) mViewY);
        (*i)->drawName(graphics, -(int) mViewX, -(int) mViewY);
        (*i)->drawEmotion(graphics, -(int) mViewX, -(int) mViewY);
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
            mWalkTime != player_node->mWalkTime && 
            get_elapsed_time(mLocalWalkTime) >= walkingMouseDelay)
    {
            mLocalWalkTime = tick_time;
            player_node->setDestination(mouseX + (int) mViewX,
                                        mouseY + (int) mViewY);
            mWalkTime = player_node->mWalkTime;
    }

    for (int i = Being::TC_SMALL; i < Being::NUM_TC; i++)
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
        int posX = target->getPixelX() + 16 -
                   targetCursor->getWidth() / 2 - (int) mViewX;
        int posY = target->getPixelY() + 16 -
                   targetCursor->getHeight() / 2 - (int) mViewY;

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
        int posX = target->getPixelX() + 16 - (int) mViewX;
        int posY = target->getPixelY() + 16 - target->getHeight() - (int) mViewY;

        graphics->drawText(mi.getName(), posX, posY, gcn::Graphics::CENTER);
    }
}

void
Viewport::drawDebugPath(Graphics *graphics)
{
    // Get the current mouse position
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    const int mouseTileX = (mouseX + (int) mViewX) / 32;
    const int mouseTileY = (mouseY + (int) mViewY) / 32;

    Path debugPath = mMap->findPath(
            player_node->mX / 32, player_node->mY / 32,
            mouseTileX, mouseTileY, 0xFF);

    graphics->setColor(gcn::Color(255, 0, 0));
    for (PathIterator i = debugPath.begin(); i != debugPath.end(); i++)
    {
        int squareX = i->x * 32 - (int) mViewX + 12;
        int squareY = i->y * 32 - (int) mViewY + 12;

        graphics->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));
        graphics->drawText(
                toString(mMap->getMetaTile(i->x, i->y)->Gcost),
                squareX + 4, squareY + 12, gcn::Graphics::CENTER);
    }
}

void
Viewport::mousePressed(gcn::MouseEvent &event)
{
    // Check if we are alive and kickin'
    if (!mMap || !player_node || player_node->mAction == Being::DEAD)
        return;

    mPlayerFollowMouse = false;

    const int tilex = (event.getX() + (int) mViewX) / 32;
    const int tiley = (event.getY() + (int) mViewY) / 32;

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
        FloorItem *item;

        // Pick up some item
        if ((item = floorItemManager->findByCoordinates(tilex, tiley)))
        {
                player_node->pickUp(item);
        }
        // Just walk around
        else
        {
            // FIXME: REALLY UGLY!
            Uint8 *keys = SDL_GetKeyState(NULL);
            if (!(keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT]) &&
                get_elapsed_time(mLocalWalkTime) >= walkingMouseDelay)
            {
                mLocalWalkTime = tick_time;
                player_node->setDestination(event.getX() + (int) mViewX,
                                            event.getY() + (int) mViewY);
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

    if (mPlayerFollowMouse && mWalkTime == player_node->mWalkTime
        && get_elapsed_time(mLocalWalkTime) >= walkingMouseDelay)
    {
        mLocalWalkTime = tick_time;
        player_node->setDestination(event.getX() + (int) mViewX,
                                    event.getY() + (int) mViewY);
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
