/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef GAME_H
#define GAME_H

#include <string>

class Map;
class MiniStatusWindow;
class WindowMenu;

/**
 * The main class responsible for running the game. The game starts after you
 * have selected your character.
 */
class Game
{
    public:
        /**
         * Constructs the game, creating all the managers, handlers, engines
         * and GUI windows that make up the game.
         */
        Game();

        /**
         * Destructor, cleans up the game.
         */
        ~Game();

        /**
         * Provides access to the game instance.
         */
        static Game *instance() { return mInstance; }

        /**
         * This method takes the game a small step further. It is called 100
         * times per second.
         */
        void logic();

        void handleInput();

        void changeMap(const std::string &mapName);

        /**
         * Returns the currently active map.
         */
        Map *getCurrentMap() { return mCurrentMap; }

        const std::string &getCurrentMapName() { return mMapName; }

        /**
         * Convenience functions used to get the current tile width and height.
         */
        int getCurrentTileWidth() const;
        int getCurrentTileHeight() const;

        /**
         * Update the key shortcuts in the window menu.
         */
        void updateWindowMenuCaptions();

        void videoResized(int width, int height);

    private:
        int mLastTarget;
        bool mDisconnected;

        WindowMenu *mWindowMenu;
        MiniStatusWindow *mMiniStatusWindow;

        Map *mCurrentMap;
        std::string mMapName;

        static Game *mInstance;
};

#endif
