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
#include "localplayer.h"
#include "monster.h"
#include "particle.h"
#include "sound.h"
#include "text.h"

#include "gui/palette.h"

#include "resources/monsterdb.h"
#include "resources/monsterinfo.h"

Monster::Monster(int id, int job, Map *map):
    Being(id, job, map),
    mText(0)
{
    const MonsterInfo& info = getInfo();

    // Setup Monster sprites
    int c = BASE_SPRITE;
    const std::list<std::string> &sprites = info.getSprites();

    for (std::list<std::string>::const_iterator i = sprites.begin();
         i != sprites.end(); i++)
    {
        if (c == VECTOREND_SPRITE) break;

        std::string file = "graphics/sprites/" + *i;
        mSprites[c] = AnimatedSprite::load(file);
        c++;
    }

    // Ensure that something is shown
    if (c == BASE_SPRITE)
    {
        mSprites[c] = AnimatedSprite::load("graphics/sprites/error.xml");
    }

    if (mParticleEffects)
    {
        const std::list<std::string> &particleEffects = info.getParticleEffects();
        for (std::list<std::string>::const_iterator i = particleEffects.begin();
             i != particleEffects.end(); i++)
        {
            controlParticle(particleEngine->addEffect((*i), 0, 0));
        }
    }

    mNameColor = &guiPalette->getColor(Palette::MONSTER);

    Being::setName(getInfo().getName());
}

Monster::~Monster()
{
    delete mText;
}

#ifdef EATHENA_SUPPORT
void Monster::logic()
{
    if (mAction != STAND)
    {
        mFrame = (get_elapsed_time(mWalkTime) * 4) / getWalkSpeed();

        if (mFrame >= 4 && mAction != DEAD)
            nextStep();
    }

    Being::logic();
}
#endif

Being::Type Monster::getType() const
{
    return MONSTER;
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
            currentAction = getInfo().getAttackAction(attackType);
            mSprites[BASE_SPRITE]->reset();

            //attack particle effect
            particleEffect = getInfo().getAttackParticleEffect(attackType);
            if (!particleEffect.empty() && mParticleEffects)
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
        for (int i = 0; i < VECTOREND_SPRITE; i++)
        {
            if (mSprites[i])
                mSprites[i]->play(currentAction);
        }
        mAction = action;
    }
}

#ifdef TMWSERV_SUPPORT

void Monster::handleAttack()
{
    Being::handleAttack();

    const MonsterInfo &mi = getInfo();

    // TODO: It's not possible to determine hit or miss here, so this stuff
    // probably needs to be moved somewhere else. We may lose synchronization
    // between attack animation and the sound, unless we adapt the protocol...
    sound.playSfx(mi.getSound(MONSTER_EVENT_HIT));
}

#else

void Monster::handleAttack(Being *victim, int damage, AttackType type)
{
    Being::handleAttack(victim, damage, type);

    const MonsterInfo &mi = getInfo();
    sound.playSfx(mi.getSound((damage > 0) ?
                MONSTER_EVENT_HIT : MONSTER_EVENT_MISS));
}

#endif

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
    return MonsterDB::get(mJob);
}

void Monster::setShowName(bool show)
{
    delete mText;

    if (show)
    {
        mText = new Text(getInfo().getName(),
                         getPixelX(),
                         getPixelY() - getHeight(),
                         gcn::Graphics::CENTER,
                         &guiPalette->getColor(Palette::MONSTER));
    }
    else
    {
        mText = 0;
    }
}

void Monster::updateCoords()
{
    if (mText)
    {
        mText->adviseXY(getPixelX(),
                        getPixelY() - getHeight() - mText->getHeight());
    }
}
