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

#include "resources/music.h"

#include "log.h"

Music::Music(Mix_Chunk *music):
    mChunk(music),
    mChannel(-1)
{
}

Music::~Music()
{
    //Mix_FreeMusic(music);
    Mix_FreeChunk(mChunk);
}

Resource *Music::load(void *buffer, unsigned bufferSize)
{
    // Load the raw file data from the buffer in an RWops structure
    SDL_RWops *rw = SDL_RWFromMem(buffer, bufferSize);

    // Use Mix_LoadMUS to load the raw music data
    //Mix_Music* music = Mix_LoadMUS_RW(rw); Need to be implemeted
    Mix_Chunk *tmpMusic = Mix_LoadWAV_RW(rw, 1);

    if (tmpMusic)
    {
        return new Music(tmpMusic);
    }
    else
    {
        logger->log("Error, failed to load music: %s", Mix_GetError());
        return NULL;
    }
}

bool Music::play(int loops)
{
    /*
     * Warning: loops should be always set to -1 (infinite) with current
     * implementation to avoid halting the playback of other samples
     */

    /*if (Mix_PlayMusic(music, loops))
        return true;*/
    Mix_VolumeChunk(mChunk, 120);
    mChannel = Mix_PlayChannel(-1, mChunk, loops);

    return mChannel != -1;
}

void Music::stop()
{
    /*
     * Warning: very dungerous trick, it could try to stop channels occupied
     * by samples rather than the current music file
     */

    //Mix_HaltMusic();

    if (mChannel != -1)
    {
        Mix_HaltChannel(mChannel);
    }
}
