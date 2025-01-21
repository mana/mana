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

#pragma once

#include "actorsprite.h"

class ItemInfo;

/**
 * An item lying on the floor.
 */
class FloorItem : public ActorSprite
{
    public:
        /**
         * Constructor.
         *
         * @param id            the unique ID of this item instance
         * @param itemId        the item ID
         * @param position      the position in pixels
         * @param map           the map this item is on
         */
        FloorItem(int id,
                  int itemId,
                  const Vector &position,
                  Map *map);

        Type getType() const override { return FLOOR_ITEM; }

        /**
         * Returns the item ID.
         */
        int getItemId() const
        { return mItemId; }

        /**
         * Returns the item info for this floor item. Useful for adding an item
         * link for the floor item to chat.
         */
        const ItemInfo &getInfo() const;

        int getTileX() const override
        { return mX; }

        int getTileY() const override
        { return mY; }

    private:
        int mItemId;
        int mX, mY;
};
