/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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

#ifndef _TMW_ACTION_H
#define _TMW_ACTION_H

#include <map>

#include <libxml/tree.h>

class Image;

struct AnimationPhase;
class Animation;

/**
 * An action consists of several animations, one for each direction.
 */
class Action
{
    public:
        /**
         * Constructor.
         */
        Action();

        /**
         * Destructor.
         */
        ~Action();

        void
        setAnimation(int direction, Animation *animation);

        /**
         * Resets all animations associated with this action.
         */
        void
        reset();

        Animation*
        getAnimation(int direction) const;

    protected:
        typedef std::map<int, Animation*> Animations;
        typedef Animations::iterator AnimationIterator;
        Animations mAnimations;
};

#endif
