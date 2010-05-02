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

#include "utils/dtor.h"

CompoundSprite::CompoundSprite()
{
    mAlpha = 1.0f;
}

CompoundSprite::~CompoundSprite()
{
    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        delete (*it);

    clear();
}

void CompoundSprite::reset()
{
    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if (*it)
            (*it)->reset();
}

void CompoundSprite::play(SpriteAction action)
{
    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if (*it)
            (*it)->play(action);
}

void CompoundSprite::update(int time)
{
    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if (*it)
            (*it)->update(time);
}

bool CompoundSprite::draw(Graphics* graphics, int posX, int posY) const
{
    SpriteConstIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
    {
        if (*it)
        {
            if ((*it)->getAlpha() != mAlpha)
                (*it)->setAlpha(mAlpha);
            (*it)->draw(graphics, posX, posY);
        }
    }

    return true;
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

Image* CompoundSprite::getImage() const
{
    // TODO http://bugs.manasource.org/view.php?id=24
    return NULL;
}

void CompoundSprite::setDirection(SpriteDirection direction)
{
    SpriteIterator it, it_end;
    for (it = begin(), it_end = end(); it != it_end; it++)
        if (*it)
            (*it)->setDirection(direction);
}
