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

#include "sprite.h"

#include "graphics.h"

#include "resources/animation.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include <cassert>

Sprite::Sprite(SpriteDef *sprite):
    mSprite(sprite)
{
    assert(mSprite);

    // Play the stand animation by default
    play(SpriteAction::STAND);
}

Sprite *Sprite::load(const std::string &filename, int variant)
{
    ResourceManager *resman = ResourceManager::getInstance();
    auto spriteDef = resman->getSprite(filename, variant);
    if (!spriteDef)
        return nullptr;
    return new Sprite(spriteDef);
}

Sprite::~Sprite() = default;

bool Sprite::reset()
{
    bool ret = mFrameIndex !=0 || mFrameTime != 0;

    mFrameIndex = 0;
    mFrameTime = 0;

    if (mAnimation)
        mFrame = mAnimation->getFrame(0);
    else
        mFrame = nullptr;
    return ret;
}

bool Sprite::play(const std::string &spriteAction)
{
    Action *action = mSprite->getAction(spriteAction);
    if (!action)
        return false;

    mAction = action;
    auto animation = mAction->getAnimation(mDirection);

    if (animation && animation != mAnimation && animation->getLength() > 0)
    {
        mAnimation = animation;
        reset();

        return true;
    }

    return false;
}

bool Sprite::update(int dt)
{
    if (!mAnimation)
        return false;

    const Animation *animation = mAnimation;
    const Frame *frame = mFrame;

    if (!updateCurrentAnimation(dt))
    {
        // Animation finished, reset to default
        play(SpriteAction::STAND);
    }

    // Make sure something actually changed
    return animation != mAnimation || frame != mFrame;
}

bool Sprite::updateCurrentAnimation(int dt)
{
    if (!mFrame || Animation::isTerminator(*mFrame))
        return false;

    mFrameTime += dt;

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

bool Sprite::draw(Graphics *graphics, int posX, int posY) const
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

bool Sprite::setDirection(SpriteDirection direction)
{
    if (mDirection != direction)
    {
        mDirection = direction;

        if (!mAction)
            return false;

        auto animation = mAction->getAnimation(mDirection);

        if (animation && animation != mAnimation && animation->getLength() > 0)
        {
            mAnimation = animation;
            reset();
        }

        return true;
    }

    return false;
}

int Sprite::getDuration() const
{
    if (mAnimation)
        return mAnimation->getDuration();
    return 0;
}

int Sprite::getWidth() const
{
    if (mFrame && mFrame->image)
        return mFrame->image->getWidth();
    return 0;
}

int Sprite::getHeight() const
{
    if (mFrame && mFrame->image)
        return mFrame->image->getHeight();
    return 0;
}

int Sprite::getOffsetX() const
{
    return mFrame ? mFrame->offsetX : 0;
}

int Sprite::getOffsetY() const
{
    return mFrame ? mFrame->offsetY : 0;
}

const Image *Sprite::getImage() const
{
    return mFrame ? mFrame->image : nullptr;
}
