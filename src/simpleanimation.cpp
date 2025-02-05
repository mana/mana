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

#include "game.h"
#include "graphics.h"
#include "log.h"

#include "resources/animation.h"
#include "resources/dye.h"
#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"

SimpleAnimation::SimpleAnimation(Animation animation):
    mAnimation(std::move(animation)),
    mCurrentFrame(mAnimation.getFrame(0)),
    mInitialized(true)
{
}

SimpleAnimation::SimpleAnimation(XML::Node animationNode,
                                 const std::string &dyePalettes)
{
    initializeAnimation(animationNode, dyePalettes);
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

int SimpleAnimation::getLength() const
{
    return mAnimation.getLength();
}

Image *SimpleAnimation::getCurrentImage() const
{
    if (mCurrentFrame)
        return mCurrentFrame->image;
    return nullptr;
}

void SimpleAnimation::initializeAnimation(XML::Node animationNode,
                                          const std::string& dyePalettes)
{
    if (!animationNode)
        return;

    std::string imagePath = animationNode.getProperty(                                                   "imageset", "");

    // Instanciate the dye coloration.
    if (!imagePath.empty() && !dyePalettes.empty())
        Dye::instantiate(imagePath, dyePalettes);

    ImageSet *imageset = ResourceManager::getInstance()->getImageSet(
        animationNode.getProperty("imageset", ""),
        animationNode.getProperty("width", 0),
        animationNode.getProperty("height", 0)
    );

    if (!imageset)
        return;

    // Get animation frames
    for (auto frameNode : animationNode.children())
    {
        int delay = frameNode.getProperty("delay", 0);
        int offsetX = frameNode.getProperty("offsetX", 0);
        int offsetY = frameNode.getProperty("offsetY", 0);
        Game *game = Game::instance();
        if (game)
        {
            offsetX -= imageset->getWidth() / 2
                - game->getCurrentTileWidth() / 2;
            offsetY -= imageset->getHeight() - game->getCurrentTileHeight();
        }

        if (frameNode.name() == "frame")
        {
            int index = frameNode.getProperty("index", -1);

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

            mAnimation.addFrame(img, delay, offsetX, offsetY);
        }
        else if (frameNode.name() == "sequence")
        {
            int start = frameNode.getProperty("start", -1);
            int end = frameNode.getProperty("end", -1);

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

                mAnimation.addFrame(img, delay, offsetX, offsetY);
                start++;
            }
        }
        else if (frameNode.name() == "end")
        {
            mAnimation.addTerminator();
        }
    }

    mInitialized = true;
}
