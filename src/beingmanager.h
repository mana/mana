/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#ifndef BEINGMANAGER_H
#define BEINGMANAGER_H

#include "being.h"

class LocalPlayer;
class Map;
class Network;

typedef std::list<Being*> Beings;
typedef Beings::iterator BeingIterator;

class BeingManager
{
    public:
        BeingManager(Network *network);

        /**
         * Sets the map on which beings are created
         */
        void setMap(Map *map);

        /**
         * Sets the current player
         */
        void setPlayer(LocalPlayer *player);

        /**
         * Create a being and add it to the list of beings.
         */
        Being *createBeing(Uint32 id, Uint16 job);

        /**
         * Remove a Being.
         */
        void destroyBeing(Being *being);

        /**
         * Return a specific id Being.
         */
        Being* findBeing(Uint32 id);
        Being* findBeingByPixel(Uint16 x, Uint16 y);

        /**
         * Returns a being at specific coordinates.
         */
        Being* findBeing(Uint16 x, Uint16 y, Being::Type type = Being::UNKNOWN);

       /**
        * Returns a being nearest to specific coordinates.
        *
        * @param x       X coordinate.
        * @param y       Y coordinate.
        * @param maxdist Maximal distance. If minimal distance is larger,
        *                no being is returned.
        * @param type    The type of being to look for.
        */
        Being* findNearestLivingBeing(Uint16 x, Uint16 y, int maxdist,
                                      Being::Type type = Being::UNKNOWN);

       /**
        * Finds a being by name and (optionally) by type.
        */
        Being* findBeingByName(std::string name, Being::Type type = Being::UNKNOWN);

       /**
        * Returns a being nearest to another being.
        *
        * \param maxdist maximal distance. If minimal distance is larger,
        *                no being is returned
        */
        Being* findNearestLivingBeing(Being *aroundBeing, int maxdist,
                                      Being::Type type = Being::UNKNOWN);

        /**
         * Returns the whole list of beings
         */
        Beings& getAll();

        /**
         * Returns true if the given being is in the manager's list, false
         * otherwise.
         *
         * \param being the being to search for
         */
        bool hasBeing(Being *being);

        /**
         * Logic.
         */
        void logic();

        /**
         * Destroys all beings except the local player and current NPC (if any)
         */
        void clear();

    protected:
        Beings mBeings;
        Map *mMap;
        Network *mNetwork;
};

extern BeingManager *beingManager;

#endif
