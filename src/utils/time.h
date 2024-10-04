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

 #ifndef UTILS_TIME_H
 #define UTILS_TIME_H

#include <cstdint>

/**
 * The amount of milliseconds in a logic tick.
 */
static constexpr unsigned MILLISECONDS_IN_A_TICK = 10;

namespace Time
{

/**
 * The time in milliseconds since the program started (wraps around after ~49
 * days).
 */
uint32_t absoluteTimeMs();

/**
 * The time in milliseconds since the last frame, but never more than 1000.
 */
unsigned deltaTimeMs();

/**
 * The time in seconds since the last frame, but never more than 1.
 */
float deltaTime();

/**
 * Called at the start of each frame, updates the above variables.
 */
void beginFrame();

} // namespace Time

/**
 * Simple timer that can be used to check if a certain amount of time
 * has passed.
 */
class Timer
{
public:
    Timer() = default;

    /**
     * Sets the timer with an optional duration in milliseconds.
     */
    void set(uint32_t ms = 0)
    { mTimeout = Time::absoluteTimeMs() + ms; }

    /**
     * Reset the timer.
     */
    void reset()
    { mTimeout = 0; }

    /**
     * Returns whether the timer has been set.
     */
    bool isSet() const
    { return mTimeout != 0; }

    /**
     * Returns whether the timer has passed.
     * A timer that wasn't set will always return true.
     */
    bool passed() const
    { return elapsed() >= 0; }

    /**
     * Extend the timer by the given number of milliseconds.
     */
    void extend(uint32_t ms)
    { mTimeout += ms; }

    /**
     * Returns the number of milliseconds elapsed since the set time, or a
     * negative value if the timer hasn't passed.
     *
     * Due to wrapping, this function is not suitable for measuring long
     * periods of time (tens of days).
     */
    int32_t elapsed() const;

private:
    uint32_t mTimeout = 0;
};

#endif // UTILS_TIME_H
