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
#include "being.h"

#include <algorithm>
#include <sstream>

#include "game.h"
#include "graphics.h"
#include "log.h"
#include "map.h"
#include "monster.h"
#include "player.h"

#include "graphic/spriteset.h"

#include "gui/gui.h"

extern Spriteset *emotionset;

PATH_NODE::PATH_NODE(Uint16 iX, Uint16 iY):
    x(iX), y(iY)
{
}

Being::Being(Uint32 id, Uint16 job, Map *map):
    job(job),
    x(0), y(0), direction(SOUTH),
    action(0), mFrame(0),
    speech_color(0),
    walk_time(0),
    emotion(0), emotion_time(0),
    aspd(350),
    mId(id),
    mWeapon(0),
    mWalkSpeed(150),
    mMap(NULL),
    hairStyle(1), hairColor(1),
    speech_time(0),
    damage_time(0),
    showSpeech(false), showDamage(false)
{
    setMap(map);
}

Being::~Being()
{
    clearPath();
    setMap(NULL);
}

void Being::setDestination(Uint16 destX, Uint16 destY)
{
    if (mMap)
    {
        setPath(mMap->findPath(x, y, destX, destY));
    }
}

void Being::clearPath()
{
    mPath.clear();
}

void Being::setPath(std::list<PATH_NODE> path)
{
    mPath = path;

    if (action != WALK && action != DEAD)
    {
        nextStep();
        walk_time = tick_time;
    }
}

void Being::setHairColor(Uint16 color)
{
    hairColor = color;
    if (hairColor < 1 || hairColor > NR_HAIR_COLORS + 1)
    {
        hairColor = 1;
    }
}

void Being::setHairStyle(Uint16 style)
{
    hairStyle = style;
    if (hairStyle < 1 || hairStyle > NR_HAIR_STYLES)
    {
        hairStyle = 1;
    }
}

void
Being::setSpeech(const std::string &text, Uint32 time)
{
    speech = text;
    speech_time = tick_time;
    showSpeech = true;
}

void
Being::setDamage(Sint16 amount, Uint32 time)
{
    if (!amount) {
        damage = "miss";
    } else {
        std::stringstream damageString;
        damageString << amount;
        damage = damageString.str();
    }
    damage_time = tick_time;
    showDamage = true;
}

void
Being::setMap(Map *map)
{
    // Remove sprite from potential previous map
    if (mMap != NULL)
    {
        mMap->removeSprite(mSpriteIterator);
    }

    mMap = map;

    // Add sprite to potential new map
    if (mMap != NULL)
    {
        mSpriteIterator = mMap->addSprite(this);
    }
}

void
Being::nextStep()
{
    mFrame = 0;

    if (mPath.empty())
    {
        action = STAND;
        return;
    }

    PATH_NODE node = mPath.front();
    mPath.pop_front();

    if (node.x > x) {
        if (node.y > y)       direction = SE;
        else if (node.y < y)  direction = NE;
        else                  direction = EAST;
    }
    else if (node.x < x) {
        if (node.y > y)       direction = SW;
        else if (node.y < y)  direction = NW;
        else                  direction = WEST;
    }
    else {
        if (node.y > y)       direction = SOUTH;
        else if (node.y < y)  direction = NORTH;
    }

    x = node.x;
    y = node.y;
    action = WALK;
    walk_time += mWalkSpeed / 10;
}

void
Being::logic()
{
    // Determine whether speech should still be displayed
    if (get_elapsed_time(speech_time) > 5000)
    {
        showSpeech = false;
    }

    // Determine whether damange should still be displayed
    if (get_elapsed_time(damage_time) > 3000)
    {
        showDamage = false;
    }

    // Update pixel coordinates
    mPx = x * 32;
    mPy = y * 32;

    mPy += getYOffset();
    mPx += getXOffset();

    if (emotion != 0)
    {
        emotion_time--;
        if (emotion_time == 0) {
            emotion = 0;
        }
    }
}

void
Being::drawSpeech(Graphics *graphics, Sint32 offsetX, Sint32 offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    // Draw speech above this being
    if (showSpeech)
    {
        graphics->setFont(speechFont);
        graphics->drawText(speech, px + 18, py - 60, gcn::Graphics::CENTER);
    }

    // Draw damage above this being
    if (showDamage)
    {
        // Selecting the right color
        if (damage == "miss")
        {
            graphics->setFont(hitYellowFont);
        }
        else if (getType() == MONSTER)
        {
            graphics->setFont(hitBlueFont);
        }
        else
        {
            graphics->setFont(hitRedFont);
        }

        int textY = (getType() == MONSTER) ? 32 : 70;
        int ft = get_elapsed_time(damage_time) - 1500;
        float a = (ft > 0) ? 1.0 - ft / 1500.0 : 1.0;

        graphics->setColor(gcn::Color(255, 255, 255, (int)(255 * a)));
        graphics->drawText(damage,
                           px + 16,
                           py - textY - get_elapsed_time(damage_time) / 100,
                           gcn::Graphics::CENTER);

        // Reset alpha value
        graphics->setColor(gcn::Color(255, 255, 255));
    }
}

Being::Type Being::getType() const
{
    return UNKNOWN;
}

void Being::setWeaponById(Uint16 weapon)
{
    switch (weapon)
    {
    case 529: // iron arrows
    case 1199: // arrows
        break;

    case 1200: // bow
    case 530: // short bow
    case 545: // forest bow
        setWeapon(2);
        break;

    case 521: // sharp knife
    case 522: // dagger
    case 536: // short sword
    case 1201: // knife
        setWeapon(1);
        break;

    case 0: // unequip
        setWeapon(0);
        break;

    default:
        logger->log("unknown item equiped : %d", weapon);
    }
}

int
Being::getXOffset() const
{
    // Only beings walking to the left or the right have an x offset
    if (action != WALK || direction == NORTH || direction == SOUTH) {
        return 0;
    }

    int offset = (get_elapsed_time(walk_time) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0) {
        offset = 0;
    }

    // Going to the right? Invert the offset.
    if (direction == WEST || direction == NW || direction == SW) {
        offset = -offset;
    }

    return offset;
}

int
Being::getYOffset() const
{
    // Only beings walking up or down have an y offset
    if (action != WALK || direction == EAST || direction == WEST) {
        return 0;
    }

    int offset = (get_elapsed_time(walk_time) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0) {
        offset = 0;
    }

    if (direction == NORTH || direction == NW || direction == NE) {
        offset = -offset;
    }

    return offset;
}

void
Being::draw(Graphics *graphics, int offsetX, int offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    if (emotion)
    {
        graphics->drawImage(emotionset->spriteset[emotion - 1],
                px + 3, py - 60);
    }
}
