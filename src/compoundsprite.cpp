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

#include "animatedsprite.h"
#include "game.h"
#include "graphics.h"
#include "imagesprite.h"
#include "localplayer.h"
#include "map.h"

#include "resources/image.h"

#include "utils/dtor.h"

#include <SDL.h>

CompoundSprite::CompoundSprite():
        mImage(0),
        mAlphaImage(0),
        mWidth(0),
        mHeight(0),
        mOffsetX(0),
        mOffsetY(0),
        mNeedsRedraw(false),
        mDirection(DIRECTION_DOWN)
{
    mAlpha = 1.0f;
}

CompoundSprite::~CompoundSprite()
{
    clear();
}

bool CompoundSprite::reset()
{
    bool ret = false;

    SpriteIterator it, it_end;

    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; ++it)
        if (it->second)
            ret |= it->second->reset();

    for (it = mAlternateSprites[mDirection].begin(),
            it_end = mAlternateSprites[mDirection].end(); it != it_end; ++it)
        if (it->second)
            ret |= it->second->reset();

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::play(std::string action)
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; ++it)
        if (it->second)
            ret |= it->second->play(action);

    for (it = mAlternateSprites[mDirection].begin(),
         it_end = mAlternateSprites[mDirection].end(); it != it_end; ++it)
        if (it->second)
            ret |= it->second->play(action);

    mNeedsRedraw |= ret;
    return ret;
}

bool CompoundSprite::update(int time)
{
    bool ret = false;

    SpriteIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; ++it)
        if (it->second)
            ret |= it->second->update(time);

    for (it = mAlternateSprites[mDirection].begin(),
         it_end = mAlternateSprites[mDirection].end(); it != it_end; ++it)
        if (it->second)
            ret |= it->second->update(time);

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
        std::map<int, std::map<SpriteDirection, Sprite*> >::const_iterator it3,
                                                                        it3_end;
        std::map<SpriteDirection, Sprite*>::const_iterator sprite_it;
        Sprite *s = 0;

        for (it3 = mDisplayedSprites.begin(), it3_end = mDisplayedSprites.end();
            it3 != it3_end; ++it3)
        {
            // Try to find any specific sprite for the given direction
            sprite_it = it3->second.find(mDirection);
            s = 0;

            if (sprite_it != it3->second.end())
                s = sprite_it->second;

            // And draws the normal one when there is none.
            if (!s)
                s = it3->second.at(DIRECTION_DEFAULT);

            if (s)
            {
                if (s->getAlpha() != mAlpha)
                    s->setAlpha(mAlpha);
                s->draw(graphics, posX - s->getWidth() / 2,
                        posY - s->getHeight());
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

    mDirection = direction;

    SpriteIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; ++it)
        if (it->second)
            ret |= it->second->setDirection(direction);

    for (it = mAlternateSprites[mDirection].begin(),
         it_end = mAlternateSprites[mDirection].end(); it != it_end; ++it)
        if (it->second)
            ret |= it->second->setDirection(direction);

    mNeedsRedraw |= ret;
    return ret;
}

int CompoundSprite::getNumberOfLayers() const
{
    if (mImage || mAlphaImage)
        return 1;
    else
        return mSprites.size();
}

bool CompoundSprite::drawnWhenBehind() const
{
    // For now, just draw actors with only one layer when obscured
    return (getNumberOfLayers() == 1);
}

void CompoundSprite::addImageSprite(Image *image)
{
    ImageSprite *sprite = new ImageSprite(image);
    if (sprite)
        addSprite(sprite);
}

void CompoundSprite::addSprite(const std::string& spriteFile, int variant)
{
    setSprite(mSprites.size(), spriteFile, variant);
}

void CompoundSprite::setSprite(int layer, const std::string& spriteFile,
                               int variant)
{
    AnimatedSprite *sprite = AnimatedSprite::load(spriteFile, variant);
    if (!sprite)
        return;

    setSprite(layer, sprite);
}


