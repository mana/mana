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

#ifndef ACTORSPRITEMANAGER_H
#define ACTORSPRITEMANAGER_H

#include "actorsprite.h"
#include "being.h"
#include "flooritem.h"

#include "gui/widgets/textfield.h"

class LocalPlayer;
class Map;

typedef std::set<ActorSprite*> ActorSprites;
typedef ActorSprites::iterator ActorSpritesIterator;
typedef ActorSprites::const_iterator ActorSpritesConstIterator;

class ActorSpriteManager
{
    public:
        ActorSpriteManager();

        ~ActorSpriteManager();

        /**
         * Sets the map on which ActorSprites are created.
         */
        void setMap(Map *map);

        /**
         * Sets the current player.
         */
        void setPlayer(LocalPlayer *player);

        /**
         * Create a Being and add it to the list of ActorSprites.
         */
        Being *createBeing(int id, ActorSprite::Type type, int subtype);

        /**
         * Create a FloorItem and add it to the list of ActorSprites.
         */
        FloorItem *createItem(int id, int itemId, int x, int y);

        /**
         * Destroys the given ActorSprite at the end of
         * ActorSpriteManager::logic.
         */
        void destroy(ActorSprite *actor);

        /**
         * Returns a specific Being, by id;
         */
        Being *findBeing(int id) const;

        /**
         * Returns a being at specific coordinates.
         */
        Being *findBeing(int x, int y,
                         ActorSprite::Type type = ActorSprite::UNKNOWN) const;

        /**
         * Returns a being at the specific pixel.
         */
        Being *findBeingByPixel(int x, int y) const;

        /**
         * Returns a specific FloorItem, by id.
         */
        FloorItem *findItem(int id) const;

        /**
         * Returns a FloorItem at specific coordinates.
         */
        FloorItem *findItem(int x, int y) const;

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
        const ActorSprites &getAll() const;

        /**
         * Returns true if the given ActorSprite is in the manager's list,
         * false otherwise.
         *
         * \param actor the ActorSprite to search for
         */
        bool hasActorSprite(ActorSprite *actor) const;

        /**
         * Performs ActorSprite logic and deletes ActorSprite scheduled to be
         * deleted.
         */
        void logic();

        /**
         * Destroys all ActorSprites except the local player
         */
        void clear();

        AutoCompleteLister *getPlayerNameLister();

        AutoCompleteLister *getPlayerNPCNameLister();

        void updatePlayerNames();

    protected:
        friend class PlayerNamesLister;
        friend class PlayerNPCNamesLister;

        AutoCompleteLister *mPlayerNames;
        AutoCompleteLister *mPlayerNPCNames;
        ActorSprites mActors;
        ActorSprites mDeleteActors;
        Map *mMap;
};

extern ActorSpriteManager *actorSpriteManager;

#endif // ACTORSPRITEMANAGER_H
