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

#ifndef _ENGINE_H
#define _ENGINE_H

#include <iosfwd>

class Map;
class Network;

/**
 * Game engine. Actually hardly does anything anymore except keeping track of
 * the current map and loading the emotes.
 */
class Engine
{
    public:
        /**
         * Constructor.
         */
        Engine(Network *network);

        /**
         * Destructor.
         */
        ~Engine();

        /**
         * Returns the currently active map.
         */
        Map *getCurrentMap() { return mCurrentMap; }

        /**
         * Sets the currently active map.
         */
        void changeMap(const std::string &mapName);

        /**
         * Performs engine logic.
         */
        void logic();

    private:
        Map *mCurrentMap;
        Network *mNetwork;
};

extern Engine *engine;

#endif
