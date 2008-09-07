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
 *  $Id: player.cpp 4237 2008-05-14 18:57:32Z b_lindeijer $
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
    mIsGM = false;
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
        if (mIsGM) {
            mName = new FlashText("(GM) " + name, mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET,
                                  gcn::Graphics::CENTER,
                                  gmNameFont, gcn::Color(255, 255, 255));
        } else {
            mName = new FlashText(name, mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET,
                                  gcn::Graphics::CENTER,
                                  speechFont, gcn::Color(255, 255, 255));
        }
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

void Player::setGender(int gender)
{
    // Players can only be male or female
    if (gender > 1)
    {
        logger->log("Warning: unsupported gender %i, assuming male.", gender);
        gender = 0;
    }


    if (gender != mGender)
    {
        Being::setGender(gender);

        setSprite(Being::BASE_SPRITE, -100); /* Human base sprite. When implementing
                                              * different races remove this line
                                              * and set the base sprite when setting
                                              * the race of the player character.
                                              */

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
    style = style < 0 ? mHairStyle : style % NR_HAIR_STYLES;
    color = color < 0 ? mHairColor : color % NR_HAIR_COLORS;
    if (style == mHairStyle && color == mHairColor) return;

    Being::setHairStyle(style, color);

    static char const *const colors[NR_HAIR_COLORS] =
    {
        "#8c4b41,da9041,ffffff", // light brown
        "#06372b,489e25,fdedcc", // green
        "#5f0b33,91191c,f9ad81", // dark red
        "#602486,934cc3,fdc689", // purple
        "#805e74,c6b09b,ffffff", // white
        "#8c6625,dab425,ffffff", // yellow
        "#1d2d6d,1594a3,fdedcc", // blue
        "#831f2d,be4f2d,f8cc8b", // brown
        "#432482,584bbc,dae8e5", // light blue
        "#460850,611967,e7b4ae", // dark purple
        "#8f3e21,bc522b,ff946c", // orange
        "#9c6b84,d272a3,ffcae5", // pink
        "#1d1d1d,414141,6f6f6f", // charcoal
        "#525252,99999b,cacaca", // silver
        "#841413,aa2829,ffaeb1", // light red
        "#a82513,ffa525,ffffff", // strawberry blond
    };

    setSprite(HAIR_SPRITE, style * -1, colors[color]);

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
