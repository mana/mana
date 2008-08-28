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

#ifndef _TMW_NPC_H
#define _TMW_NPC_H

#include "being.h"

class Network;
class Graphics;
class Text;

class NPC : public Being
{
    public:
        NPC(Uint32 id, Uint16 job, Map *map, Network *network);

        ~NPC();

        void setName(const std::string &name);

        virtual Type
        getType() const;

        void talk();
        void nextDialog();
        void dialogChoice(char choice);

        void buy();
        void sell();

    protected:
        Network *mNetwork;
        void updateCoords();
    private:
        Text *mName;
};

extern NPC *current_npc;

#endif
