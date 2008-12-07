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

#include "animatedsprite.h"
#include "game.h"
#include "localplayer.h"
#include "monster.h"
#include "particle.h"
#include "sound.h"
#include "text.h"

#include "gui/gui.h"

#include "resources/monsterdb.h"

#include "utils/tostring.h"

static const int NAME_X_OFFSET = 16;
static const int NAME_Y_OFFSET = 16;

Monster::Monster(Uint32 id, Uint16 job, Map *map):
    Being(id, job, map),
    mText(0)
{
    const MonsterInfo&  info = MonsterDB::get(job - 1002);

    // Setup Monster sprites
    int c = BASE_SPRITE;
    const std::list<std::string> &sprites = info.getSprites();

    for (std::list<std::string>::const_iterator i = sprites.begin();
         i != sprites.end();
         i++)
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
        for (   std::list<std::string>::const_iterator i = particleEffects.begin();
                i != particleEffects.end(); i++
            )
        {
            controlParticle(particleEngine->addEffect((*i), 0, 0));
        }
    }
}

Monster::~Monster() 
{
    if (mText)
    {
        delete mText;
        player_node->setTarget(NULL);
    }
}

void Monster::logic()
{
    if (mAction != STAND)
    {
        mFrame = (get_elapsed_time(mWalkTime) * 4) / mWalkSpeed;

        if (mFrame >= 4 && mAction != DEAD)
        {
            nextStep();
        }
    }

    Being::logic();
}

Being::Type Monster::getType() const
{
    return MONSTER;
}

void Monster::setAction(Action action)
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
            currentAction = ACTION_ATTACK;
            mSprites[BASE_SPRITE]->reset();

            //attack particle effect
            particleEffect = getInfo().getAttackParticleEffect();
            if (particleEffect != "" && mParticleEffects)
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
                p = particleEngine->addEffect(
                                    particleEffect, 0, 0, rotation);
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
            {
                mSprites[i]->play(currentAction);
            }
        }
        mAction = action;
    }
}

void Monster::handleAttack(Being *victim, int damage)
{
    Being::handleAttack(victim, damage);

    const MonsterInfo &mi = getInfo();
    sound.playSfx(mi.getSound((damage > 0) ?
                MONSTER_EVENT_HIT : MONSTER_EVENT_MISS));
}

void Monster::takeDamage(int amount)
{
    if (amount > 0) sound.playSfx(getInfo().getSound(MONSTER_EVENT_HURT));
    Being::takeDamage(amount);
}

Being::TargetCursorSize Monster::getTargetCursorSize() const
{
    return getInfo().getTargetCursorSize();
}

const MonsterInfo& Monster::getInfo() const
{
    return MonsterDB::get(mJob - 1002);
}

void Monster::showName(bool show)
{
    if (mText)
    {
        delete mText;
    }
    if (show)
    {
        mText = new Text(getInfo().getName(), mPx + NAME_X_OFFSET,
                         mPy + NAME_Y_OFFSET - getHeight(),
                         gcn::Graphics::CENTER,
                         mobNameFont, gcn::Color(255, 32, 32));
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
        mText->adviseXY(mPx + NAME_X_OFFSET,
                        mPy + NAME_Y_OFFSET - getHeight());
    }
}
