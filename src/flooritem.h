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

#ifndef FLOORITEM_H
#define FLOORITEM_H

#include <list>

#include "sprite.h"

class Graphics;
class Image;
class Item;
class Map;

typedef std::list<Sprite*> Sprites;

/**
 * An item lying on the floor.
 */
class FloorItem : public Sprite
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
        int getId() const { return mId; }

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
         * Returns the x coordinate in tiles.
         */
        int getX() const { return mX; }

        /**
         * Returns the y coordinate in tiles.
         */
        int getY() const { return mY; }

        /**
         * Returns the pixel y coordinate.
         *
         * @see Sprite::getPixelY()
         */
        int getPixelY() const { return mY * 32 + 16; }

        /**
         * Draws this floor item to the given graphics context.
         *
         * @see Sprite::draw(Graphics, int, int)
         */
        void draw(Graphics *graphics, int offsetX, int offsetY) const;

    private:
        int mId;
        int mX, mY;
        Item *mItem;
        Sprites::iterator mSpriteIterator;
        Map *mMap;
};

#endif
