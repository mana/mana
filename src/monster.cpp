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

#include "monster.h"

#include "animatedsprite.h"
#include "client.h"
#include "localplayer.h"
#include "particle.h"
#include "sound.h"
#include "text.h"

#include "gui/userpalette.h"

#include "net/net.h"

#include "resources/monsterdb.h"
#include "resources/monsterinfo.h"

Monster::Monster(int id, int subtype, Map *map):
    Being(id, subtype, map),
    mAttackType(1)
{
    setSubtype(subtype);

    mNameColor = &userPalette->getColor(UserPalette::MONSTER);
    mTextColor = &userPalette->getColor(UserPalette::MONSTER);

    Being::setName(getInfo().getName());
}

void Monster::logic()
{
    if ((Net::getNetworkType() == ServerInfo::EATHENA) && (mAction != STAND))
    {
        mFrame = (int) ((get_elapsed_time(mWalkTime) * 4) / getWalkSpeed().x);

        if (mFrame >= 4 && mAction != DEAD)
            nextTile();
    }

    Being::logic();
}


void Monster::setAction(Action action, int attackType)
{
    SpriteAction currentAction = ACTION_INVALID;
    int rotation = 0;
    std::string particleEffect;

    switch (action)
    {
        case WALK:
            currentAction = ACTION_WALK;
            break;
        case DEAD:
            currentAction = ACTION_DEAD;
            sound.playSfx(getInfo().getSound(MONSTER_EVENT_DIE));
            break;
        case ATTACK:
            mAttackType = attackType;
            currentAction = getInfo().getAttackAction(attackType);
            for (SpriteIterator it = mSprites.begin(); it != mSprites.end(); it++)
                (*it)->reset();

            //attack particle effect
            particleEffect = getInfo().getAttackParticleEffect(attackType);
            if (!particleEffect.empty() && Particle::enabled)
            {
                switch (mSpriteDirection)
                {
                    case DIRECTION_DOWN: rotation = 0; break;
                    case DIRECTION_LEFT: rotation = 90; break;
                    case DIRECTION_UP: rotation = 180; break;
                    case DIRECTION_RIGHT: rotation = 270; break;
                    default: break;
                }
                Particle *p;
                p = particleEngine->addEffect(particleEffect, 0, 0, rotation);
                controlParticle(p);
            }
            break;
        case STAND:
           currentAction = ACTION_STAND;
           break;
        case HURT:
           // Not implemented yet
           break;
        case SIT:
           // Also not implemented yet
           break;
    }

    if (currentAction != ACTION_INVALID)
    {
        for (SpriteIterator it = mSprites.begin(); it != mSprites.end(); it++)
            if (*it)
                (*it)->play(currentAction);
        mAction = action;
    }
}

void Monster::setSubtype(Uint16 subtype)
{
    Being::setSubtype(subtype);

    const MonsterInfo &info = getInfo();

    // Setup Monster sprites
    const std::list<std::string> &sprites = info.getSprites();

    mSprites.clear();
    for (std::list<std::string>::const_iterator i = sprites.begin();
         i != sprites.end(); i++)
    {
        std::string file = "graphics/sprites/" + *i;
        mSprites.push_back(AnimatedSprite::load(file));
    }

    // Ensure that something is shown
    if (mSprites.size() == 0)
    {
        mSprites.push_back(AnimatedSprite::load("graphics/sprites/error.xml"));
    }

    if (Particle::enabled)
    {
        const std::list<std::string> &particleEffects = info.getParticleEffects();
        for (std::list<std::string>::const_iterator i = particleEffects.begin();
             i != particleEffects.end(); i++)
        {
            controlParticle(particleEngine->addEffect((*i), 0, 0));
        }
    }
}

void Monster::handleAttack(Being *victim, int damage, AttackType type)
{
    Being::handleAttack(victim, damage, type);

    const MonsterInfo &mi = getInfo();
    sound.playSfx(mi.getSound((damage > 0) ?
                MONSTER_EVENT_HIT : MONSTER_EVENT_MISS));

    fireMissile(victim, mi.getAttackMissileParticle(mAttackType));
}

void Monster::takeDamage(Being *attacker, int amount, AttackType type)
{
    if (amount > 0)
        sound.playSfx(getInfo().getSound(MONSTER_EVENT_HURT));

    Being::takeDamage(attacker, amount, type);
}

Being::TargetCursorSize Monster::getTargetCursorSize() const
{
    return getInfo().getTargetCursorSize();
}

const MonsterInfo &Monster::getInfo() const
{
    return MonsterDB::get(mSubType);
}

void Monster::updateCoords()
{
    if (mDispName)
    {
        mDispName->adviseXY(getPixelX(),
                        getPixelY() - getHeight() - mDispName->getHeight());
    }
}

void Monster::showName()
{
    Being::showName();

    updateCoords();
}
