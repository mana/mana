/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef BEINGINFO_H
#define BEINGINFO_H

#include "actorsprite.h"

#include "resources/spritedef.h"

#include <list>
#include <map>
#include <string>
#include <vector>

struct Attack {
    std::string mAction;
    int mEffectId, mHitEffectId, mCriticalHitEffectId;
    std::string mMissileParticleFilename;

    Attack(std::string action, int effectId, int hitEffectId,
           int criticalHitEffectId, std::string missileParticleFilename)
    {
        mAction = action;
        mEffectId = effectId;
        mHitEffectId = hitEffectId;
        mCriticalHitEffectId = criticalHitEffectId;
        mMissileParticleFilename = missileParticleFilename;
    }
};

typedef std::map<int, Attack*> Attacks;

enum SoundEvent
{
    SOUND_EVENT_HIT,
    SOUND_EVENT_MISS,
    SOUND_EVENT_HURT,
    SOUND_EVENT_DIE
};

typedef std::map<SoundEvent, std::vector<std::string>* > SoundEvents;

/**
 * Holds information about a certain type of monster. This includes the name
 * of the monster, the sprite to display and the sounds the monster makes.
 *
 * @see MonsterDB
 * @see NPCDB
 */
class BeingInfo
{
    public:
        static BeingInfo *Unknown;

        BeingInfo();

        ~BeingInfo();

        void setName(const std::string &name) { mName = name; }

        const std::string &getName() const
        { return mName; }

        void setDisplay(SpriteDisplay display);

        const SpriteDisplay &getDisplay() const
        { return mDisplay; }

        void setTargetCursorSize(const std::string &size);

        void setTargetCursorSize(ActorSprite::TargetCursorSize targetSize)
        { mTargetCursorSize = targetSize; }

        ActorSprite::TargetCursorSize getTargetCursorSize() const
        { return mTargetCursorSize; }

        void addSound(SoundEvent event, const std::string &filename);

        const std::string &getSound(SoundEvent event) const;

        void addAttack(int id, std::string action, int effectId,
                       int hitEffectId, int criticalHitEffectId,
                       const std::string &missileParticleFilename);

        const Attack *getAttack(int id) const;

        void setWalkMask(unsigned char mask)
        { mWalkMask = mask; }

        /**
         * Gets the way the being is blocked by other objects
         */
        unsigned char getWalkMask() const
        { return mWalkMask; }

        void setBlockType(Map::BlockType blockType)
        { mBlockType = blockType; }

        Map::BlockType getBlockType() const
        { return mBlockType; }

    private:
        SpriteDisplay mDisplay;
        std::string mName;
        ActorSprite::TargetCursorSize mTargetCursorSize;
        SoundEvents mSounds;
        Attacks mAttacks;
        unsigned char mWalkMask;
        Map::BlockType mBlockType;
};

typedef std::map<int, BeingInfo*> BeingInfos;
typedef BeingInfos::iterator BeingInfoIterator;

#endif // BEINGINFO_H
