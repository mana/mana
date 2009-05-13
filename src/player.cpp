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
#include "configuration.h"
#include "game.h"
#ifdef TMWSERV_SUPPORT
#include "guild.h"
#endif
#include "localplayer.h"
#include "particle.h"
#include "player.h"
#include "text.h"

#include "gui/palette.h"

#include "resources/colordb.h"
#include "resources/itemdb.h"
#include "resources/iteminfo.h"

#include "utils/stringutils.h"

Player::Player(int id, int job, Map *map):
    Being(id, job, map),
    mName(0),
    mIsGM(false),
    mInParty(false)
{
    mShowName = config.getValue("visiblenames", 1);
    config.addListener("visiblenames", this);
}

Player::~Player()
{
    config.removeListener("visiblenames", this);
    delete mName;
}

void Player::setName(const std::string &name)
{
    if (!mName && mShowName)
    {
        mNameColor = &guiPalette->getColor(Palette::PLAYER);

        const gcn::Color *color;
        if (this == player_node)
        {
            color = &guiPalette->getColor(Palette::SELF);
        }
        else if (mIsGM)
        {
            mNameColor = &guiPalette->getColor(Palette::GM);
            color = &guiPalette->getColor(Palette::GM_NAME);
        }
        else if (mInParty)
        {
            color = &guiPalette->getColor(Palette::PARTY);
        }
        else
        {
            color = &guiPalette->getColor(Palette::PC);
        }

        mName = new FlashText(name,
                              getPixelX(),
                              getPixelY(),
                              gcn::Graphics::CENTER,
                              color);
    }

    Being::setName(name);
}

#ifdef EATHENA_SUPPORT
void Player::logic()
{
    switch (mAction)
    {
        case STAND:
        case SIT:
        case DEAD:
        case HURT:
           break;

        case WALK:
            mFrame = (get_elapsed_time(mWalkTime) * 6) / getWalkSpeed();
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
#endif

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

void Player::setGM(bool gm)
{
    mIsGM = gm;

    if (gm && mName)
        mName->setColor(&guiPalette->getColor(Palette::GM));
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

void Player::setSprite(int slot, int id, const std::string &color)
{
    // TODO: Find a better way
    if (getType() == NPC)
        return;

    // id = 0 means unequip
    if (id == 0)
    {
        delete mSprites[slot];
        mSprites[slot] = NULL;

#ifdef EATHENA_SUPPORT
        if (slot == WEAPON_SPRITE)
            mEquippedWeapon = NULL;
#endif
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
        mName->adviseXY(getPixelX(), getPixelY());
}

#ifdef TMWSERV_SUPPORT

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

short Player::getNumberOfGuilds()
{
    return mGuilds.size();
}

#endif

void Player::setInParty(bool inParty)
{
    mInParty = inParty;

    if (this != player_node && mName)
    {
        Palette::ColorType colorType = mInParty ? Palette::PARTY : Palette::PC;
        mName->setColor(&guiPalette->getColor(colorType));
    }
}

void Player::optionChanged(const std::string &value)
{
    if (value == "visiblenames" && getType() == Being::PLAYER && player_node != this)
    {
        mShowName = config.getValue("visiblenames", 1);
        if (!mShowName && mName)
        {
            delete mName;
            mName = NULL;
        }
        else if (mShowName && !mName && !(getName().empty()))
        {
            setName(getName());
        }
    }
}
