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

#include "graphics.h"
#include "item.h"
#include "map.h"
#include "sprite.h"
#include "resources/image.h"

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
        unsigned int getId() const { return mId; }

        /**
         * Returns the item id.
         */
        unsigned int getItemId() const { return mItem->getId(); }

        /**
         * Returns the x coordinate.
         */
        unsigned short getX() const { return mX; }

        /**
         * Returns the y coordinate.
         */
        unsigned short getY() const { return mY; }

        /**
         * Returns the pixel y coordinate.
         *
         * @see Sprite::getPixelY()
         */
        int getPixelY() const { return mY * 32; }

        /**
         * Draws this floor item to the given graphics context.
         *
         * @see Sprite::draw(Graphics, int, int)
         */
        void draw(Graphics *graphics, int offsetX, int offsetY) const
        {
            graphics->drawImage(mItem->getImage(),
                                mX * 32 + offsetX,
                                mY * 32 + offsetY);
        }

    private:
        unsigned int mId;
        unsigned short mX, mY;
        Item *mItem;
        Sprites::iterator mSpriteIterator;
        Map *mMap;
};

#endif
