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
 *  $Id: monsterinfo.h 2650 2006-09-03 15:00:47Z b_lindeijer $
 */

#ifndef _TMW_MONSTERINFO_H_
#define _TMW_MONSTERINFO_H_

#include <map>
#include <string>
#include <vector>


enum SoundEvent
{
    EVENT_HIT,
    EVENT_MISS,
    EVENT_HURT,
    EVENT_DIE
};


class MonsterInfo
{
    public:
        MonsterInfo();

        ~MonsterInfo();

        void
        setName(std::string name) { mName = name; } ;

        void
        setSprite(std::string filename) { mSprite = filename; }

        void
        addSound (SoundEvent event, std::string filename);

        const std::string&
        getName () const { return mName; };

        const std::string&
        getSprite () const { return mSprite; };

        std::string
        getSound (SoundEvent event);

    private:

        std::string mName;
        std::string mSprite;

        std::map<SoundEvent, std::vector<std::string>* > mSounds;
};

#endif
