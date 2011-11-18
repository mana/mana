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

#ifndef MUSIC_H
#define MUSIC_H

#include "resources/resource.h"

#ifdef __APPLE__
#include <SDL/SDL_mixer.h>
#else
#include <SDL_mixer.h>
#endif

/**
 * Defines a class for loading and storing music.
 */
class Music : public Resource
{
    public:
        virtual ~Music();

        /**
         * Loads a music from a buffer in memory.
         *
         * @param buffer     The memory buffer containing the music data.
         * @param bufferSize The size of the memory buffer in bytes.
         *
         * @return <code>NULL</code> if the an error occurred, a valid pointer
         *         otherwise.
         */
        static Resource *load(void *buffer, unsigned bufferSize);

        /**
         * Plays the music.
         *
         * @param loops     Number of times to repeat the playback.
         *
         * @return <code>true</code> if the playback started properly
         *         <code>false</code> otherwise.
         */
        virtual bool play(int loops);

        /**
         * Stops the music.
         */
        virtual void stop();

    protected:
        Music(Mix_Chunk *music);

        //Mix_Music *music;
        Mix_Chunk *mChunk;
        int mChannel;
};

#endif
