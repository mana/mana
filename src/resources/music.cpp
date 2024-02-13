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

#include "resources/music.h"

#include "log.h"

Music::Music(Mix_Music *music):
    mMusic(music)
{
}

Music::~Music()
{
    Mix_FreeMusic(mMusic);
}

Resource *Music::load(SDL_RWops *rw)
{
    if (Mix_Music *music = Mix_LoadMUS_RW(rw, 1))
    {
        return new Music(music);
    }

    logger->log("Error, failed to load music: %s", Mix_GetError());
    return nullptr;
}

bool Music::play(int loops, int fadeIn)
{
    return Mix_FadeInMusic(mMusic, loops, fadeIn);
}
