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

#include "animation.h"

#include <algorithm>

#include "utils/dtor.h"

Animation::Animation():
    mLength(0)
{
    reset();
}

void
Animation::reset()
{
    mTime = 0;
    iCurrentPhase = mAnimationPhases.begin();
}

bool
Animation::update(unsigned int time)
{
    mTime += time;
    if (mAnimationPhases.empty())
        return true;
    if (isTerminator(*iCurrentPhase))
        return false;

    unsigned int delay = iCurrentPhase->delay;

    while (mTime > delay)
    {
        if (!delay)
            return true;
        mTime -= delay;
        iCurrentPhase++;
        if (iCurrentPhase == mAnimationPhases.end())
        {
            iCurrentPhase = mAnimationPhases.begin();
        }
        if (isTerminator(*iCurrentPhase))
            return false;
        delay = iCurrentPhase->delay;
    }
    return true;
}

const AnimationPhase*
Animation::getCurrentPhase() const
{
    return mAnimationPhases.empty() ? NULL : &(*iCurrentPhase);
}

void
Animation::addPhase(Image *image, unsigned int delay, int offsetX, int offsetY)
{
    // Add new phase to animation list
    AnimationPhase newPhase = { image, delay, offsetX, offsetY};

    mAnimationPhases.push_back(newPhase);
    mLength += delay;
    // Reset animation circle
    iCurrentPhase = mAnimationPhases.begin();
}

void
Animation::addTerminator()
{
    AnimationPhase terminator = { NULL, 0, 0, 0};
    mAnimationPhases.push_back(terminator);
    iCurrentPhase = mAnimationPhases.begin();
}

bool
Animation::isTerminator(AnimationPhase candidate)
{
    return (candidate.image == NULL);
}
