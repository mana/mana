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

#include <algorithm>

#include "monsterinfo.h"

#include "../utils/dtor.h"

MonsterInfo::MonsterInfo():
    mSprite("error.xml")
{

}

MonsterInfo::~MonsterInfo()
{
    // kill vectors in mSoundEffects
    for_each (mSounds.begin(), mSounds.end(),
              make_dtor(mSounds));
    mSounds.clear();
}


void
MonsterInfo::addSound(MonsterSoundEvent event, std::string filename)
{
    if (mSounds.find(event) == mSounds.end())
    {
        mSounds[event] = new std::vector<std::string>;
    }

    mSounds[event]->push_back("sfx/" + filename);
}


const std::string &
MonsterInfo::getSound(MonsterSoundEvent event) const
{
    static std::string nothing("");
    std::map<MonsterSoundEvent, std::vector<std::string>* >::const_iterator i;
    i = mSounds.find(event);
    if (i == mSounds.end())
    {
        return nothing;
    }
    else
    {
        return i->second->at(rand()%i->second->size());
    }
}

const std::string &
MonsterInfo::getAttackParticleEffect(int attackType) const
{
    static std::string nothing("");
    std::map<int, MonsterAttack*>::const_iterator i;
    i = mMonsterAttacks.find(attackType);
    if (i == mMonsterAttacks.end())
    {
        return nothing;
    }
    else
    {
        return (*i).second->particleEffect;
    }
}

SpriteAction
MonsterInfo::getAttackAction(int attackType) const
{
    std::map<int, MonsterAttack*>::const_iterator i;
    i = mMonsterAttacks.find(attackType);
    if (i == mMonsterAttacks.end())
    {
        return ACTION_ATTACK;
    }
    else
    {
        return (*i).second->action;
    }
}

void
MonsterInfo::addMonsterAttack (int id, const std::string &particleEffect, SpriteAction action)
{
    MonsterAttack* a = new MonsterAttack;
    a->particleEffect = particleEffect;
    a->action = action;
    mMonsterAttacks[id] = a;
}
