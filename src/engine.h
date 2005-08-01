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

#include "map.h"
#include "graphic/spriteset.h"

extern int camera_x, camera_y;

/**
 * Game engine that does the main drawing.
 */
class Engine
{
    public:
        /**
         * Constructor.
         */
        Engine();

        /**
         * Destructor.
         */
        ~Engine();

        /**
         * Returns the currently active map.
         */
        Map *getCurrentMap();

        /**
         * Sets the currently active map.
         */
        void setCurrentMap(Map *newMap);

        /**
         * Performs engine logic.
         */
        void logic();

        /**
         * Draws everything on the screen.
         */
        void draw();

    private:
        Spriteset *emotionset, *npcset, *weaponset, *itemset;
        Map *mCurrentMap;
        Image *attackTarget;
};

extern Engine *engine;

#endif
