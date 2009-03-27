/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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

#ifndef MUSIC_H
#define MUSIC_H

#include "resources/resource.h"

#ifdef __APPLE__
#include <SDL_mixer/SDL_mixer.h>
#else
#include <SDL_mixer.h>
#endif

/**
 * Defines a class for loading and storing music.
 */
class Music : public Resource
{
    public:
        /**
         * Destructor.
         */
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
        /**
         * Constructor.
         */
        Music(Mix_Chunk *music);

        //Mix_Music *music;
        Mix_Chunk *mChunk;
        int mChannel;
};

#endif
