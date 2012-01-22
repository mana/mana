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

#ifndef SOUND_EFFECT_H
#define SOUND_EFFECT_H

#include "resources/resource.h"

#ifdef __APPLE__
#include <SDL/SDL_mixer.h>
#else
#include <SDL_mixer.h>
#endif

/**
 * Defines a class for loading and storing sound effects.
 */
class SoundEffect : public Resource
{
    public:
        virtual ~SoundEffect();

        /**
         * Loads a sample from a buffer in memory.
         *
         * @param rw         The SDL_RWops to load the sample data from.
         *
         * @return <code>NULL</code> if the an error occurred, a valid pointer
         *         otherwise.
         */
        static Resource *load(SDL_RWops *rw);

        /**
         * Plays the sample.
         *
         * @param loops     Number of times to repeat the playback.
         * @param volume    Sample playback volume.
         *
         * @return <code>true</code> if the playback started properly
         *         <code>false</code> otherwise.
         */
        virtual bool play(int loops, int volume);

    protected:
        SoundEffect(Mix_Chunk *soundEffect): mChunk(soundEffect) {}

        Mix_Chunk *mChunk;
};

#endif // SOUND_EFFECT_H
