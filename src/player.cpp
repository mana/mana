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

#include "resources/equipmentdb.h"

#include "utils/tostring.h"

#include "gui/gui.h"

Player::Player(Uint32 id, Uint16 job, Map *map):
    Being(id, job, map)
{
    setWeapon(0);
}

void
Player::logic()
{
    switch (mAction) {
        case WALK:
            mFrame = (get_elapsed_time(mWalkTime) * 6) / mWalkSpeed;
            if (mFrame >= 6) {
                nextStep();
            }
            break;
        case ATTACK:
            int frames = 4;
            if (getWeapon() == 2)
            {
                frames = 5;
            }
            mFrame = (get_elapsed_time(mWalkTime) * frames) / mAttackSpeed;
            if (mFrame >= frames) {
                nextStep();
            }
            break;
    }
    Being::logic();
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
    if (sex != mSex)
    {
        // Reload base sprite
        AnimatedSprite *newBaseSprite;
        if (sex == 0)
        {
            newBaseSprite = new AnimatedSprite(
                    "graphics/sprites/player_male_base.xml");
        }
        else
        {
            newBaseSprite = new AnimatedSprite(
                    "graphics/sprites/player_female_base.xml");
        }
        delete mSprites[BASE_SPRITE];
        mSprites[BASE_SPRITE] = newBaseSprite;

        // Reload equipment
        for (int i=1; i<VECTOREND_SPRITE ; i++)
        {
            if (i != HAIR_SPRITE && mEquipmentSpriteIDs.at(i) != 0)
            {
                AnimatedSprite *newEqSprite = new AnimatedSprite(
                        "graphics/sprites/" + EquipmentDB::get(
                            mEquipmentSpriteIDs.at(i))->getSprite(sex));
                delete mSprites[i];
                mSprites[i] = newEqSprite;
            }
        }
    }

    Being::setSex(sex);
}


void
Player::setWeapon(Uint16 weapon)
{
    if (weapon != mWeapon)
    {
        AnimatedSprite *newWeaponSprite = NULL;

        switch (weapon)
        {
            case 0:
                newWeaponSprite =
                    new AnimatedSprite("graphics/sprites/weapon-fist.xml");
                break;
            case 1:
                newWeaponSprite =
                    new AnimatedSprite("graphics/sprites/weapon-dagger.xml");
                break;
            case 2:
                newWeaponSprite =
                    new AnimatedSprite("graphics/sprites/weapon-bow.xml");
                break;
            case 3:
                newWeaponSprite =
                    new AnimatedSprite("graphics/sprites/weapon-scythe.xml");
                break;
        }

        delete mSprites[WEAPON_SPRITE];
        mSprites[WEAPON_SPRITE] = newWeaponSprite;
    }
    Being::setWeapon(weapon);
}


void
Player::setHairColor(Uint16 color)
{
    if (color != mHairColor && mHairStyle > 0)
    {
        AnimatedSprite *newHairSprite = new AnimatedSprite(
                "graphics/sprites/hairstyle" + toString(mHairStyle) + ".xml",
                color - 1);
        newHairSprite->setDirection(getSpriteDirection());

        delete mSprites[HAIR_SPRITE];
        mSprites[HAIR_SPRITE] = newHairSprite;

        setAction(mAction);
    }

    Being::setHairColor(color);
}

void
Player::setHairStyle(Uint16 style)
{
    if (style != mHairStyle && mHairColor > 0)
    {
        AnimatedSprite *newHairSprite = new AnimatedSprite(
                "graphics/sprites/hairstyle" + toString(style) + ".xml",
                mHairColor - 1);
        newHairSprite->setDirection(getSpriteDirection());

        delete mSprites[HAIR_SPRITE];
        mSprites[HAIR_SPRITE] = newHairSprite;

        setAction(mAction);
    }

    Being::setHairStyle(style);
}

void
Player::setVisibleEquipment(Uint8 slot, int id)
{
    // id = 0 means unequip
    if (id == 0)
    {
        delete mSprites[slot];
        mSprites[slot] = NULL;
    }
    else
    {
        AnimatedSprite *equipmentSprite;

        if (mSex == 0)
        {
            equipmentSprite = new AnimatedSprite(
                "graphics/sprites/" + EquipmentDB::get(id)->getSprite(0));
        }
        else {
            equipmentSprite = new AnimatedSprite(
                "graphics/sprites/" + EquipmentDB::get(id)->getSprite(1));
        }

        equipmentSprite->setDirection(getSpriteDirection());

        delete mSprites[slot];
        mSprites[slot] = equipmentSprite;

        setAction(mAction);
    }

    Being::setVisibleEquipment(slot, id);
}
