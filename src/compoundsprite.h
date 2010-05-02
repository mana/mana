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

#ifndef COMPOUNDSPRITE_H
#define COMPOUNDSPRITE_H

#include "sprite.h"

#include <vector>

class CompoundSprite : public Sprite, public std::vector<Sprite*>
{
public:
    CompoundSprite();

    ~CompoundSprite();

    virtual void reset();

    virtual void play(SpriteAction action);

    virtual void update(int time);

    virtual bool draw(Graphics* graphics, int posX, int posY) const;

    /**
     * Gets the width in pixels of the first sprite in the list.
     */
    virtual int getWidth() const;

    /**
     * Gets the height in pixels of the first sprite in the list.
     */
    virtual int getHeight() const;

    virtual Image* getImage() const;

    virtual void setDirection(SpriteDirection direction);

    virtual Sprite *getSprite(int index) const
    { return at(index); }

    int getNumberOfLayers()
    { return size(); }
};

typedef CompoundSprite::iterator SpriteIterator;
typedef CompoundSprite::const_iterator SpriteConstIterator;

#endif // COMPOUNDSPRITE_H
