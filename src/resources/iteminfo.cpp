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

void
ItemInfo::setAttackType(const std::string &attackType)
{
    if (attackType == "swing")
    {
        mAttackType = ACTION_ATTACK_SWING;
    }
    else if (attackType == "stab")
    {
        mAttackType = ACTION_ATTACK_STAB;
    }
    else if (attackType == "bow")
    {
        mAttackType = ACTION_ATTACK_BOW;
    }
    else if (attackType == "throw")
    {
        mAttackType = ACTION_ATTACK_THROW;
    }
    else if (attackType == "none")
    {
        mAttackType = ACTION_DEFAULT;
    }
    else
    {
        mAttackType = ACTION_ATTACK;
    }
}

void
ItemInfo::addSound(EquipmentSoundEvent event, const std::string &filename)
{
    mSounds[event].push_back("sfx/" + filename);
}


const std::string&
ItemInfo::getSound(EquipmentSoundEvent event) const
{
    static const std::string empty;
    std::map< EquipmentSoundEvent, std::vector<std::string> >::const_iterator i;
    i = mSounds.find(event);

    return i == mSounds.end() ? empty : i->second[rand() % i->second.size()];
}
