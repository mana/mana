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
#include "guild.h"
#include "log.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"

#include "utils/strprintf.h"

#include "gui/gui.h"

Player::Player(int id, int job, Map *map):
    Being(id, job, map),
    mGender(GENDER_UNSPECIFIED),
    mHairStyle(0),
    mHairColor(0)
{
}

Player::~Player()
{
    if (mMap) mMap->freeTile(mX / 32, mY / 32, getBlockType());
}

Being::Type
Player::getType() const
{
    return PLAYER;
}

void
Player::drawName(Graphics *graphics, int offsetX, int offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    graphics->setFont(speechFont);
    graphics->setColor(gcn::Color(255, 255, 255));
    graphics->drawText(mName, px + 15, py + 30, gcn::Graphics::CENTER);
}

void Player::setGender(Gender gender)
{
    if (gender != mGender)
    {
        mGender = gender;

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
    style = style < 0 ? mHairStyle : style % NR_HAIR_STYLES;
    color = color < 0 ? mHairColor : color % NR_HAIR_COLORS;
    if (style == mHairStyle && color == mHairColor) return;

    mHairStyle = style;
    mHairColor = color;

    static char const *const colors[NR_HAIR_COLORS] =
    {
        "#8c4b41,da9041,ffffff", // light brown
        "#06372b,489e25,fdedcc", // green
        "#5f0b33,91191c,f9ad81", // red
        "#602486,934cc3,fdc689", // purple
        "#805e74,c6b09b,ffffff", // gray
        "#8c6625,dab425,ffffff", // yellow
        "#1d2d6d,1594a3,fdedcc", // blue
        "#831f2d,be4f2d,f8cc8b", // brown
        "#432482,584bbc,dae8e5", // light blue
        "#460850,611967,e7b4ae", // dark purple
    };

    setSprite(HAIR_SPRITE, style * -1, colors[color]);

    setAction(mAction);
}

void Player::setSprite(int slot, int id, const std::string &color)
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

        if (!filename.empty())
        {
            if (!color.empty())
                filename += "|" + color;
            equipmentSprite =
                AnimatedSprite::load("graphics/sprites/" + filename);
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

Guild* Player::addGuild(short guildId, bool inviteRights)
{
    Guild *guild = new Guild(guildId, inviteRights);
    mGuilds.insert(std::pair<int, Guild*>(guildId, guild));
    return guild;
}

void Player::removeGuild(int id)
{
    mGuilds.erase(id);
}

Guild* Player::getGuild(const std::string &guildName)
{
    std::map<int, Guild*>::iterator itr, itr_end = mGuilds.end();
    for (itr = mGuilds.begin(); itr != itr_end; ++itr)
    {
        Guild *guild = itr->second;
        if (guild->getName() == guildName)
        {
            return guild;
        }
    }

    return NULL;
}

Guild* Player::getGuild(int id)
{
    std::map<int, Guild*>::iterator itr;
    itr = mGuilds.find(id);
    if (itr != mGuilds.end())
    {
        return itr->second;
    }

    return NULL;
}

short Player::getNumberOfGuilds()
{
    return mGuilds.size();
}

void Player::setInParty(bool value)
{
    mInParty = value;
}
