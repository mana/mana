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

#include "resources/image.h"

#include "utils/dtor.h"

#include <SDL.h>

CompoundSprite::CompoundSprite():
        mImage(nullptr),
        mAlphaImage(nullptr),
        mWidth(0),
        mHeight(0),
        mOffsetX(0),
        mOffsetY(0),
        mNeedsRedraw(false)
{
    mAlpha = 1.0f;
}

CompoundSprite::~CompoundSprite()
{
    delete_all(mSprites);
    mSprites.clear();

    delete mImage;
    delete mAlphaImage;
}

bool CompoundSprite::reset()
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; it++)
        if (*it)
            ret |= (*it)->reset();

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::play(std::string action)
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; it++)
        if (*it)
            ret |= (*it)->play(action);

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::update(int time)
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; it++)
        if (*it)
            ret |= (*it)->update(time);

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::draw(Graphics *graphics, int posX, int posY) const
{
    if (mNeedsRedraw)
        redraw();

    if (mSprites.empty()) // Nothing to draw
        return false;

    posX += mOffsetX;
    posY += mOffsetY;

    if (mAlpha == 1.0f && mImage)
    {
        return graphics->drawImage(mImage, posX, posY);
    }
    else if (mAlpha && mAlphaImage)
    {
        if (mAlphaImage->getAlpha() != mAlpha)
            mAlphaImage->setAlpha(mAlpha);

        return graphics->drawImage(mAlphaImage,
                                   posX, posY);
    }
    else
    {
        SpriteConstIterator it, it_end;
        for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; it++)
        {
            Sprite *s = *it;
            if (s)
            {
                if (s->getAlpha() != mAlpha)
                    s->setAlpha(mAlpha);
                s->draw(graphics, posX - s->getWidth() / 2, posY - s->getHeight());
            }
        }
    }

    return false;
}

const Image *CompoundSprite::getImage() const
{
    return mImage;
}

bool CompoundSprite::setDirection(SpriteDirection direction)
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; it++)
        if (*it)
            ret |= (*it)->setDirection(direction);

    mNeedsRedraw |= ret;
    return ret;
}

int CompoundSprite::getNumberOfLayers() const
{
    if (mImage || mAlphaImage)
        return 1;
    else
        return size();
}

bool CompoundSprite::drawnWhenBehind() const
{
    // For now, just draw actors with only one layer when obscured
    return (getNumberOfLayers() == 1);
}

void CompoundSprite::addSprite(Sprite *sprite)
{
    mSprites.push_back(sprite);
    mNeedsRedraw = true;
}

void CompoundSprite::setSprite(int layer, Sprite *sprite)
{
    // Skip if it won't change anything
    if (mSprites.at(layer) == sprite)
        return;

    if (mSprites.at(layer))
        delete mSprites.at(layer);
    mSprites[layer] = sprite;
    mNeedsRedraw = true;
}

void CompoundSprite::removeSprite(int layer)
{
    // Skip if it won't change anything
    if (!mSprites.at(layer))
        return;

    delete mSprites.at(layer);
    mSprites.at(layer) = nullptr;
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

int CompoundSprite::getDuration() const
{
    int duration = 0;
    SpriteConstIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; it++)
        if ((*it) && (*it)->getDuration() > duration)
            duration = (*it)->getDuration();

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
    mWidth = mSprites.at(0)->getWidth();
    mHeight = mSprites.at(0)->getHeight();
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
    Sprite *s = nullptr;
    SpriteConstIterator it, it_end = mSprites.end();

    int posX = 0;
    int posY = 0;

    for (it = mSprites.begin(); it != it_end; ++it)
    {
        s = *it;

        if (s)
        {
            updateValues(mWidth, posX, s->getWidth() / 2, s->getWidth() / 2, s->getOffsetX());
            updateValues(mHeight, posY, s->getHeight(), 0, s->getOffsetY());
        }
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

    for (it = mSprites.begin(); it != it_end; ++it)
    {
        s = *it;

        if (s)
            s->draw(graphics, posX - s->getWidth() / 2, posY - s->getHeight());
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
    SDL_BlitSurface(surface, NULL, surfaceA, NULL);

    delete mImage;
    delete mAlphaImage;

    mImage = Image::load(surface);
    SDL_FreeSurface(surface);

    mAlphaImage = Image::load(surfaceA);
    SDL_FreeSurface(surfaceA);

    mNeedsRedraw = false;
#endif
}
