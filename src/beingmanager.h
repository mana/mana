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

#ifndef _TMW_BEINGMANAGER_H
#define _TMW_BEINGMANAGER_H

#include "being.h"

class LocalPlayer;
class Map;

typedef std::list<Being*> Beings;
typedef Beings::iterator BeingIterator;

class BeingManager
{
    public:
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
        Being* createBeing(Uint16 id, Uint16 job);

        /**
         * Remove a Being.
         */
        void destroyBeing(Being *being);

        /**
         * Return a specific id Being.
         */
        Being* findBeing(Uint16 id);

        /**
         * Return a being at specific coordinates.
         */
        Being* findBeing(Uint16 x, Uint16 y, Being::Type type = Being::UNKNOWN);

        /**
         * Return a being nearest to specific coordinates.
         *
         * \param maxdist maximal distance. If minimal distance is larger,
         *                no being is returned
         */
        Being* findNearestLivingBeing(Uint16 x, Uint16 y, int maxdist,
                                      Being::Type type = Being::UNKNOWN);

        /**
         * Returns the whole list of beings
         */
        Beings& getAll();

        /**
         * Logic.
         */
        void logic();

        /**
         * Destroys all beings except the local player
         */
        void clear();

    protected:
        Beings mBeings;
        Map *mMap;
};

extern BeingManager *beingManager;

#endif
