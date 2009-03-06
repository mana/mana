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

class Network;
class Graphics;
class Text;

class NPC : public Player
{
    public:
        NPC(int id, Uint16 job, Map *map, Network *network);

        ~NPC();

        void setName(const std::string &name);
        void setGender(Gender gender);
        void setSprite(int slot, int id, std::string color);

        virtual Type getType() const;

        void talk();

        static bool isTalking;
    protected:
        Network *mNetwork;
        void updateCoords();
    private:
        Text *mName;
};

extern int current_npc;

#endif
