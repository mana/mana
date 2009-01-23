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

#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>

#include "buddylist.h"

#include "../main.h"
#include "../configuration.h"

BuddyList::BuddyList()
{
    // TODO: A buddy list would have to use the Configuration class to store
    //       the buddies. Also, there is now a player relationship manager
    //       which probably makes this buddy list kind of obsolete.

    // Find saved buddy list file
    //mFilename = homeDir + "/buddy.txt";

    // Load buddy from file
    loadFile();
}

void BuddyList::loadFile()
{
    // Open file
    std::ifstream inputStream(mFilename.c_str(), std::ios::in);
    if (!inputStream) {
        std::cerr << "Error opening input stream" << std::endl;
        return;
    }

    do {
        char *buddy = new char[LEN_MAX_USERNAME];
        inputStream.getline(buddy, LEN_MAX_USERNAME);
        // Ugly ?
        if(strcmp(buddy,"")) mBuddylist.push_back(buddy);
        delete [] buddy;
    } while(!inputStream.eof());

    // Read buddy and close file
    inputStream.close();
}

void BuddyList::saveFile()
{
    std::string str;

    // Open file
    std::ofstream outputStream(mFilename.c_str(), std::ios::trunc);
    if (!outputStream) {
        std::cerr << "Error opening output stream" << std::endl;
        return;
    }

    // Write buddy and close file
    for (BuddyIterator i = mBuddylist.begin(); i != mBuddylist.end(); ++i)
    {
        outputStream << (const char*) i->c_str() << std::endl;
    }
    outputStream.close();
}

bool BuddyList::addBuddy(const std::string buddy)
{
    if (find(mBuddylist.begin(), mBuddylist.end(), buddy) != mBuddylist.end())
    {
        return false;
    }

    // Buddy doesnt exist, add it
    mBuddylist.push_back(buddy);

    // Save file
    saveFile();

    return true;
}

bool BuddyList::removeBuddy(const std::string buddy)
{
    BuddyIterator i = find(mBuddylist.begin(), mBuddylist.end(), buddy);

    if (i != mBuddylist.end()) {
        mBuddylist.erase(i);
        saveFile();
        return true;
    }

    return false;
}

int  BuddyList::getNumberOfElements()
{
    return mBuddylist.size();
}

std::string BuddyList::getElementAt(int number)
{
    if (number >= (int) mBuddylist.size()) {
        return "";
    }

    BuddyIterator i = mBuddylist.begin();
    std::advance(i, number);
    return *i;
}
