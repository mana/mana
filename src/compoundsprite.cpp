/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#include "game.h"
#include "graphics.h"
#include "map.h"

#include "resources/image.h"

#include <SDL.h>

CompoundSprite::CompoundSprite():
        mImage(NULL),
        mAlphaImage(NULL),
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
    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        delete (*it);

    clear();

    delete mImage;
    delete mAlphaImage;
}

bool CompoundSprite::reset()
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if (*it)
            ret |= (*it)->reset();

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::play(std::string action)
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if (*it)
            ret |= (*it)->play(action);

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::update(int time)
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if (*it)
            ret |= (*it)->update(time);

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::draw(Graphics* graphics, int posX, int posY) const
{
    if (mNeedsRedraw)
        redraw();

    if (empty()) // Nothing to draw
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
        for (it = begin(), it_end = end(); it != it_end; it++)
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

const Image* CompoundSprite::getImage() const
{
    return mImage;
}

bool CompoundSprite::setDirection(SpriteDirection direction)
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
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

size_t CompoundSprite::getCurrentFrame() const
{
    SpriteConstIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if (*it)
            return (*it)->getCurrentFrame();

    return 0;
}

size_t CompoundSprite::getFrameCount() const
{
    SpriteConstIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if (*it)
            return (*it)->getFrameCount();

    return 0;
}

void CompoundSprite::addSprite(Sprite* sprite)
{
    push_back(sprite);
    mNeedsRedraw = true;
}

void CompoundSprite::setSprite(int layer, Sprite* sprite)
{
    // Skip if it won't change anything
    if (at(layer) == sprite)
        return;

    if (at(layer))
        delete at(layer);
    at(layer) = sprite;
    mNeedsRedraw = true;
}

void CompoundSprite::removeSprite(int layer)
{
    // Skip if it won't change anything
    if (at(layer) == NULL)
        return;

    delete at(layer);
    at(layer) = NULL;
    mNeedsRedraw = true;
}

void CompoundSprite::clear()
{
    // Skip if it won't change anything
    if (empty())
        return;

    std::vector<Sprite*>::clear();
    mNeedsRedraw = true;
}

void CompoundSprite::ensureSize(size_t layerCount)
{
    // Skip if it won't change anything
    if (size() >= layerCount)
        return;

    resize(layerCount, NULL);
}

/**
 * Returns the curent frame in the current animation of the given layer.
 */
size_t CompoundSprite::getCurrentFrame(size_t layer)
{
    if (layer >= size())
        return 0;

    Sprite *s = getSprite(layer);
    if (s)
        return s->getCurrentFrame();

    return 0;
}

/**
 * Returns the frame count in the current animation of the given layer.
 */
size_t CompoundSprite::getFrameCount(size_t layer)
{
    if (layer >= size())
        return 0;

    Sprite *s = getSprite(layer);
    if (s)
        return s->getFrameCount();

    return 0;
}

static void updateValues(int &dimension, int &pos, int imgDim, int imgOffset)
{
    // Handle going beyond the left/up
    if (imgOffset < 0)
    {
        int temp = -(pos + imgOffset); // Negated for easier use

        if (temp > 0)
        {
            pos += temp;
            dimension += temp;
        }
    }

    // Handle going beyond the right/down
    int temp = pos + imgOffset + imgDim;
    if (temp > dimension)
        dimension = temp;
}

void CompoundSprite::redraw() const
{
    // TODO OpenGL support
    if (Image::getLoadAsOpenGL())
    {
        // Temporary fix for position
        Map *map = Game::instance() ? Game::instance()->getCurrentMap() : 0;
        if (map)
        {
            mOffsetX = map->getTileWidth() / 2;
            mOffsetY = map->getTileHeight();
        }
        else // Char selection screen fix
        {
            mOffsetX = 16;
            mOffsetY = 32;
        }
        mNeedsRedraw = false;
        return;
    }

    Sprite *s = NULL;
    SpriteConstIterator it = begin(), it_end = end();
    for (it = begin(), it_end = end(); it != it_end; it++)
        if ((s = *it))
            break;

    if (!s)
    {
        mWidth = mHeight = mOffsetX = mOffsetY = 0;
        mNeedsRedraw = false;
        return;
    }

    mWidth = s->getWidth();
    mHeight = s->getHeight();
    mOffsetX = s->getOffsetX();
    mOffsetY = s->getOffsetY();
    int posX = mWidth / 2;
    int posY = mHeight;

    for (it++; it != it_end; ++it)
    {
        s = *it;

        if (s)
        {
            updateValues(mWidth, posX, s->getWidth(), s->getOffsetX() - s->getWidth() / 2);
            updateValues(mHeight, posY, s->getHeight(), s->getOffsetY());
        }
    }

    mOffsetX -= posX;
    mOffsetY -= posY;

    Map *map = Game::instance() ? Game::instance()->getCurrentMap() : 0;
    if (map)
    {
        mOffsetX += map->getTileWidth() / 2;
        mOffsetY += map->getTileHeight();
    }
    else // Char selection screen fix
    {
        mOffsetX += 16;
        mOffsetY += 32;
    }

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

    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_HWSURFACE, mWidth, mHeight,
                                            32, rmask, gmask, bmask, amask);

    if (!surface)
        return;

    Graphics *graphics = new Graphics();
    graphics->setBlitMode(Graphics::BLIT_GFX);
    graphics->setTarget(surface);
    graphics->_beginDraw();

    for (it = begin(), it_end = end(); it != it_end; ++it)
    {
        s = *it;

        if (s)
            s->draw(graphics, posX - s->getWidth() / 2, posY - s->getHeight());
    }

    delete graphics;

    SDL_Surface *surfaceA = SDL_CreateRGBSurface(SDL_HWSURFACE, mWidth, mHeight,
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
}
