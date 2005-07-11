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

#include "music.h"

Music::Music(Mix_Chunk *music):
    music(music)
{
    channel = -1;
}

Music::~Music()
{
    //Mix_FreeMusic(music);
    Mix_FreeChunk(music);
    music = NULL;
}

Music* Music::load(void* buffer, unsigned int bufferSize)
{
    // Load the raw file data from the buffer in an RWops structure
    SDL_RWops *rw = SDL_RWFromMem(buffer, bufferSize);

    // Use Mix_LoadMUS to load the raw music data
    //Mix_Music* music = Mix_LoadMUS_RW(rw); Need to be implemeted
    Mix_Chunk *tmpMusic = Mix_LoadWAV_RW(rw, 0);
    
    // Now free the SDL_RWops data
    SDL_FreeRW(rw);

    return new Music(tmpMusic);
}

bool Music::play(int loops)
{
    /*
     * Warning: loops should be always set to -1 (infinite) with current
     * implementation to avoid halting the playback of other samples
     */
     
    /*if (Mix_PlayMusic(music, loops))
        return true;*/
    Mix_VolumeChunk(music, 120);
    channel = Mix_PlayChannel(-1, music, loops);
    if (channel != -1)
        return true;
    return false;
}

bool Music::stop()
{
    /*
     * Warning: very dungerous trick, it could try to stop channels occupied
     * by samples rather than the current music file
     */
     
    //Mix_HaltMusic();
    if (channel != -1)
        Mix_HaltChannel(channel);
    // Never fails
    return true;
}
