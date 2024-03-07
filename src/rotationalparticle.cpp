/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "rotationalparticle.h"
#include "simpleanimation.h"

#define PI 3.14159265

RotationalParticle::RotationalParticle(Map *map, Animation animation):
    ImageParticle(map, nullptr),
    mAnimation(std::move(animation))
{}

RotationalParticle::RotationalParticle(Map *map, xmlNodePtr animationNode,
                                       const std::string &dyePalettes):
    ImageParticle(map, nullptr),
    mAnimation(animationNode, dyePalettes)
{}

RotationalParticle::~RotationalParticle()
{
    // Prevent ImageParticle from decreasing the reference count of the image
    mImage = nullptr;
}

bool RotationalParticle::update()
{
    // TODO: cache velocities to avoid spamming atan2()

    float rad = atan2(mVelocity.x, mVelocity.y);
    if (rad < 0)
        rad = PI + (PI + rad);
    int size = mAnimation.getLength();
    float range = PI / size;

    // Determines which frame the particle should play
    if (rad < range || rad > ((PI*2) - range))
    {
        mAnimation.setFrame(0);
    }
    else
    {
        for (int c = 1; c < size; c++)
        {
            if (((c * (2 * range)) - range) < rad && rad < ((c * (2 * range)) + range))
            {
                mAnimation.setFrame(c);
                break;
            }
        }
    }

    mImage = mAnimation.getCurrentImage();

    return Particle::update();
}
