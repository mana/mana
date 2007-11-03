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

#ifndef _TMW_PLAYER_H
#define _TMW_PLAYER_H

#include "being.h"

class Graphics;
class Map;

/**
 * A player being. Players have their name drawn beneath them. This class also
 * implements player-specific loading of base sprite, hair sprite and equipment
 * sprites.
 */
class Player : public Being
{
    public:
        /**
         * Constructor.
         */
        Player(int id, int job, Map *map);

        virtual Type
        getType() const;

        virtual void
        drawName(Graphics *graphics, int offsetX, int offsetY);

        /**
         * Sets the sex for this player.
         */
        void setGender(int);

        /**
         * Gets the hair color for this player.
         */
        int getHairColor() const
        { return mHairColor; }

        /**
         * Gets the hair style for this player.
         */
        int getHairStyle() const
        { return mHairStyle; }

        /**
         * Sets the hair style and color for this player.
         */
        void setHairStyle(int style, int color);

        /**
         * Sets visible equipments for this player.
         */
        void setVisibleEquipment(int slot, int id);

    private:

        std::vector<int> mEquipmentSpriteIDs;
        Uint8 mGender, mHairStyle, mHairColor;
};

#endif
