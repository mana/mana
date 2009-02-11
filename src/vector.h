/*
 *  Aethyra
 *  Copyright (C) 2007  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

#include <iostream>

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
        Vector(float x, float y, float z = 0.0f):
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
        Vector &operator*=(float c)
        {
            x *= c;
            y *= c;
            z *= c;
            return *this;
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
         * In-place scale vector operator.
         */
        Vector &operator/=(float c)
        {
            x /= c;
            y /= c;
            z /= c;
            return *this;
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
        Vector &operator+=(const Vector &v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
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
        Vector &operator-=(const Vector &v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }

        /**
         * Returns the length of this vector. This method does a relatively
         * slow square root.
         */
        float length() const
        {
            return sqrtf(x * x + y * y + z * z);
        }

        /**
         * Returns the squared length of this vector. Avoids the square root.
         */
        float squaredLength() const
        {
            return x * x + y * y + z * z;
        }

        /**
         * Returns the manhattan length of this vector.
         */
        float manhattanLength() const
        {
            return fabsf(x) + fabsf(y) + fabsf(z);
        }

        /**
         * Returns a normalized version of this vector. This is a unit vector
         * running parallel to it.
         */
        Vector normalized() const
        {
            const float l = length();
            return Vector(x / l, y / l, z / l);
        }

        float x, y, z;
};

/**
 * Appends a string representation of a vector to the output stream.
 */
std::ostream& operator <<(std::ostream &os, const Vector &v);

#endif // VECTOR_H
