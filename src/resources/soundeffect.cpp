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

#include "resources/soundeffect.h"

#include "log.h"

SoundEffect::~SoundEffect()
{
    Mix_FreeChunk(mChunk);
}

SoundEffect *SoundEffect::load(SDL_RWops *rw)
{
    // Load the music data and free the RWops structure
    if (Mix_Chunk *soundEffect = Mix_LoadWAV_RW(rw, 1))
    {
        return new SoundEffect(soundEffect);
    }

    logger->log("Error, failed to load sound effect: %s", Mix_GetError());
    return nullptr;
}

int SoundEffect::play(int loops, int volume, int channel)
{
    Mix_VolumeChunk(mChunk, volume);
    return Mix_PlayChannel(channel, mChunk, loops);
}
