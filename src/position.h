/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#ifndef POSITION_H
#define POSITION_H

#include <list>
#include <iostream>

/**
 * A position along a being's path.
 */
struct Position
{
    Position(int x, int y):
        x(x), y(y)
    { }

    int x;
    int y;
};

typedef std::list<Position> Path;
typedef Path::iterator PathIterator;

/**
 * Appends a string representation of a position to the output stream.
 */
std::ostream& operator <<(std::ostream &os, const Position &p);

/**
 * Appends a string representation of a path (sequence of positions) to the
 * output stream.
 */
std::ostream& operator <<(std::ostream &os, const Path &path);

#endif // POSITION_H
