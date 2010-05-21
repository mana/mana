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
#include "openglgraphics.h"
#include "map.h"

#include "resources/image.h"

#include "utils/dtor.h"

#include <SDL.h>

#define BUFFER_WIDTH 100
#define BUFFER_HEIGHT 100

CompoundSprite::CompoundSprite():
        mImage(NULL),
        mAlphaImage(NULL),
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

bool CompoundSprite::play(SpriteAction action)
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

    if (mAlpha == 1.0f && mImage)
        return graphics->drawImage(mImage, posX + mOffsetX, posY + mOffsetY);
    else if (mAlpha && mAlphaImage)
    {
        if (mAlphaImage->getAlpha() != mAlpha)
            mAlphaImage->setAlpha(mAlpha);

        return graphics->drawImage(mAlphaImage,
                                   posX + mOffsetX, posY + mOffsetY);
    }
    else
    {
        SpriteConstIterator it, it_end;
        for (it = begin(), it_end = end(); it != it_end; it++)
        {
            if (*it)
            {
                (*it)->draw(graphics, posX, posY);
            }
        }
    }

    return false;
}

int CompoundSprite::getWidth() const
{
    Sprite *base = NULL;

    SpriteConstIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if ((base = (*it)))
            break;

    if (base)
        return base->getWidth();

    return 0;
}

int CompoundSprite::getHeight() const
{
    Sprite *base = NULL;

    SpriteConstIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if ((base = (*it)))
            break;

    if (base)
        return base->getHeight();

    return 0;
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
    {
        return 1;
    }
    else
    {
        return size();
    }
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
    mNeedsRedraw = true;
}

void CompoundSprite::redraw() const
{
    // TODO OpenGL support
    if (Image::getLoadAsOpenGL())
    {
        mNeedsRedraw = false;
        return;
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

    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_HWSURFACE,
                                            BUFFER_WIDTH, BUFFER_HEIGHT,
                                            32, rmask, gmask, bmask, amask);

    if (!surface)
        return;

    Graphics *graphics = new Graphics();
    graphics->setBlitMode(Graphics::BLIT_GFX);
    graphics->setTarget(surface);
    graphics->_beginDraw();

    int tileX = 32 / 2;
    int tileY = 32;

    Game *game = Game::instance();
    if (game)
    {
        Map *map = game->getCurrentMap();
        tileX = map->getTileWidth() / 2;
        tileY = map->getTileWidth();
    }

    int posX = BUFFER_WIDTH / 2 - tileX;
    int posY = BUFFER_HEIGHT - tileY;

    mOffsetX = tileX - BUFFER_WIDTH / 2;
    mOffsetY = tileY - BUFFER_HEIGHT;

    SpriteConstIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
    {
        if (*it)
        {
            (*it)->draw(graphics, posX, posY);
        }
    }

    delete graphics;

    SDL_Surface *surfaceA = SDL_CreateRGBSurface(SDL_HWSURFACE,
                                            BUFFER_WIDTH, BUFFER_HEIGHT,
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
