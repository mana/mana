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

#include "resources/iteminfo.h"

#include "resources/itemdb.h"

const std::string &ItemInfo::getSprite(Gender gender) const
{
    if (mView)
    {
        // Forward the request to the item defining how to view this item
        return ItemDB::get(mView).getSprite(gender);
    }
    else
    {
        static const std::string empty = "";
        std::map<int, std::string>::const_iterator i =
            mAnimationFiles.find(gender);

        return (i != mAnimationFiles.end()) ? i->second : empty;
    }
}

void ItemInfo::setWeaponType(int type)
{
    // See server item.hpp file for type values.
    switch (type)
    {
        case WPNTYPE_NONE:
            mAttackType = ACTION_DEFAULT;
            break;
        case WPNTYPE_KNIFE:
        case WPNTYPE_SWORD:
            mAttackType = ACTION_ATTACK_STAB;
            break;
        case WPNTYPE_THROWN:
            mAttackType = ACTION_ATTACK_THROW;
            break;
        case WPNTYPE_BOW:
            mAttackType = ACTION_ATTACK_BOW;
            break;
        case WPNTYPE_POLEARM:
            mAttackType = ACTION_ATTACK_SWING;
            break;
        default:
            mAttackType = ACTION_ATTACK;
    }
}

void ItemInfo::addSound(EquipmentSoundEvent event, const std::string &filename)
{
    mSounds[event].push_back("sfx/" + filename);
}

const std::string &ItemInfo::getSound(EquipmentSoundEvent event) const
{
    static const std::string empty;
    std::map< EquipmentSoundEvent, std::vector<std::string> >::const_iterator i;
    i = mSounds.find(event);

    return i == mSounds.end() ? empty : i->second[rand() % i->second.size()];
}
