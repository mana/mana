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

#ifndef _TMW_FLOORITEM_H_
#define _TMW_FLOORITEM_H_

#include "graphics.h"
#include "map.h"
#include "sprite.h"

/**
 * An item lying on the floor.
 */
class FloorItem : public Sprite
{
    public:
        /**
         * Constructor.
         */
        FloorItem(unsigned int id,
                  unsigned int itemId,
                  unsigned short x,
                  unsigned short y,
                  Map *map);

        /**
         * Destructor.
         */
        ~FloorItem();

        /**
         * Returns instance id of this item.
         */
        unsigned int
        getId() { return mId; }

        /**
         * Returns the item id.
         */
        unsigned int
        getItemId() { return mItemId; }

        /**
         * Returns the x coordinate.
         */
        unsigned short
        getX() { return mX; }

        /**
         * Returns the y coordinate.
         */
        unsigned short
        getY() { return mY; }

        /**
         * Returns the pixel y coordinate.
         *
         * @see Sprite::getPixelY()
         */
        int
        getPixelY() const { return mY * 32; }

        /**
         * Draws this floor item to the given graphics context.
         *
         * @see Sprite::draw(Graphics, int, int)
         */
        void
        draw(Graphics *graphics, int offsetX, int offsetY)
        {
            graphics->drawImage(mImage,
                                mX * 32 + offsetX,
                                mY * 32 + offsetY);
        }

    private:
        unsigned int mId;
        unsigned int mItemId;
        unsigned short mX, mY;
        Image *mImage;
        Sprites::iterator mSpriteIterator;
        Map *mMap;
};

/** Removes all items from the list */
void empty_floor_items();

/** Add an Item to the list */
void add_floor_item(FloorItem *floorItem);

/** Return a specific id FloorItem */
FloorItem *find_floor_item_by_id(unsigned int int_id);

/** Remove a FloorItem */
void remove_floor_item(unsigned int int_id);

/** Find a Item id based on its coordinates */
unsigned int find_floor_item_by_cor(unsigned short x, unsigned short y);

#endif
