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

#ifndef ITEM_MANAGER_H
#define ITEM_MANAGER_H

#include <list>
#include <map>
#include <string>

#include "utils/xml.h"

#define ITEMS_DB_FILE "items.xml"

class ItemInfo;
class SpriteDisplay;

namespace TmwAthena {
class TaItemDB;
}

namespace ManaServ {
class ManaServItemDB;
}

/**
 * Nano-description functions
 */
class ItemStat
{
    friend class ItemDB;
    friend class TmwAthena::TaItemDB;
    friend class ManaServ::ManaServItemDB;

    public:
        ItemStat(const std::string &tag,
                 const std::string &format):
        mTag(tag), mFormat(format) {}

        bool operator ==(const std::string &name) const
        { return mTag == name; }

    private:
        std::string mTag;
        std::string mFormat;
};

// Used to set nano-description
static std::list<ItemStat> extraStats;
void setStatsList(const std::list<ItemStat> &stats);

/**
 * Item information database generic definition.
 */
class ItemDB
{
    public:
        ItemDB() :
          mUnknown(0),
          mLoaded(false)
        {}

        virtual ~ItemDB()
        {}

        /**
         * Loads the item data from <code>items.xml</code>.
         */
        virtual void load() = 0;

        /**
         * Frees item data.
         */
        virtual void unload();

        /**
         * Tells whether the item database is loaded.
         */
        bool isLoaded() const
        { return mLoaded; }

        bool exists(int id);

        const ItemInfo &get(int id);
        const ItemInfo &get(const std::string &name);

    protected:
        /**
         * Permits to load item definitions which are common
         * for each protocols to avoid code duplication.
         */
        void loadCommonRef(ItemInfo *itemInfo, xmlNodePtr node);

        /**
         * Checks the items parameters consistency.
         */
        virtual void checkItemInfo(ItemInfo* itemInfo);

        /**
         * Registers the item to mItemInfos and mNamedItemsInfos
         */
        void addItem(ItemInfo *itemInfo);

        /**
         * Loads the empty item definition.
         */
        void loadEmptyItemDefinition();

        // Default unknown reference
        ItemInfo *mUnknown;

        bool mLoaded;

    private:
        /**
         * Loads the sprite references contained in a <sprite> tag.
         */
        void loadSpriteRef(ItemInfo *itemInfo, xmlNodePtr node);

        /**
         * Loads the sound references contained in a <sound> tag.
         */
         void loadSoundRef(ItemInfo *itemInfo, xmlNodePtr node);

        /**
         * Loads the floor item references contained in a <floor> tag.
         */
        void loadFloorSprite(SpriteDisplay *display, xmlNodePtr node);

        // Items database
        typedef std::map<int, ItemInfo*> ItemInfos;
        typedef std::map<std::string, ItemInfo*> NamedItemInfos;

        ItemInfos mItemInfos;
        NamedItemInfos mNamedItemInfos;
};

namespace TmwAthena {

class TaItemInfo;

/**
 * Item information database TmwAthena specific class.
 */
class TaItemDB: public ItemDB
{
    public:
        TaItemDB() : ItemDB()
        { load(); }

        ~TaItemDB()
        { unload(); }

        /**
         * Loads the item data from <code>items.xml</code>.
         */
        void load();

    private:
        /**
         * Check items id specific hard limits and log errors found.
         * TODO: Do it.
         */
        void checkHairWeaponsRacesSpecialIds()
        {}

        void checkItemInfo(ItemInfo* itemInfo);
};

} // namespace TmwAthena

namespace ManaServ {

class ManaServItemInfo;

/**
 * Item information database TmwAthena specific class.
 */
class ManaServItemDB: public ItemDB
{
    public:
        ManaServItemDB() : ItemDB()
        { load(); }

        ~ManaServItemDB()
        { unload(); }

        /**
         * Loads the item data from <code>items.xml</code>.
         */
        void load();

    private:
        void checkItemInfo(ItemInfo* itemInfo);
};

} // namespace ManaServ

extern ItemDB *itemDb;

#endif
