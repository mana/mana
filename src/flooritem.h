/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
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
#include "sprite.h"

#include <list>

class Graphics;
class Image;
class Item;

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
        int getPixelY() const
        { return mY * mMap->getTileHeight() + mMap->getTileHeight() / 2; }

        /**
         * Draws this floor item to the given graphics context.
         *
         * @see Sprite::draw(Graphics, int, int)
         */
        void draw(Graphics *graphics, int offsetX, int offsetY) const;

        /**
         * Sets the alpha value of the floor item
         */
        void setAlpha(float alpha)
        { mAlpha = alpha; }

        /**
         * Returns the current alpha opacity of the floor item.
         */
        virtual float getAlpha() const
        { return mAlpha; }

        /** We consider flooritems (at least for now) to be one layer-sprites */
        virtual int getNumberOfLayers() const
        { return 1; }

    private:
        int mId;
        int mX, mY;
        Item *mItem;
        MapSprite mMapSprite;
        Map *mMap;
        float mAlpha;
};

#endif
