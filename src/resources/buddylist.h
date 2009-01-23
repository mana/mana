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

#ifndef _TMW_BUDDYLIST_H
#define _TMW_BUDDYLIST_H

#include <list>
#include <string>

#include <guichan/listmodel.hpp>

class BuddyList : public gcn::ListModel {
    public:
        /**
         * Constructor
         */
        BuddyList();

        /**
         * Destructor
         */
        virtual ~BuddyList() { }

        /**
         * Adds buddy to the list
         */
        bool addBuddy(const std::string buddy);

        /**
         * Removes buddy from the list
         */
        bool removeBuddy(const std::string buddy);

        /**
         * Returns the number of buddy on the list
         */
        int  getNumberOfElements();

        /**
         * Returns the buddy of the number or null
         */
        std::string getElementAt(int number);

    private:
        /**
         * Save buddy to file
         */
        void saveFile();

        /**
         * Load buddy from file
         */
        void loadFile();

        typedef std::list<std::string> Buddies;
        typedef Buddies::iterator BuddyIterator;
        Buddies mBuddylist;               /**< Buddy list */
        std::string mFilename;            /* File to work with */
};

#endif /* _TMW_BUDDYLIST_H */
