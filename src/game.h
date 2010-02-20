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

#ifndef GAME_H
#define GAME_H

#include "configlistener.h"

#include <SDL.h>
#include <SDL_framerate.h>

extern std::string map_path;
extern volatile int fps;
extern volatile int tick_time;
extern const int MILLISECONDS_IN_A_TICK;

class Map;
class WindowMenu;

/**
 * The main class responsible for running the game.
 */
class Game : public ConfigListener
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
         * This method runs the game. It returns when the game stops.
         */
        void exec();

        void handleInput();

        void optionChanged(const std::string &name);

        void changeMap(const std::string &mapName);

        /**
         * Returns the currently active map.
         */
        Map *getCurrentMap() { return mCurrentMap; }

        const std::string &getCurrentMapName() { return mMapName; }

    private:
        int mLastTarget;

        SDL_TimerID mLogicCounterId;
        SDL_TimerID mSecondsCounterId;

        bool mLimitFps;
        FPSmanager mFpsManager;

        WindowMenu *mWindowMenu;

        Map *mCurrentMap;
        std::string mMapName;

        static Game *mInstance;
};

/**
 * Returns elapsed time. (Warning: supposes the delay is always < 100 seconds)
 */
int get_elapsed_time(int start_time);

void setScreenshotDir(const std::string &dir);

#endif
