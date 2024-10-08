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

#ifndef COMPOUNDSPRITE_H
#define COMPOUNDSPRITE_H

#include "sprite.h"

#include <vector>

class Image;

class CompoundSprite : public Sprite
{
public:
    CompoundSprite();

    ~CompoundSprite() override;

    bool reset() final;

    bool play(const std::string &action) final;

    bool update(int time) final;

    bool draw(Graphics *graphics, int posX, int posY) const override;

    /**
     * Gets the width in pixels of the first sprite in the list.
     */
    int getWidth() const override
    { return mWidth; }

    /**
     * Gets the height in pixels of the first sprite in the list.
     */
    int getHeight() const override
    { return mHeight; }

    int getOffsetX() const final
    { return mOffsetX; }

    int getOffsetY() const final
    { return mOffsetY; }

    const Image *getImage() const final;

    bool setDirection(SpriteDirection direction) final;

    int getNumberOfLayers() const;

    int getDuration() const final;

    size_t size() const
    { return mSprites.size(); }

    void addSprite(Sprite *sprite);

    void setSprite(int layer, Sprite *sprite);

    Sprite *getSprite(int layer) const
    { return mSprites.at(layer); }

    void removeSprite(int layer);

    void clear();

    void ensureSize(size_t layerCount);

    void doRedraw()
    { mNeedsRedraw = true; }

private:
    void redraw() const;

    mutable Image *mImage = nullptr;
    mutable Image *mAlphaImage = nullptr;

    mutable int mWidth = 0, mHeight = 0;
    mutable int mOffsetX = 0, mOffsetY = 0;

    mutable bool mNeedsRedraw = false;

    std::vector<Sprite*> mSprites;
};

#endif // COMPOUNDSPRITE_H
