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

#include <vector>

class CompoundSprite
{
public:
    CompoundSprite() = default;
    ~CompoundSprite();

    bool reset();
    bool play(const std::string &action);
    bool update(int time);
    bool draw(Graphics *graphics, int posX, int posY) const;

    /**
     * Gets the width in pixels of the first sprite in the list.
     */
    int getWidth() const { doRedraw(); return mWidth; }

    /**
     * Gets the height in pixels of the first sprite in the list.
     */
    int getHeight() const { doRedraw(); return mHeight; }

    float getAlpha() const { return mAlpha; }
    void setAlpha(float alpha) { mAlpha = alpha; }

    bool setDirection(SpriteDirection direction);

    int getNumberOfLayers() const;

    int getMaxDuration() const;

    size_t size() const { return mSprites.size(); }

    void add(Sprite *sprite);
    void set(int layer, Sprite *sprite);
    void clear();
    void ensureSize(size_t layerCount);

    void doRedraw() const;

private:
    void redraw() const;

    mutable Image *mImage = nullptr;
    mutable Image *mAlphaImage = nullptr;

    mutable int mWidth = 0, mHeight = 0;
    mutable int mOffsetX = 0, mOffsetY = 0;

    mutable bool mNeedsRedraw = false;

    float mAlpha = 1.0f;
    std::vector<Sprite*> mSprites;
};

inline void CompoundSprite::doRedraw() const
{
    if (mNeedsRedraw)
        redraw();
}
