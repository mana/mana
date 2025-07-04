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

#include "resources/resource.h"

#include "log.h"

#include "resources/resourcemanager.h"

#include <cassert>

void Resource::decRef(OrphanPolicy orphanPolicy)
{
    // Reference may not already have reached zero
    if (mRefCount == 0) {
        Log::warn("mRefCount already zero for %s", mIdPath.c_str());
        assert(false);
    }

    --mRefCount;

    if (mRefCount == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();

        switch (orphanPolicy)
        {
            case DeleteLater:
            default:
                resman->release(this);
                break;
            case DeleteImmediately:
                resman->remove(this);
                delete this;
                break;
        }
    }
}
