/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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
        Being *createBeing(int id, ActorSprite::Type type, int subtype);

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
        Being *findBeing(int x, int y,
                         ActorSprite::Type type = ActorSprite::UNKNOWN) const;
        Being *findBeingByPixel(int x, int y) const;

        /**
         * Returns a being nearest to specific coordinates.
         *
         * @param x           X coordinate in pixels.
         * @param y           Y coordinate in pixels.
         * @param maxTileDist Maximal distance in tiles. If minimal distance is
         *                    larger, no being is returned.
         * @param type        The type of being to look for.
         * @param excluded    The being to exclude from the search.
         */
        Being *findNearestLivingBeing(int x, int y, int maxTileDist,
                                      ActorSprite::Type type = Being::UNKNOWN,
                                      Being *excluded = 0) const;

        /**
         * Returns a being nearest to another being.
         *
         * @param aroundBeing The being to search around.
         * @param maxTileDist Maximal distance in tiles. If minimal distance is
         *                    larger, no being is returned.
         * @param type        The type of being to look for.
         */
        Being *findNearestLivingBeing(Being *aroundBeing, int maxTileDist,
                                      ActorSprite::Type type = Being::UNKNOWN) const;

        /**
         * Finds a being by name and (optionally) by type.
         */
        Being *findBeingByName(const std::string &name,
                               ActorSprite::Type type = Being::UNKNOWN) const;

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

        void updatePlayerNames();

        /**
         * Destroys the given Being at the end of BeingManager::logic
         */
        void scheduleDelete(Being *being);

    protected:
        Beings mBeings;
        Beings mDeleteBeings;
        Map *mMap;
};

extern BeingManager *beingManager;

#endif
