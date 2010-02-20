/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#ifndef ACTION_H
#define ACTION_H

#include <libxml/tree.h>

#include <map>

class Animation;

/**
 * An action consists of several animations, one for each direction.
 */
class Action
{
    public:
        Action();

        ~Action();

        void setAnimation(int direction, Animation *animation);

        Animation *getAnimation(int direction) const;

    protected:
        typedef std::map<int, Animation*> Animations;
        typedef Animations::iterator AnimationIterator;
        Animations mAnimations;
};

#endif
