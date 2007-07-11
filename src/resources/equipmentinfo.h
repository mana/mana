/*
 *  The Mana World
 *  Copyright 2006 The Mana World Development Team
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
 *  $Id:
 */

#ifndef _TMW_EQUIPMENTINFO_H_
#define _TMW_EQUIPMENTINFO_H_

#include <map>
#include <string>
#include <vector>

#include "spritedef.h"

enum EquipmentSoundEvent
{
    EQUIP_EVENT_STRIKE,
    EQUIP_EVENT_HIT
};

class EquipmentInfo
{
    public:
        EquipmentInfo():
            mSlot (0),
            mAttackType(ACTION_DEFAULT)
        {
        };

        void
        setSlot (int slot) { mSlot = slot; }

        const std::string&
        getSprite(int gender) {return animationFiles[gender]; }

        void
        setSprite(std::string animationFile, int gender) {animationFiles[gender] = animationFile; }

        void
        setAttackType(std::string attackType);

        const SpriteAction
        getAttackType() { return mAttackType; }

        void
        addSound(EquipmentSoundEvent event, std::string filename);

        std::string
        getSound(EquipmentSoundEvent event) const;

    private:
        int mSlot;   //not used at the moment but maybe useful on our own server
        SpriteAction mAttackType;
        std::map<int, std::string> animationFiles;
        std::map<EquipmentSoundEvent, std::vector<std::string>* > mSounds;
};

#endif
