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
 */

#include "player.h"

#include "animatedsprite.h"
#include "game.h"
#include "graphics.h"
#include "log.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"

#include "utils/strprintf.h"

#include "gui/gui.h"
#include <iostream>

static const int NAME_X_OFFSET = 15;
static const int NAME_Y_OFFSET = 30;

Player::Player(int id, int job, Map *map):
    Being(id, job, map)
{
    mName = 0;
}

Player::~Player()
{
    if (mName)
    {
        delete mName;
    }
}

void Player::setName(const std::string &name)
{
    if (mName == 0)
    {
        mName = new FlashText(name, mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET,
                              gcn::Graphics::CENTER,
                              speechFont, gcn::Color(255, 255, 255));
        Being::setName(name);
    }
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
            if (    mEquippedWeapon
                &&  mEquippedWeapon->getAttackType() == ACTION_ATTACK_BOW)
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
Player::flash(int time)
{
    if (mName)
    {
        mName->flash(time);
    }
}

void Player::setGender(Gender gender)
{
    if (gender != mGender)
    {
        Being::setGender(gender);

        /* Human base sprite. When implementing different races remove this
         * line and set the base sprite when setting the race of the player
         * character.
         */
        setSprite(Being::BASE_SPRITE, -100);

        // Reload all subsprites
        for (int i = 1; i < VECTOREND_SPRITE; i++)
        {
            if (mSpriteIDs.at(i) != 0)
            {
                setSprite(i, mSpriteIDs.at(i), mSpriteColors.at(i));
            }
        }
    }
}

void Player::setHairStyle(int style, int color)
{
    style = style < 0 ? mHairStyle : style % getHairStylesNr();
    color = color < 0 ? mHairColor : color % getHairColorsNr();
    if (style == mHairStyle && color == mHairColor) return;

    Being::setHairStyle(style, color);

    setSprite(HAIR_SPRITE, style * -1, getHairColor(color));

    setAction(mAction);
}

void Player::setSprite(int slot, int id, std::string color)
{
    // id = 0 means unequip
    if (id == 0)
    {
        delete mSprites[slot];
        mSprites[slot] = NULL;
    }
    else
    {
        std::string filename = ItemDB::get(id).getSprite(mGender);
        AnimatedSprite *equipmentSprite = NULL;

        if (filename != "")
        {
            if (color!="") filename += "|" + color;
            equipmentSprite = AnimatedSprite::load(
                "graphics/sprites/" + filename);
        }

        if (equipmentSprite)
            equipmentSprite->setDirection(getSpriteDirection());

        delete mSprites[slot];
        mSprites[slot] = equipmentSprite;

        if (slot == WEAPON_SPRITE)
        {
            mEquippedWeapon = &ItemDB::get(id);
        }

        setAction(mAction);
    }

    Being::setSprite(slot, id, color);
}

void Player::updateCoords()
{
    if (mName)
    {
        mName->adviseXY(mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET);
    }
}


