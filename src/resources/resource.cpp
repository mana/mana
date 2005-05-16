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

#include "resource.h"

Resource::Resource():
    referenceCount(0)
{
}

bool Resource::isLoaded() const
{
    return loaded;
}

void Resource::incRef()
{
    ++referenceCount;
}

bool Resource::decRef()
{
    /* Warning: There is still a serious problem with the object deleting
     * itself and that is that the resource manager doesn't know about it
     * currently, causing it to crash while trying to clean up. Don't use
     * this function until that is solved. Probably we'll have to make it
     * so that decrementing count goes through resource manager too.
     */
    --referenceCount;

    if (!referenceCount) {
        //delete this;
        return true;
    }

    return false;
}
