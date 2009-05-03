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

#include "graphics.h"
#include "log.h"
#include "simpleanimation.h"

#include "resources/animation.h"
#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"

SimpleAnimation::SimpleAnimation(Animation *animation):
    mAnimation(animation),
    mAnimationTime(0),
    mAnimationPhase(0),
    mCurrentFrame(mAnimation->getFrame(0))
{
}

SimpleAnimation::SimpleAnimation(xmlNodePtr animationNode):
    mAnimationTime(0),
    mAnimationPhase(0)
{
    mAnimation = new Animation;

    ImageSet *imageset = ResourceManager::getInstance()->getImageSet(
        XML::getProperty(animationNode, "imageset", ""),
        XML::getProperty(animationNode, "width", 0),
        XML::getProperty(animationNode, "height", 0)
    );

    // Get animation frames
    for (   xmlNodePtr frameNode = animationNode->xmlChildrenNode;
            frameNode;
            frameNode = frameNode->next)
    {
        int delay = XML::getProperty(frameNode, "delay", 0);
        int offsetX = XML::getProperty(frameNode, "offsetX", 0);
        int offsetY = XML::getProperty(frameNode, "offsetY", 0);
        offsetY -= imageset->getHeight() - 32;
        offsetX -= imageset->getWidth() / 2 - 16;

        if (xmlStrEqual(frameNode->name, BAD_CAST "frame"))
        {
            int index = XML::getProperty(frameNode, "index", -1);

            if (index < 0)
            {
                logger->log("No valid value for 'index'");
                continue;
            }

            Image *img = imageset->get(index);

            if (!img)
            {
                logger->log("No image at index %d", index);
                continue;
            }

            mAnimation->addFrame(img, delay, offsetX, offsetY);
        }
        else if (xmlStrEqual(frameNode->name, BAD_CAST "sequence"))
        {
            int start = XML::getProperty(frameNode, "start", -1);
            int end = XML::getProperty(frameNode, "end", -1);

            if (start < 0 || end < 0)
            {
                logger->log("No valid value for 'start' or 'end'");
                continue;
            }

            while (end >= start)
            {
                Image *img = imageset->get(start);

                if (!img)
                {
                    logger->log("No image at index %d", start);
                    continue;
                }

                mAnimation->addFrame(img, delay, offsetX, offsetY);
                start++;
            }
        }
        else if (xmlStrEqual(frameNode->name, BAD_CAST "end"))
        {
            mAnimation->addTerminator();
        }
    }

    mCurrentFrame = mAnimation->getFrame(0);
}

bool SimpleAnimation::draw(Graphics* graphics, int posX, int posY) const
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
    if (frame >= mAnimation->getLength())
        frame = mAnimation->getLength() - 1;
    mAnimationPhase = frame;
    mCurrentFrame = mAnimation->getFrame(mAnimationPhase);
}

void SimpleAnimation::update(int timePassed)
{
    mAnimationTime += timePassed;

    while (mAnimationTime > mCurrentFrame->delay && mCurrentFrame->delay > 0)
    {
        mAnimationTime -= mCurrentFrame->delay;
        mAnimationPhase++;

        if (mAnimationPhase >= mAnimation->getLength())
            mAnimationPhase = 0;

        mCurrentFrame = mAnimation->getFrame(mAnimationPhase);
    }
}

int SimpleAnimation::getLength() const
{
    return mAnimation->getLength();
}

Image *SimpleAnimation::getCurrentImage() const
{
    return mCurrentFrame->image;
}

SimpleAnimation::~SimpleAnimation()
{
    delete mAnimation;
}
