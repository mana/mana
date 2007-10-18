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

#include "iteminfo.h"

#include "resourcemanager.h"
#include "image.h"
#include "itemdb.h"

ItemInfo::~ItemInfo()
{
    if (mImage)
    {
        mImage->decRef();
    }
}

void
ItemInfo::setImage(const std::string &image)
{
    if (mImage)
    {
        mImage->decRef();
    }

    ResourceManager *resman = ResourceManager::getInstance();
    mImageName = "graphics/items/" + image;
    mImage = ResourceManager::getInstance()->getImage(mImageName);

    if (!mImage)
    {
        mImage = resman->getImage("graphics/gui/unknown-item.png");
    }
}

const std::string&
ItemInfo::getSprite(int gender) const
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
        case 0:     // none
            mAttackType = ACTION_DEFAULT;
            break;
        case 1:     // knife
        case 2:     // sword
            mAttackType = ACTION_ATTACK_STAB;
            break;
        case 8:     // projectile
            mAttackType = ACTION_ATTACK_THROW;
            break;
        case 10:    // bow
            mAttackType = ACTION_ATTACK_BOW;
            break;
        case 11:    // sickle
            mAttackType = ACTION_ATTACK_SWING;
            break;
        default:
            mAttackType = ACTION_ATTACK;
    }
}

void
ItemInfo::addSound(EquipmentSoundEvent event, const std::string &filename)
{
    if (mSounds.find(event) == mSounds.end())
    {
        mSounds[event] = new std::vector<std::string>;
    }

    mSounds[event]->push_back("sfx/" + filename);
}


const std::string&
ItemInfo::getSound(EquipmentSoundEvent event) const
{
    static const std::string empty = "";
    std::map<EquipmentSoundEvent, std::vector<std::string>*>::const_iterator i;
    i = mSounds.find(event);

    return (i == mSounds.end()) ? empty :
        i->second->at(rand() % i->second->size());
}
