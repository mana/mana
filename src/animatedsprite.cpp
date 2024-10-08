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

#include "animatedsprite.h"

#include "graphics.h"

#include "resources/action.h"
#include "resources/animation.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include <cassert>

AnimatedSprite::AnimatedSprite(SpriteDef *sprite):
    mSprite(sprite)
{
    assert(mSprite);

    // Play the stand animation by default
    play(SpriteAction::STAND);
}

AnimatedSprite *AnimatedSprite::load(const std::string &filename, int variant)
{
    ResourceManager *resman = ResourceManager::getInstance();
    SpriteDef *s = resman->getSprite(filename, variant);
    if (!s)
        return nullptr;
    auto *as = new AnimatedSprite(s);
    s->decRef();
    return as;
}

AnimatedSprite::~AnimatedSprite() = default;

bool AnimatedSprite::reset()
{
    bool ret = mFrameIndex !=0 || mFrameTime != 0;

    mFrameIndex = 0;
    mFrameTime = 0;

    return ret;
}

bool AnimatedSprite::play(const std::string &spriteAction)
{
    Action *action = mSprite->getAction(spriteAction);
    if (!action)
        return false;

    mAction = action;
    Animation *animation = mAction->getAnimation(mDirection);

    if (animation && animation != mAnimation && animation->getLength() > 0)
    {
        mAnimation = animation;
        mFrame = mAnimation->getFrame(0);

        reset();

        return true;
    }

    return false;
}

bool AnimatedSprite::update(int time)
{
    if (!mAnimation)
        return false;

    Animation *animation = mAnimation;
    Frame *frame = mFrame;

    if (!updateCurrentAnimation(time))
    {
        // Animation finished, reset to default
        play(SpriteAction::STAND);
    }

    // Make sure something actually changed
    return animation != mAnimation || frame != mFrame;
}

bool AnimatedSprite::updateCurrentAnimation(int time)
{
    if (!mFrame || Animation::isTerminator(*mFrame))
        return false;

    mFrameTime += time;

    while (mFrameTime > mFrame->delay && mFrame->delay > 0)
    {
        mFrameTime -= mFrame->delay;
        mFrameIndex++;

        if (mFrameIndex == mAnimation->getLength())
            mFrameIndex = 0;

        mFrame = mAnimation->getFrame(mFrameIndex);

        if (Animation::isTerminator(*mFrame))
        {
            mAnimation = nullptr;
            mFrame = nullptr;
            return false;
        }
    }

    return true;
}

bool AnimatedSprite::draw(Graphics *graphics, int posX, int posY) const
{
    if (!mFrame)
        return false;

    if (!mFrame->image)
      return false;

    if (mFrame->image->getAlpha() != mAlpha)
        mFrame->image->setAlpha(mAlpha);

    return graphics->drawImage(mFrame->image,
                               posX + mFrame->offsetX,
                               posY + mFrame->offsetY);
}

bool AnimatedSprite::setDirection(SpriteDirection direction)
{
    if (mDirection != direction)
    {
        mDirection = direction;

        if (!mAction)
            return false;

        Animation *animation = mAction->getAnimation(mDirection);

        if (animation && animation != mAnimation && animation->getLength() > 0)
        {
            mAnimation = animation;
            mFrame = mAnimation->getFrame(0);
            reset();
        }

        return true;
    }

    return false;
}

int AnimatedSprite::getDuration() const
{
    if (mAnimation)
        return mAnimation->getDuration();
    return 0;
}

int AnimatedSprite::getWidth() const
{
    if (mFrame && mFrame->image)
        return mFrame->image->getWidth();
    return 0;
}

int AnimatedSprite::getHeight() const
{
    if (mFrame && mFrame->image)
        return mFrame->image->getHeight();
    return 0;
}

int AnimatedSprite::getOffsetX() const
{
    return mFrame ? mFrame->offsetX : 0;
}

int AnimatedSprite::getOffsetY() const
{
    return mFrame ? mFrame->offsetY : 0;
}

const Image *AnimatedSprite::getImage() const
{
    return mFrame ? mFrame->image : nullptr;
}
