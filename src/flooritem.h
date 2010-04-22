/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef FLOORITEM_H
#define FLOORITEM_H

#include "map.h"

class Graphics;
class Image;
class Item;

/**
 * An item lying on the floor.
 */
class FloorItem : public Actor
{
    public:
        /**
         * Constructor.
         *
         * @param id     the unique ID of this item instance
         * @param itemId the item ID
         * @param x      the x position in tiles
         * @param y      the y position in tiles
         * @param map    the map this item is on
         */
        FloorItem(int id,
                  int itemId,
                  int x,
                  int y,
                  Map *map);

        ~FloorItem();

        /**
         * Returns instance ID of this item.
         */
        int getId() const
        { return mId; }

        /**
         * Returns the item ID.
         */
        int getItemId() const;

        /**
         * Returns the item object. Useful for adding an item link for the
         * floor item to chat.
         */
        Item *getItem() const;

        /**
         * Draws this floor item to the given graphics context.
         *
         * @see Actor::draw(Graphics, int, int)
         */
        void draw(Graphics *graphics, int offsetX, int offsetY) const;

        /** We consider flooritems (at least for now) to be one layer-sprites */
        virtual int getNumberOfLayers() const
        { return 1; }

    private:
        int mId;
        Item *mItem;
};

#endif
