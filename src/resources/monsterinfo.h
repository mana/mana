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

#ifndef MONSTERINFO_H
#define MONSTERINFO_H

#include "being.h"

#include <list>
#include <map>
#include <string>
#include <vector>

enum MonsterSoundEvent
{
    MONSTER_EVENT_HIT,
    MONSTER_EVENT_MISS,
    MONSTER_EVENT_HURT,
    MONSTER_EVENT_DIE
};

struct MonsterAttack
{
    std::string particleEffect;
    SpriteAction action;
};

/**
 * Holds information about a certain type of monster. This includes the name
 * of the monster, the sprite to display and the sounds the monster makes.
 *
 * @see MonsterDB
 */
class MonsterInfo
{
    public:
        MonsterInfo();

        ~MonsterInfo();

        void setName(const std::string &name) { mName = name; }

        void addSprite(const std::string &filename)
        { mSprites.push_back(filename); }

        void setTargetCursorSize(Being::TargetCursorSize targetCursorSize)
        { mTargetCursorSize = targetCursorSize; }

        void addSound(MonsterSoundEvent event, const std::string &filename);

        void addParticleEffect(const std::string &filename);

        const std::string &getName() const
        { return mName; }

        const std::list<std::string>& getSprites() const
        { return mSprites; }

        Being::TargetCursorSize getTargetCursorSize() const
        { return mTargetCursorSize; }

        const std::string &getSound(MonsterSoundEvent event) const;

        void addMonsterAttack(int id,
                              const std::string &particleEffect,
                              SpriteAction action);

        const std::string &getAttackParticleEffect(int attackType) const;

        SpriteAction getAttackAction(int attackType) const;

        const std::list<std::string>& getParticleEffects() const
        { return mParticleEffects; }

    private:
        std::string mName;
        std::list<std::string> mSprites;
        Being::TargetCursorSize mTargetCursorSize;
        std::map<MonsterSoundEvent, std::vector<std::string>* > mSounds;
        std::map<int, MonsterAttack*> mMonsterAttacks;
        std::list<std::string> mParticleEffects;
};

#endif // MONSTERINFO_H
