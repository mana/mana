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

#pragma once

#include "sprite.h"

#include "resources/image.h"

class Graphics;

class ImageSprite final : public Sprite
{
public:
    ImageSprite(Image *image);

    ~ImageSprite() override;

    bool reset() override
    { return false; }

    bool play(const std::string &action) override
    { return false; }

    bool update(int time) override
    { return false; }

    bool draw(Graphics *graphics, int posX, int posY) const override;

    int getWidth() const override
    { return mImage->getWidth(); }

    int getHeight() const override
    { return mImage->getHeight(); }

    const Image *getImage() const override
    { return mImage; }

    bool setDirection(SpriteDirection direction) override
    { return false; }

    int getDuration() const override
    { return 0; }

private:
    ResourceRef<Image> mImage;
};
