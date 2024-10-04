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

#include "time.h"

#include <algorithm>

#include <SDL.h>

namespace Time
{

static uint32_t s_absoluteTimeMs;
static unsigned s_deltaTimeMs;

uint32_t absoluteTimeMs()
{
    return s_absoluteTimeMs;
}

unsigned deltaTimeMs()
{
    return s_deltaTimeMs;
}

float deltaTime()
{
    return s_deltaTimeMs / 1000.f;
}

static int32_t getElapsedTime(uint32_t timeMs)
{
    return static_cast<int32_t>(s_absoluteTimeMs - timeMs);
}

void beginFrame()
{
    // todo: Use SDL_GetTicks64 once we require SDL 2.0.18 or newer.

    const uint32_t previousTime = s_absoluteTimeMs;
    s_absoluteTimeMs = SDL_GetTicks();
    s_deltaTimeMs = std::clamp(getElapsedTime(previousTime), 0, 1000);
}

} // namespace Time

int32_t Timer::elapsed() const
{
    return Time::getElapsedTime(mTimeout);
}
