/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef UTILS_MATHUTILS_H
#define UTILS_MATHUTILS_H

/* A very fast function to calculate the approximate inverse square root of a
 * floating point value and a helper function that uses it for getting the
 * normal squareroot. For an explanation of the inverse squareroot function
 * read:
 * http://www.math.purdue.edu/~clomont/Math/Papers/2003/InvSqrt.pdf
 *
 * Unfortunately the original creator of this function seems to be unknown.
 */

inline float fastInvSqrt(float x)
{
    union { int i; float x; } tmp;
    float xhalf = 0.5f * x;
    tmp.x = x;
    tmp.i = 0x5f375a86 - (tmp.i >> 1);
    x = tmp.x;
    x = x * (1.5f - xhalf * x * x);
    return x;
}

inline float fastSqrt(float x)
{
    return 1.0f / fastInvSqrt(x);
}

inline float weightedAverage(float n1, float n2, float w)
{
    if (w < 0.0f)
        return n1;

    if (w > 1.0f)
        return n2;

    return w * n2 + (1.0f - w) * n1;
}

#endif // UTILS_MATHUTILS_H
