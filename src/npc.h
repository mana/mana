/*
 *  The Mana World
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#ifndef NPC_H
#define NPC_H

#include "player.h"

class Graphics;
class Text;

class NPC : public Player
{
    public:
        NPC(int id, int job, Map *map);

        void setName(const std::string &name);

        virtual Type getType() const { return Being::NPC; }

        void talk();

        void setSprite(unsigned int slot, int id,
                        const std::string &color = "");

        /**
         * Gets the way an NPC is blocked by other things on the map
         */
        virtual unsigned char getWalkMask() const
        {
            return Map::BLOCKMASK_WALL
                    | Map::BLOCKMASK_CHARACTER
                    | Map::BLOCKMASK_MONSTER;
        }

        /** We consider NPCs (at least for now) to be one layer-sprites */
        virtual int getNumberOfLayers() const
        { return 1; }

        static const bool isTalking();

    protected:
        /**
         * Gets the way a monster blocks pathfinding for other objects
         */
        virtual Map::BlockType getBlockType() const
        { return Map::BLOCKTYPE_CHARACTER; } //blocks like a player character

        // Colors don't change for NPCs
        virtual void updateColors() {}
};

extern int current_npc;

#endif
