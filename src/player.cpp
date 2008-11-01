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
#include "guild.h"
#include "log.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"

#include "utils/strprintf.h"

#include "gui/gui.h"

Player::Player(int id, int job, Map *map):
    Being(id, job, map)
{
}

Player::~Player()
{
}

Being::Type
Player::getType() const
{
    return PLAYER;
}

void Player::drawName(Graphics *graphics, int offsetX, int offsetY)
{
    const Vector &pos = getPosition();
    const int px = (int) pos.x + offsetX;
    const int py = (int) pos.y + offsetY;

    graphics->setFont(speechFont);
    graphics->setColor(gcn::Color(255, 255, 255));
    graphics->drawText(mName, px, py, gcn::Graphics::CENTER);
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

Guild* Player::addGuild(short guildId, short rights)
{
    Guild *guild = new Guild(guildId, rights);
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


