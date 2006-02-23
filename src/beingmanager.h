/*
 *  the mana world
 *  copyright 2004 the mana world development team
 *
 *  this file is part of the mana world.
 *
 *  the mana world is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  any later version.
 *
 *  the mana world is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with the mana world; if not, write to the free software
 *  foundation, inc., 59 temple place, suite 330, boston, ma  02111-1307  usa
 *
 *  $id: being.cpp,v 1.98 2005/12/25 01:28:03 b_lindeijer exp $
 */

#ifndef _TMW_BEINGMANAGER_H
#define _TMW_BEINGMANAGER_H

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
         * Create a being and add it to the list of beings
         */
        Being* createBeing(Uint32 id, Uint16 job);

        /**
         * Remove a Being
         */
        void destroyBeing(Being *being);

        /**
         * Return a specific id Being
         */
        Being* findBeing(Uint32 id);

        /**
         * Return a being at specific coordinates
         */
        Being* findBeing(Uint16 x, Uint16 y, Being::Type type = Being::UNKNOWN);

        /**
         * Returns the whole list of beings
         */
        Beings* getAll();

        /**
         * Destroys all beings except the local player
         */
        void clear();

    protected:
        Beings mBeings;
        Map *mMap;
        Network *mNetwork;
};

extern BeingManager *beingManager;

#endif
