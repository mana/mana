/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "player.h"

#include "animatedsprite.h"
#include "client.h"
#include "configuration.h"
#include "guild.h"
#include "localplayer.h"
#include "particle.h"
#include "party.h"
#include "text.h"

#include "gui/socialwindow.h"
#include "gui/theme.h"
#include "gui/userpalette.h"

#include "net/charhandler.h"
#include "net/net.h"

#include "resources/colordb.h"
#include "resources/itemdb.h"
#include "resources/iteminfo.h"

#include "utils/stringutils.h"

Player::Player(int id, int subtype, Map *map, bool isNPC):
    Being(id, subtype, map),
    mGender(GENDER_UNSPECIFIED),
    mParty(NULL),
    mIsGM(false)
{
    if (!isNPC)
    {
        for (int i = 0; i < Net::getCharHandler()->maxSprite(); i++)
        {
            mSprites.push_back(NULL);
            mSpriteIDs.push_back(0);
            mSpriteColors.push_back("");
        }

        setSubtype(subtype);
    }
    mShowName = config.getValue("visiblenames", 1);
    config.addListener("visiblenames", this);

    updateColors();
}

Player::~Player()
{
    config.removeListener("visiblenames", this);
}

void Player::logic()
{
    if (Net::getNetworkType() == ServerInfo::EATHENA)
    {
        switch (mAction)
        {
            case STAND:
            case SIT:
            case DEAD:
            case HURT:
               break;

            case WALK:
                mFrame = (int) ((get_elapsed_time(mWalkTime) * 6)
                         / getWalkSpeed().x);
                if (mFrame >= 6)
                    nextTile();
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

                if (!particleEffect.empty() && Particle::enabled && mFrame == 1)
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
                    nextTile();

                break;
        }
    }

    Being::logic();
}

void Player::setSubtype(Uint16 subtype)
{
    Being::setSubtype(subtype);

    int id = -100 - subtype;
    if (ItemDB::exists(id)) // Prevent showing errors when sprite doesn't exist
        setSprite(Net::getCharHandler()->baseSprite(), id);
    else
        setSprite(Net::getCharHandler()->baseSprite(), -100);
}

void Player::setGender(Gender gender)
{
    if (gender != mGender)
    {
        mGender = gender;

        // Reload all subsprites
        for (unsigned int i = 0; i < mSprites.size(); i++)
        {
            if (mSpriteIDs.at(i) != 0)
                setSprite(i, mSpriteIDs.at(i), mSpriteColors.at(i));
        }
    }
}

void Player::setGM(bool gm)
{
    mIsGM = gm;

    updateColors();
}

void Player::setSprite(int slot, int id, const std::string &color,
                       bool isWeapon)
{
    if (getType() == NPC)
        return;

    assert(slot < Net::getCharHandler()->maxSprite());

    // id = 0 means unequip
    if (id == 0)
    {
        delete mSprites[slot];
        mSprites[slot] = NULL;

        if (isWeapon)
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

        if (mSprites[slot])
            delete mSprites[slot];

        mSprites[slot] = equipmentSprite;

        if (isWeapon)
            mEquippedWeapon = &ItemDB::get(id);

        setAction(mAction);
    }

    mSpriteIDs[slot] = id;
    mSpriteColors[slot] = color;
}

void Player::setSpriteID(unsigned int slot, int id)
{
    setSprite(slot, id, mSpriteColors[slot]);
}

void Player::setSpriteColor(unsigned int slot, const std::string &color)
{
    setSprite(slot, mSpriteIDs[slot], color);
}

void Player::addGuild(Guild *guild)
{
    mGuilds[guild->getId()] = guild;
    guild->addMember(mId, mName);

    if (this == player_node && socialWindow)
    {
        socialWindow->addTab(guild);
    }
}

void Player::removeGuild(int id)
{
    if (this == player_node && socialWindow)
    {
        socialWindow->removeTab(mGuilds[id]);
    }

    mGuilds[id]->removeMember(mId);
    mGuilds.erase(id);
}

Guild *Player::getGuild(const std::string &guildName) const
{
    std::map<int, Guild*>::const_iterator itr, itr_end = mGuilds.end();
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

Guild *Player::getGuild(int id) const
{
    std::map<int, Guild*>::const_iterator itr;
    itr = mGuilds.find(id);
    if (itr != mGuilds.end())
    {
        return itr->second;
    }

    return NULL;
}

const std::map<int, Guild*> &Player::getGuilds() const
{
    return mGuilds;
}

void Player::clearGuilds()
{
    std::map<int, Guild*>::const_iterator itr, itr_end = mGuilds.end();
    for (itr = mGuilds.begin(); itr != itr_end; ++itr)
    {
        Guild *guild = itr->second;

        if (this == player_node && socialWindow)
            socialWindow->removeTab(guild);

        guild->removeMember(mId);
    }

    mGuilds.clear();
}

void Player::setParty(Party *party)
{
    if (party == mParty)
        return;

    Party *old = mParty;
    mParty = party;

    if (old)
    {
        old->removeMember(mId);
    }

    if (party)
    {
        party->addMember(mId, mName);
    }

    updateColors();

    if (this == player_node && socialWindow)
    {
        if (old)
            socialWindow->removeTab(old);

        if (party)
            socialWindow->addTab(party);
    }
}

void Player::optionChanged(const std::string &value)
{
    if (value == "visiblenames")
    {
        setShowName(config.getValue("visiblenames", 1));
    }
}

void Player::updateColors()
{
    mTextColor = &userPalette->getColor(Theme::PLAYER);

    if (mIsGM)
    {
        mTextColor = &userPalette->getColor(Theme::GM);
        mNameColor = &userPalette->getColor(UserPalette::GM);
    }
    else if (mParty && mParty == player_node->getParty())
    {
        mNameColor = &userPalette->getColor(UserPalette::PARTY);
    }
    else
    {
        mNameColor = &userPalette->getColor(UserPalette::PC);
    }

    if (mDispName)
    {
        mDispName->setColor(mNameColor);
    }
}
