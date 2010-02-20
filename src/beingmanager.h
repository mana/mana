/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BEINGMANAGER_H
#define BEINGMANAGER_H

#include "being.h"

class LocalPlayer;
class Map;

typedef std::list<Being*> Beings;

class BeingManager
{
    public:
        BeingManager();

        ~BeingManager();

        /**
         * Sets the map on which beings are created.
         */
        void setMap(Map *map);

        /**
         * Sets the current player.
         */
        void setPlayer(LocalPlayer *player);

        /**
         * Create a being and add it to the list of beings.
         */
        Being *createBeing(int id, Being::Type type, int subtype);

        /**
         * Remove a Being.
         */
        void destroyBeing(Being *being);

        /**
         * Returns a specific id Being.
         */
        Being *findBeing(int id) const;

        /**
         * Returns a being at specific coordinates.
         */
        Being *findBeing(int x, int y, Being::Type type = Being::UNKNOWN) const;
        Being *findBeingByPixel(int x, int y) const;

        /**
         * Returns a being nearest to specific coordinates.
         *
         * @param x           X coordinate in pixels.
         * @param y           Y coordinate in pixels.
         * @param maxTileDist Maximal distance in tiles. If minimal distance is
         *                    larger, no being is returned.
         * @param type        The type of being to look for.
         */
        Being *findNearestLivingBeing(int x, int y, int maxTileDist,
                                      Being::Type type = Being::UNKNOWN) const;

        /**
         * Returns a being nearest to another being.
         *
         * @param aroundBeing The being to search around.
         * @param maxTileDist Maximal distance in tiles. If minimal distance is
         *                    larger, no being is returned.
         * @param type        The type of being to look for.
         */
         Being *findNearestLivingBeing(Being *aroundBeing, int maxTileDist,
                                       Being::Type type = Being::UNKNOWN) const;

       /**
        * Finds a being by name and (optionally) by type.
        */
        Being *findBeingByName(const std::string &name,
                               Being::Type type = Being::UNKNOWN) const;

        /**
         * Returns the whole list of beings.
         */
        const Beings &getAll() const;

        /**
         * Returns true if the given being is in the manager's list, false
         * otherwise.
         *
         * \param being the being to search for
         */
        bool hasBeing(Being *being) const;

        /**
         * Performs being logic and deletes dead beings when they have been
         * dead long enough.
         */
        void logic();

        /**
         * Destroys all beings except the local player
         */
        void clear();

        void getPlayerNames(std::vector<std::string> &names,
                            bool npcNames);

    protected:
        Beings mBeings;
        Map *mMap;
};

extern BeingManager *beingManager;

#endif
