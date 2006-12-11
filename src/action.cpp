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

#include "action.h"

#include <algorithm>

#include "animation.h"
#include "utils/dtor.h"


Action::Action()
{
}

Action::~Action()
{
    std::for_each(mAnimations.begin(), mAnimations.end(),
                  make_dtor(mAnimations));
}

Animation*
Action::getAnimation(int direction) const
{
    Animations::const_iterator i = mAnimations.find(direction);

    // When the direction isn't defined, try the default
    if (i == mAnimations.end())
    {
        i = mAnimations.find(0);
    }

    return (i == mAnimations.end()) ? NULL : i->second;
}

void
Action::setAnimation(int direction, Animation *animation)
{
    // Set first direction as default direction
    if (mAnimations.empty())
    {
        mAnimations[0] = animation;
    }

    mAnimations[direction] = animation;
}
