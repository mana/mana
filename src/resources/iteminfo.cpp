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

#include "resources/iteminfo.h"

#include "resources/itemdb.h"
#include "configuration.h"

const std::string &ItemInfo::getSprite(Gender gender, int race) const
{
    if (mView)
    {
        // Forward the request to the item defining how to view this item
        return itemDb->get(mView).getSprite(gender, race);
    }

    auto i = mAnimationFiles.find(static_cast<int>(gender) + race * 4);

    // Fall back to ignoring race
    if (race != 0 && i == mAnimationFiles.end())
        i = mAnimationFiles.find(static_cast<int>(gender));

    static const std::string empty;
    return i != mAnimationFiles.end() ? i->second : empty;
}

void ItemInfo::addSound(EquipmentSoundEvent event, const std::string &filename)
{
    mSounds[event].push_back(paths.getStringValue("sfx") + filename);
}

const std::string &ItemInfo::getSound(EquipmentSoundEvent event) const
{
    static const std::string empty;
    auto i = mSounds.find(event);
    return i == mSounds.end() ? empty : i->second[rand() % i->second.size()];
}

void ItemInfo::setSprite(const std::string &animationFile, Gender gender, int race)
{
    mAnimationFiles[static_cast<int>(gender) + race * 4] = animationFile;
}
