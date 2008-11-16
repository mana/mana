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
 */

#ifndef _TMW_NPC_H
#define _TMW_NPC_H

#include "being.h"

class Graphics;

class NPC : public Being
{
    public:
        NPC(Uint16 id, int sprite, Map *map);

        virtual Type
        getType() const;

        virtual void
        drawName(Graphics *graphics, Sint32 offsetX, Sint32 offsetY);

        void talk();
        void nextDialog();
        void dialogChoice(int choice);

        void buy();
        void sell();

        /**
         * Gets the way an NPC is blocked by other things on the map
         */
        virtual unsigned char getWalkMask() const
        { return 0x83; } // blocked like a monster by walls, monsters and characters ( bin 1000 0011)

    protected:
        /**
         * Gets the way a monster blocks pathfinding for other objects
         */
        virtual Map::BlockType getBlockType() const
        { return Map::BLOCKTYPE_CHARACTER; } //blocks like a player character
};

extern NPC *current_npc;

#endif
