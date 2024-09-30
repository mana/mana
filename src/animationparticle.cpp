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

#include "animationparticle.h"

#include "simpleanimation.h"

#include "utils/time.h"

AnimationParticle::AnimationParticle(Animation animation):
    ImageParticle(nullptr),
    mAnimation(std::move(animation))
{
}

AnimationParticle::AnimationParticle(XML::Node animationNode,
                                     const std::string &dyePalettes):
    ImageParticle(nullptr),
    mAnimation(animationNode, dyePalettes)
{
}

AnimationParticle::~AnimationParticle()
{
    // Prevent ImageParticle from decreasing the reference count of the image
    mImage = nullptr;
}

bool AnimationParticle::update()
{
    mAnimation.update(MILLISECONDS_IN_A_TICK);
    mImage = mAnimation.getCurrentImage();

    return Particle::update();
}
