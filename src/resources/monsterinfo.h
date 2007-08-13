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

#ifndef _TMW_MONSTERINFO_H_
#define _TMW_MONSTERINFO_H_

#include <map>
#include <string>
#include <vector>

#include "../being.h"


enum SoundEvent
{
    EVENT_HIT,
    EVENT_MISS,
    EVENT_HURT,
    EVENT_DIE
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
        /**
         * Constructor.
         */
        MonsterInfo();

        /**
         * Destructor.
         */
        ~MonsterInfo();

        void
        setName(std::string name) { mName = name; } ;

        void
        setSprite(std::string filename) { mSprite = filename; }

        void
        setTargetCursorSize(Being::TargetCursorSize targetCursorSize)
        { mTargetCursorSize = targetCursorSize; }

        void
        addSound(SoundEvent event, std::string filename);

        const std::string&
        getName () const { return mName; };

        const std::string&
        getSprite () const { return mSprite; };

        const Being::TargetCursorSize
        getTargetCursorSize() const { return mTargetCursorSize; }

        std::string
        getSound (SoundEvent event) const;

    private:
        std::string mName;
        std::string mSprite;
        Being::TargetCursorSize mTargetCursorSize;
        std::map<SoundEvent, std::vector<std::string>* > mSounds;
};

#endif
