/*
 *  The Mana World
 *  Copyright (C) 2006  The Mana World Development Team
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

#include "rotationalparticle.h"
#include "graphics.h"
#include "simpleanimation.h"

#define PI 3.14159265

RotationalParticle::RotationalParticle(Map *map, Animation *animation):
    ImageParticle(map, NULL),
    mAnimation(new SimpleAnimation(animation))
{
}

RotationalParticle::RotationalParticle(Map *map, xmlNodePtr animationNode):
    ImageParticle(map, 0),
    mAnimation(new SimpleAnimation(animationNode))
{
}

RotationalParticle::~RotationalParticle()
{
    delete mAnimation;
    mImage = NULL;
}

bool RotationalParticle::update()
{
    // TODO: cache velocities to avoid spamming atan2()

    float rad = atan2(mVelocity.x, mVelocity.y);
    if (rad < 0)
        rad = PI + (PI + rad);
    int size = mAnimation->getLength();
    float range = PI / size;

    // Determines which frame the particle should play
    if (rad < range || rad > ((PI*2) - range))
    {
        mAnimation->setFrame(0);
    }
    else
    {
        for (int c = 1; c < size; c++)
        {
            if (((c * (2 * range)) - range) < rad && rad < ((c * (2 * range)) + range))
            {
                mAnimation->setFrame(c);
                break;
            }
        }
    }

    mImage = mAnimation->getCurrentImage();

    return Particle::update();
}
