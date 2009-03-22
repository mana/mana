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

#ifndef NPC_H
#define NPC_H

#include "player.h"

#ifdef EATHENA_SUPPORT
class Network;
#endif
class Graphics;
class Text;

class NPC : public Player
{
    public:
#ifdef TMWSERV_SUPPORT
        NPC(Uint16 id, int sprite, Map *map);
#else
        NPC(Uint32 id, Uint16 job, Map *map, Network *network);
#endif

        ~NPC();

        void setName(const std::string &name);
        void setGender(Gender gender);
        void setSprite(int slot, int id, std::string color);

        virtual Type getType() const;

        void talk();
        void nextDialog();
        void dialogChoice(char choice);
        void integerInput(int value);
        void stringInput(const std::string &value);

        void buy();
        void sell();

        /**
         * Call this to ease clean up of the current NPC, without causing
         * interface problems
         */
        void handleDeath();

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

#ifdef EATHENA_SUPPORT
        Network *mNetwork;
#endif
        void updateCoords();
    private:
        Text *mName;
};

extern NPC *current_npc;

#endif
