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

#include "buddylist.h"
#include <iostream>

BuddyList::BuddyList()
{
}

BuddyList::~BuddyList()
{
}

bool BuddyList::addBuddy(const std::string buddy)
{
    for (buddyit = buddylist.begin(); buddyit != buddylist.end(); buddyit++)
    {
        // Buddy already exist
        if (*buddyit == buddy) return false;
    }

    // Buddy doesnt exist, add it
    buddylist.push_back(buddy);

    return true;
}

bool BuddyList::removeBuddy(const std::string buddy)
{
    if (buddylist.size() > 0) {
        for (buddyit = buddylist.begin(); buddyit != buddylist.end(); buddyit++)
        {
            // Buddy exist, remove it
            if (*buddyit == buddy) {
                buddylist.remove(buddy);
                return true;
            }
        }
    }

    // Buddy doesnt exist
    return false;
}

int  BuddyList::getNumberOfElements(void)
{
    return buddylist.size();
}

std::string BuddyList::getElementAt(int number)
{
    if (number <= buddylist.size() - 1)
    {
        buddyit = buddylist.begin();
        std::advance(buddyit, number);
        return *buddyit;
    }

    return "";
}
