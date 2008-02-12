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

#include "monster.h"

#include "animatedsprite.h"
#include "game.h"
#include "sound.h"

#include "resources/monsterdb.h"

#include "utils/tostring.h"


Monster::Monster(Uint16 id, Uint16 job, Map *map):
    Being(id, job, map)
{
    mSprites[BASE_SPRITE] = AnimatedSprite::load(
            "graphics/sprites/" + getInfo().getSprite());
}

Being::Type
Monster::getType() const
{
    return MONSTER;
}

void
Monster::setAction(Action action)
{
    SpriteAction currentAction = ACTION_INVALID;

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
            break;
        case STAND:
            currentAction = ACTION_STAND;
            break;
        case HURT:
            // Not implemented yet
            break;
        default:
            break;
    }

    if (currentAction != ACTION_INVALID)
    {
        mSprites[BASE_SPRITE]->play(currentAction);
        mAction = action;
    }
}

void
Monster::handleAttack()
{
    Being::handleAttack();

    const MonsterInfo &mi = getInfo();

    // TODO: It's not possible to determine hit or miss here, so this stuff
    // probably needs to be moved somewhere else. We may lose synchronization
    // between attack animation and the sound, unless we adapt the protocol...
    sound.playSfx(mi.getSound(MONSTER_EVENT_HIT));
}

void
Monster::takeDamage(int amount)
{
    if (amount > 0) sound.playSfx(getInfo().getSound(MONSTER_EVENT_HURT));
    Being::takeDamage(amount);
}

Being::TargetCursorSize
Monster::getTargetCursorSize() const
{
    return getInfo().getTargetCursorSize();
}

const MonsterInfo&
Monster::getInfo() const
{
    return MonsterDB::get(mJob);
}
