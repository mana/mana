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

#ifndef IMAGESPRITE_H
#define IMAGESPRITE_H

#include "sprite.h"

#include "resources/image.h"

class Graphics;

class ImageSprite : public Sprite
{
public:
    ImageSprite(Image *image);

    ~ImageSprite();

    bool reset()
    { return false; }

    bool play(std::string action)
    { return false; }

    bool update(int time)
    { return false; }

    bool draw(Graphics* graphics, int posX, int posY) const;

    int getWidth() const
    { return mImage->getWidth(); }

    int getHeight() const
    { return mImage->getHeight(); }

    const Image* getImage() const
    { return mImage; }

    virtual bool setDirection(SpriteDirection direction)
    { return false; }

    int getNumberOfLayers()
    { return 1; }

    size_t getCurrentFrame() const
    { return 0; }

    size_t getFrameCount() const
    { return 1; }

private:
    Image *mImage;
};

#endif // IMAGESPRITE_H