void CompoundSprite::addSprite(Sprite* sprite)
{
    setSprite(mSprites.size(), sprite);
}

void CompoundSprite::setSprite(int layer, Sprite *sprite)
{
    // Setting an empty sprite object is the same as removing it.
    if (!sprite)
    {
        removeSprite(layer);
        return;
    }

    // First, set the currently used sprite direction
    sprite->setDirection(mDirection);

    SpriteIterator it = mSprites.find(layer);

    // Layer found
    if (it != mSprites.end())
    {
        if (it->second)
        {
            unregisterSprite(layer, it->second, false);
            delete it->second;
        }

        it->second = sprite;
        registerSprite(layer, sprite, false);
    }
    else
    {
        mSprites[layer] = sprite;
        registerSprite(layer, sprite, false);
    }

    mNeedsRedraw = true;
}

void CompoundSprite::setAlternateSprite(int layer,
                                        const std::string &spriteFile,
                                        int variant,
                                   const std::list<SpriteDirection> &directions)
{
    AnimatedSprite *sprite = 0;
    // For each desired directions, set the alternate sprite
    for (std::list<SpriteDirection>::const_iterator i = directions.begin();
         i != directions.end(); ++i)
    {
        SpriteIterator it = mAlternateSprites[*i].find(layer);

        // Load a new sprite instance (one for each direction given)
        sprite = AnimatedSprite::load(spriteFile, variant);
        // Adding an empty sprite is the same as removing it.
        if (!sprite)
        {
            removeAlternateSprite(layer, directions);
            return;
        }

        // Layer found
        if (it != mAlternateSprites[*i].end())
        {
            if (it->second)
            {
                unregisterSprite(layer, it->second, true);
                delete it->second;
            }

            sprite->setDirection(mDirection);
            it->second = sprite;
        }
        else
        {
            mAlternateSprites[*i][layer] = sprite;
        }
        registerSprite(layer, sprite, true, *i);
    }
    mNeedsRedraw = true;
}

void CompoundSprite::registerSprite(int layer, Sprite *sprite, bool alternate,
                                    SpriteDirection direction)
{
    if (!sprite)
        return;

    // if it's the normal sprite or when there isn't at all,
    // we set it in the default one.
    if (!alternate || mDisplayedSprites[layer][DIRECTION_DEFAULT] == 0)
        mDisplayedSprites[layer][DIRECTION_DEFAULT] = sprite;
    // If we're registering the normal one, we don't need to set the specific
    // directions for it.
    if (!alternate)
        return;

    // For alternate sprites, we add every required direction, except
    // the default one, reserved for the normal sprite.
    if (direction != DIRECTION_DEFAULT)
        mDisplayedSprites[layer][direction] = sprite;
}

void CompoundSprite::unregisterSprite(int layer, Sprite *sprite, bool alternate)
{
    // If it's the normal sprite, we'll unregister everything, including the
    // alternate entries.
    if (!alternate)
        mDisplayedSprites[layer][DIRECTION_DEFAULT] = 0;

    // We keep a reference of what should replace the previous value.
    Sprite *replaceSprite = alternate ?
        mDisplayedSprites[layer][DIRECTION_DEFAULT] : 0;

    std::map<SpriteDirection, Sprite*>::iterator it, it_end;
    it = mDisplayedSprites[layer].begin();
    it_end = mDisplayedSprites[layer].end();

    for (; it != it_end; ++it)
    {
        if (it->first == DIRECTION_DEFAULT)
            continue;

        if (it->second && it->second == sprite)
            it->second = replaceSprite;
    }
}

void CompoundSprite::removeAlternateSprite(int layer,
                                   const std::list<SpriteDirection> &directions)
{
    SpriteIterator it;

    for (std::list<SpriteDirection>::const_iterator i = directions.begin();
         i != directions.end(); ++i)
    {
        it = mAlternateSprites[*i].find(layer);
        if (it != mAlternateSprites[*i].end())
        {
            if (!it->second)
                return;

            // Update the displayed sprites map
            unregisterSprite(layer, it->second, true);

            delete it->second;
            it->second = 0;
        }
    }
    mNeedsRedraw = true;
}

