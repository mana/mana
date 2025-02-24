/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "simpleanimation.h"

#include "graphics.h"

#include "resources/animation.h"
#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"

SimpleAnimation::SimpleAnimation(Animation animation)
    : mAnimation(std::move(animation))
    , mInitialized(true)
{
    if (mAnimation.getLength() > 0)
        mCurrentFrame = mAnimation.getFrame(0);
}

SimpleAnimation::SimpleAnimation(XML::Node animationNode,
                                 const std::string &dyePalettes)
{
    if (animationNode)
    {
        mAnimation = Animation::fromXML(animationNode, dyePalettes);
        mInitialized = true;
    }

    if (mAnimation.getLength() > 0)
        mCurrentFrame = mAnimation.getFrame(0);
}

bool SimpleAnimation::draw(Graphics *graphics, int posX, int posY) const
{
    if (!mCurrentFrame || !mCurrentFrame->image)
        return false;

    return graphics->drawImage(mCurrentFrame->image,
                               posX + mCurrentFrame->offsetX,
                               posY + mCurrentFrame->offsetY);
}

void SimpleAnimation::reset()
{
    mAnimationTime = 0;
    mAnimationPhase = 0;
}

void SimpleAnimation::setFrame(int frame)
{
    if (frame < 0)
        frame = 0;
    if (frame >= mAnimation.getLength())
        frame = mAnimation.getLength() - 1;
    mAnimationPhase = frame;
    mCurrentFrame = mAnimation.getFrame(mAnimationPhase);
}

void SimpleAnimation::update(int dt)
{
    if (mInitialized)
    {
        mAnimationTime += dt;

        while (mAnimationTime > mCurrentFrame->delay && mCurrentFrame->delay > 0)
        {
            mAnimationTime -= mCurrentFrame->delay;
            mAnimationPhase++;

            if (mAnimationPhase >= mAnimation.getLength())
                mAnimationPhase = 0;

            mCurrentFrame = mAnimation.getFrame(mAnimationPhase);
        }
    }
}

Image *SimpleAnimation::getCurrentImage() const
{
    if (mCurrentFrame)
        return mCurrentFrame->image;
    return nullptr;
}
