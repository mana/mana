/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "resources/animation.h"

#include "utils/dtor.h"

Animation::Animation():
    mDuration(0)
{
}

void Animation::addFrame(Image *image, unsigned int delay,
                         int offsetX, int offsetY)
{
    Frame frame = { image, delay, offsetX, offsetY };
    mFrames.push_back(frame);
    mDuration += delay;
}

void Animation::addTerminator()
{
    addFrame(NULL, 0, 0, 0);
}

bool Animation::isTerminator(const Frame &candidate)
{
    return (candidate.image == NULL);
}
