/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "resources/action.h"

#include "resources/animation.h"

#include "utils/dtor.h"

Action::Action()
{
}

Action::~Action()
{
    delete_all(mAnimations);
}

Animation *Action::getAnimation(int direction) const
{
    Animations::const_iterator i = mAnimations.find(direction);

    // When the given direction is not available, return the first one.
    // (either DEFAULT, or more usually DOWN).
    if (i == mAnimations.end())
        i = mAnimations.begin();

    return (i == mAnimations.end()) ? NULL : i->second;
}

void Action::setAnimation(int direction, Animation *animation)
{
    mAnimations[direction] = animation;
}
