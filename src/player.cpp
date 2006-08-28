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

#include "player.h"

#include "animatedsprite.h"
#include "game.h"
#include "graphics.h"
#include "log.h"

#include "utils/tostring.h"

#include "gui/gui.h"

Player::Player(Uint32 id, Uint16 job, Map *map):
    Being(id, job, map)
{
    /* Load the weapon sprite. When there are more different weapons this
     * should be moved to the setWeapon Method.
     */
    mSprites[WEAPON_SPRITE] =
        new AnimatedSprite("graphics/sprites/weapons.xml", 0);
}

Being::Type
Player::getType() const
{
    return PLAYER;
}

void
Player::drawName(Graphics *graphics, Sint32 offsetX, Sint32 offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    graphics->setFont(speechFont);
    graphics->setColor(gcn::Color(255, 255, 255));
    graphics->drawText(mName, px + 15, py + 30, gcn::Graphics::CENTER);
}

void
Player::setSex(Uint8 sex)
{
    // Players can only be male or female
    if (sex > 1)
    {
        logger->log("Warning: unsupported gender %i, assuming male.", sex);
        sex = 0;
    }

    if (sex != mSex)
    {
        delete mSprites[BASE_SPRITE];
        if (sex == 0)
        {
            mSprites[BASE_SPRITE] = new AnimatedSprite(
                    "graphics/sprites/player_male_base.xml", 0);
        }
        else
        {
            mSprites[BASE_SPRITE] = new AnimatedSprite(
                    "graphics/sprites/player_female_base.xml", 0);
        }

        Being::setSex(sex);
        resetAnimations();
    }
}

void
Player::setHairColor(Uint16 color)
{
    if (color != mHairColor)
    {
        delete mSprites[HAIR_SPRITE];
        AnimatedSprite *newHairSprite = new AnimatedSprite(
                "graphics/sprites/hairstyle" + toString(mHairStyle) + ".xml",
                color);
        newHairSprite->setDirection(getSpriteDirection());

        mSprites[HAIR_SPRITE] = newHairSprite;
        resetAnimations();

        setAction(mAction);
    }

    Being::setHairColor(color);
}

void
Player::setHairStyle(Uint16 style)
{
    if (style != mHairStyle)
    {
        delete mSprites[HAIR_SPRITE];
        AnimatedSprite *newHairSprite = new AnimatedSprite(
                "graphics/sprites/hairstyle" + toString(style) + ".xml",
                mHairColor);
        newHairSprite->setDirection(getSpriteDirection());

        mSprites[HAIR_SPRITE] = newHairSprite;
        resetAnimations();

        setAction(mAction);
    }

    Being::setHairStyle(style);
}

void
Player::setVisibleEquipment(Uint8 slot, Uint8 id)
{
    // Translate eAthena specific slot
    Uint8 position = 0;
    switch (slot) {
        case 3:
            position = BOTTOMCLOTHES_SPRITE;
            break;
        case 4:
            position = HAT_SPRITE;
            break;
        case 5:
            position = TOPCLOTHES_SPRITE;
            break;
    }

    delete mSprites[position];
    mSprites[position] = NULL;

    // id = 0 means unequip
    if (id) {
        char stringId[4];
        sprintf(stringId, "%03i", id);

        AnimatedSprite *equipmentSprite = new AnimatedSprite(
                "graphics/sprites/item" + toString(stringId) + ".xml", 0);
        equipmentSprite->setDirection(getSpriteDirection());

        mSprites[position] = equipmentSprite;
        resetAnimations();

        setAction(mAction);
    }

    Being::setVisibleEquipment(slot, id);
}

void
Player::resetAnimations()
{
    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
        {
            mSprites[i]->reset();
        }
    }
}