void CompoundSprite::removeSprite(int layer)
{
    SpriteIterator it;
    it = mSprites.find(layer);
    if (it != mSprites.end())
    {
        if (!it->second)
            return;

        // Update the displayed sprites map
        unregisterSprite(layer, it->second, false);

        delete it->second;
        it->second = 0;
    }
    mNeedsRedraw = true;
}

void CompoundSprite::resetAlternateSprites()
{
    SpriteIterator it, it_end;
    for (int i = DIRECTION_DEFAULT; i < DIRECTION_INVALID; ++i)
    {
        for (it = mAlternateSprites[i].begin(),
             it_end = mAlternateSprites[i].end(); it != it_end; ++it)
        {
            if (it->second)
            {
                unregisterSprite(it->first, it->second, true);

                delete it->second;
                it->second = 0;
            }
        }
    }
}

void CompoundSprite::clear()
{
    SpriteIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; ++it)
        delete it->second;
    for (int i = DIRECTION_DEFAULT; i < DIRECTION_INVALID; ++i)
    {
        for (it = mAlternateSprites[i].begin(),
             it_end = mAlternateSprites[i].end(); it != it_end; ++it)
            delete it->second;

        mAlternateSprites[i].clear();
    }

    mSprites.clear();
    mDisplayedSprites.clear();

    delete mImage;
    delete mAlphaImage;

    mNeedsRedraw = true;
}

int CompoundSprite::getDuration() const
{
    int duration = 0;
    SpriteConstIterator it, it_end;
    for (it = mSprites.begin(), it_end = mSprites.end(); it != it_end; ++it)
        if (it->second && it->second->getDuration() > duration)
            duration = it->second->getDuration();

    return duration;
}

static void updateValues(int &dimension, int &pos, int imgDimUL,
                         int imgDimRD, int imgOffset)
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

void CompoundSprite::redraw() const
{
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
    Sprite *s = 0;
    SpriteConstIterator it = mSprites.begin(), it_end = mSprites.end();

    int posX = 0;
    int posY = 0;

    for (it = mSprites.begin(); it != it_end; ++it)
    {
        s = it->second;

        if (s)
        {
            updateValues(mWidth, posX, s->getWidth() / 2, s->getWidth() / 2,
                         s->getOffsetX());
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

    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_HWSURFACE, mWidth, mHeight,
                                            32, rmask, gmask, bmask, amask);

    if (!surface)
        return;

    Graphics *graphics = new Graphics();
    graphics->setBlitMode(Graphics::BLIT_GFX);
    graphics->setTarget(surface);
    graphics->_beginDraw();

    std::map<int, std::map<SpriteDirection, Sprite*> >::const_iterator it3,
                                                                       it3_end;
    std::map<SpriteDirection, Sprite*>::const_iterator sprite_it;

    for (it3 = mDisplayedSprites.begin(), it3_end = mDisplayedSprites.end();
        it3 != it3_end; ++it3)
    {
        // Try to find any specific sprite for the given direction
        sprite_it = it3->second.find(mDirection);
        s = 0;

        if (sprite_it != it3->second.end())
            s = sprite_it->second;

        // And draws the normal one when there is none.
        if (!s)
            s = it3->second.at(DIRECTION_DEFAULT);

        if (s)
            s->draw(graphics, posX - s->getWidth() / 2, posY - s->getHeight());
    }

    // Uncomment to see buffer sizes
    /*graphics->fillRectangle(gcn::Rectangle(0, 0, 3, 3));
    graphics->fillRectangle(gcn::Rectangle(mWidth - 3, 0, 3, 3));
    graphics->fillRectangle(gcn::Rectangle(mWidth - 3, mHeight - 3, 3, 3));
    graphics->fillRectangle(gcn::Rectangle(0, mHeight - 3, 3, 3));*/

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
