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
#include "../main.h"
#include <iostream>
#include <fstream>

BuddyList::BuddyList()
{
    // Find saved buddy list file
    filename = new std::string(std::string(config.getValue("homeDir", "") + "buddy.txt"));

    // Load buddy from file
    loadFile();
}

BuddyList::~BuddyList()
{
    delete filename;
}

void BuddyList::loadFile(void)
{
    char *buddy;

    // Open file
    std::ifstream inputStream(filename->c_str(), std::ios::in);
    if( !inputStream ) {
        std::cerr << "Error opening input stream" << std::endl;
        return;
    }

    do {
        buddy = (char *) calloc(LEN_USERNAME, sizeof(char));
        inputStream.getline(buddy, LEN_USERNAME);
        // Ugly ?
        if(strcmp(buddy,"") != 0) buddylist.push_back(buddy);
        free(buddy);
    } while(!inputStream.eof());

    // Read buddy and close file
    inputStream.close();
}

void BuddyList::saveFile(void)
{
    std::string str;

    // Open file
    std::ofstream outputStream(filename->c_str(), std::ios::trunc);
    if( !outputStream ) {
        std::cerr << "Error opening output stream" << std::endl;
        return;
    }

    // Write buddy and close file
    for (buddyit = buddylist.begin(); buddyit != buddylist.end(); buddyit++)
    {
        str = *buddyit;
        outputStream << (const char*) str.c_str() << std::endl;
    }
    outputStream.close();
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

    // Save file
    saveFile();

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

                // Save file
                saveFile();
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
    if (number <= (int) buddylist.size() - 1)
    {
        buddyit = buddylist.begin();
        std::advance(buddyit, number);
        return *buddyit;
    }

    return "";
}
