/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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

#ifndef _TMW_VECTOR_H_
#define _TMW_VECTOR_H_

/**
 * Vector class. Represents either a 3D point in space, a velocity or a force.
 * Provides several convenient operator overloads.
 */
class Vector
{
    public:
        /**
         * Constructor.
         */
        Vector():
            x(0.0f),
            y(0.0f),
            z(0.0f)
        {}

        /**
         * Constructor.
         */
        Vector(float x, float y, float z):
            x(x),
            y(y),
            z(z)
        {}

        /**
         * Copy constructor.
         */
        Vector(const Vector &v):
            x(v.x),
            y(v.y),
            z(v.z)
        {}

        /**
         * Scale vector operator.
         */
        Vector operator*(float c) const
        {
            return Vector(x * c,
                          y * c,
                          z * c);
        }

        /**
         * In-place scale vector operator.
         */
        void operator*=(float c)
        {
            x *= c;
            y *= c;
            z *= c;
        }

        /**
         * Scale vector operator.
         */
        Vector operator/(float c) const
        {
            return Vector(x / c,
                          y / c,
                          z / c);
        }

        /**
         * Add vector operator.
         */
        Vector operator+(const Vector &v) const
        {
            return Vector(x + v.x,
                          y + v.y,
                          z + v.z);
        }

        /**
         * In-place add vector operator.
         */
        void operator+=(const Vector &v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
        }

        /**
         * Substract vector operator.
         */
        Vector operator-(const Vector &v) const
        {
            return Vector(x - v.x,
                          y - v.y,
                          z - v.z);
        }

        /**
         * In-place substract vector operator.
         */
        void operator-=(const Vector &v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
        }

        float x, y, z;
};

#endif
