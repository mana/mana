/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

void Resource::decRef(Resource *resource, OrphanPolicy orphanPolicy)
{
    // The ResourceManager may already be gone during static destruction, in
    // which case 'resource' could be a dangling pointer. Bail out first.
    if (!ResourceManager::instance)
    {
        Log::warn("Resource::decRef() called after ResourceManager destruction");
        return;
    }

    // Reference may not already have reached zero
    if (resource->mRefCount == 0) {
        Log::warn("mRefCount already zero for %s", resource->mIdPath.c_str());
        assert(false);
    }

    --resource->mRefCount;

    if (resource->mRefCount == 0)
    {
        switch (orphanPolicy)
        {
            case DeleteLater:
            default:
                ResourceManager::instance->release(resource);
                break;
            case DeleteImmediately:
                ResourceManager::instance->remove(resource);
                delete resource;
                break;
        }
    }
}
