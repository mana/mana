/*
 *  The Mana World
 *  Copyright 2006 The Mana World Development Team
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

/**
 * Returns a random numeric value that is larger than or equal min and smaller
 * than max
 */

template <typename T> struct MinMax
{
    void set(T min, T max)
    {
        minVal=min; maxVal=max;
    }

    void set(T val)
    {
        set(val, val);
    }

    T value()
    {
        return (T)(minVal + (maxVal - minVal) * (rand() / ((double) RAND_MAX + 1)));
    }

    T minVal;
    T maxVal;
};
