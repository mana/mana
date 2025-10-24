/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#include "compoundsprite.h"

#include "graphics.h"
#include "map.h"
#include "sprite.h"

#include "resources/image.h"

#include "utils/dtor.h"

#include <SDL.h>

CompoundSprite::~CompoundSprite()
{
    delete_all(mSprites);
    delete mImage;
    delete mAlphaImage;
}

bool CompoundSprite::reset()
{
    bool ret = false;

    for (auto sprite : mSprites)
        if (sprite)
            ret |= sprite->reset();

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::play(const std::string &action)
{
    bool ret = false;

    for (auto sprite : mSprites)
        if (sprite)
            ret |= sprite->play(action);

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::update(int time)
{
    bool ret = false;

    for (auto sprite : mSprites)
        if (sprite)
            ret |= sprite->update(time);

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::draw(Graphics *graphics, int posX, int posY) const
{
    doRedraw();

    if (mSprites.empty()) // Nothing to draw
        return false;

    posX += mOffsetX;
    posY += mOffsetY;

    if (mAlpha == 1.0f && mImage)
    {
        return graphics->drawImage(mImage, posX, posY);
    }

    if (mAlpha && mAlphaImage)
    {
        mAlphaImage->setAlpha(mAlpha);
        return graphics->drawImage(mAlphaImage, posX, posY);
    }

    for (auto sprite : mSprites)
    {
        if (sprite)
        {
            sprite->setAlpha(mAlpha);
            sprite->draw(graphics, posX - sprite->getWidth() / 2, posY - sprite->getHeight());
        }
    }

    return false;
}

bool CompoundSprite::setDirection(SpriteDirection direction)
{
    bool ret = false;

    for (auto sprite : mSprites)
        if (sprite)
            ret |= sprite->setDirection(direction);

    mNeedsRedraw |= ret;
    return ret;
}

int CompoundSprite::getNumberOfLayers() const
{
    if (mImage || mAlphaImage)
        return 1;

    return size();
}

void CompoundSprite::add(Sprite *sprite)
{
    mSprites.push_back(sprite);
    mNeedsRedraw = true;
}

void CompoundSprite::set(int layer, Sprite *sprite)
{
    // Skip if it won't change anything
    if (mSprites.at(layer) == sprite)
        return;

    delete mSprites.at(layer);
    mSprites[layer] = sprite;
    mNeedsRedraw = true;
}

void CompoundSprite::clear()
{
    // Skip if it won't change anything
    if (mSprites.empty())
        return;

    delete_all(mSprites);
    mSprites.clear();
    mNeedsRedraw = true;
}

void CompoundSprite::ensureSize(size_t layerCount)
{
    // Skip if it won't change anything
    if (mSprites.size() >= layerCount)
        return;

    mSprites.resize(layerCount);
}

int CompoundSprite::getMaxDuration() const
{
    int duration = 0;
    for (auto sprite : mSprites)
        if (sprite && sprite->getDuration() > duration)
            duration = sprite->getDuration();

    return duration;
}

#if 0
static void updateValues(int &dimension, int &pos, int imgDimUL, int imgDimRD, int imgOffset)
{
    // Handle going beyond the left/up
    int temp = -(pos + imgOffset - imgDimUL); // Negated for easier use
    if (temp > 0)
    {
        pos += temp;
        dimension += temp;
    }

    // Handle going beyond the right/down
    temp = pos + imgOffset + imgDimRD;
    if (temp > dimension)
        dimension = temp;
}
#endif

void CompoundSprite::redraw() const
{
#if 1   // TODO_SDL2: Does it make sense to implement CompoundSprite?
    auto baseSprite = mSprites.empty() ? nullptr : mSprites.at(0);
    mWidth = baseSprite ? baseSprite->getWidth() : 0;
    mHeight = baseSprite ? baseSprite->getHeight() : 0;
    mOffsetX = 0;
    mOffsetY = 0;
    mNeedsRedraw = false;
#else

#ifdef USE_OPENGL
    // TODO OpenGL support
    if (Image::getLoadAsOpenGL())
    {
        mWidth = mSprites.at(0)->getWidth();
        mHeight = mSprites.at(0)->getHeight();
        mOffsetX = 0;
        mOffsetY = 0;
        mNeedsRedraw = false;
        return;
    }
#endif

    mWidth = mHeight = mOffsetX = mOffsetY = 0;

    int posX = 0;
    int posY = 0;

    for (auto sprite : mSprites)
    {
        if (!sprite)
            continue;

        updateValues(mWidth, posX,
                     sprite->getWidth() / 2,
                     sprite->getWidth() / 2,
                     sprite->getOffsetX());

        updateValues(mHeight, posY,
                     sprite->getHeight(),
                     0,
                     sprite->getOffsetY());
    }

    if (mWidth == 0 && mHeight == 0)
    {
        mNeedsRedraw = false;
        return;
    }

    mOffsetX -= posX;
    mOffsetY -= posY;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int rmask = 0xff000000;
    int gmask = 0x00ff0000;
    int bmask = 0x0000ff00;
    int amask = 0x000000ff;
#else
    int rmask = 0x000000ff;
    int gmask = 0x0000ff00;
    int bmask = 0x00ff0000;
    int amask = 0xff000000;
#endif

    SDL_Surface *surface = SDL_CreateRGBSurface(0, mWidth, mHeight,
                                            32, rmask, gmask, bmask, amask);

    if (!surface)
        return;

    Graphics *graphics = new Graphics();
    graphics->setBlitMode(Graphics::BLIT_GFX);
    graphics->setTarget(surface);
    graphics->_beginDraw();

    for (auto sprite : mSprites)
    {
        if (!sprite)
            continue;

        sprite->draw(graphics,
                     posX - sprite->getWidth() / 2,
                     posY - sprite->getHeight());
    }

    // Uncomment to see buffer sizes
    /*graphics->fillRectangle(gcn::Rectangle(0, 0, 3, 3));
    graphics->fillRectangle(gcn::Rectangle(mWidth - 3, 0, 3, 3));
    graphics->fillRectangle(gcn::Rectangle(mWidth - 3, mHeight - 3, 3, 3));
    graphics->fillRectangle(gcn::Rectangle(0, mHeight - 3, 3, 3));*/

    delete graphics;

    SDL_Surface *surfaceA = SDL_CreateRGBSurface(0, mWidth, mHeight,
                                            32, rmask, gmask, bmask, amask);

    SDL_SetAlpha(surface, 0, SDL_ALPHA_OPAQUE);
    SDL_BlitSurface(surface, nullptr, surfaceA, nullptr);

    delete mImage;
    delete mAlphaImage;

    mImage = Image::load(surface);
    SDL_FreeSurface(surface);

    mAlphaImage = Image::load(surfaceA);
    SDL_FreeSurface(surfaceA);

    mNeedsRedraw = false;
#endif
}
