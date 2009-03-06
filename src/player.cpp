/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "animatedsprite.h"
#include "game.h"
#include "particle.h"
#include "player.h"
#include "text.h"

#include "resources/colordb.h"
#include "resources/itemdb.h"

#include "utils/strprintf.h"

static const int NAME_X_OFFSET = 15;
static const int NAME_Y_OFFSET = 30;

Player::Player(int id, int job, Map *map):
    Being(id, job, map)
{
    mName = NULL;
}

Player::~Player()
{
    delete mName;
}

void Player::setName(const std::string &name)
{
    if (mName == NULL)
    {
        if (mIsGM)
        {
            mNameColor = 0x009000;
            mName = new FlashText("(GM) " + name, mPx + NAME_X_OFFSET, mPy +
                                  NAME_Y_OFFSET, gcn::Graphics::CENTER,
                                  gcn::Color(0, 255, 0));
        }
        else
        {
            mNameColor = 0x202020;
            mName = new FlashText(name, mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET,
                                  gcn::Graphics::CENTER,
                                  gcn::Color(255, 255, 255));
        }
        Being::setName(name);
    }
}

void Player::logic()
{
    switch (mAction)
    {
        case STAND:
           break;

        case SIT:
           break;

        case DEAD:
           break;

        case HURT:
           break;

        case WALK:
            mFrame = (get_elapsed_time(mWalkTime) * 6) / mWalkSpeed;

            if (mFrame >= 6)
                nextStep();

            break;

        case ATTACK:
            int rotation = 0;
            std::string particleEffect = "";
            int frames = 4;

            if (mEquippedWeapon &&
                mEquippedWeapon->getAttackType() == ACTION_ATTACK_BOW)
            {
                frames = 5;
            }

            mFrame = (get_elapsed_time(mWalkTime) * frames) / mAttackSpeed;

            //attack particle effect
            if (mEquippedWeapon)
                particleEffect = mEquippedWeapon->getParticleEffect();

            if (!particleEffect.empty() && mParticleEffects && mFrame == 1)
            {
                switch (mDirection)
                {
                    case DOWN: rotation = 0; break;
                    case LEFT: rotation = 90; break;
                    case UP: rotation = 180; break;
                    case RIGHT: rotation = 270; break;
                    default: break;
                }
                Particle *p;
                p = particleEngine->addEffect("graphics/particles/" +
                                              particleEffect, 0, 0, rotation);
                controlParticle(p);
            }

            if (mFrame >= frames)
                nextStep();

            break;
    }

    Being::logic();
}

Being::Type Player::getType() const
{
    return PLAYER;
}

void Player::flash(int time)
{
    if (mName)
        mName->flash(time);
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
                setSprite(i, mSpriteIDs.at(i), mSpriteColors.at(i));
        }
    }
}

void Player::setHairStyle(int style, int color)
{
    style = style < 0 ? mHairStyle : style % mNumberOfHairstyles;
    color = color < 0 ? mHairColor : color % ColorDB::size();
    if (style == mHairStyle && color == mHairColor) return;

    Being::setHairStyle(style, color);

    setSprite(HAIR_SPRITE, style * -1, ColorDB::get(color));

    setAction(mAction);
}

void Player::setSprite(int slot, int id, std::string color)
{
    // id = 0 means unequip
    if (id == 0)
    {
        delete mSprites[slot];
        mSprites[slot] = NULL;

        if (slot == WEAPON_SPRITE)
            mEquippedWeapon = NULL;
    }
    else
    {
        std::string filename = ItemDB::get(id).getSprite(mGender);
        AnimatedSprite *equipmentSprite = NULL;

        if (!filename.empty())
        {
            if (!color.empty())
                filename += "|" + color;

            equipmentSprite = AnimatedSprite::load("graphics/sprites/" +
                                                   filename);
        }

        if (equipmentSprite)
            equipmentSprite->setDirection(getSpriteDirection());

        delete mSprites[slot];
        mSprites[slot] = equipmentSprite;

        if (slot == WEAPON_SPRITE)
            mEquippedWeapon = &ItemDB::get(id);

        setAction(mAction);
    }

    Being::setSprite(slot, id, color);
}

void Player::updateCoords()
{
    if (mName)
        mName->adviseXY(mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET);
}

