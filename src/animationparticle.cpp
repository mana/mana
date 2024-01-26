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

#include "graphics.h"
#include "simpleanimation.h"

AnimationParticle::AnimationParticle(Map *map, Animation *animation):
    ImageParticle(map, nullptr),
    mAnimation(new SimpleAnimation(animation))
{
}

AnimationParticle::AnimationParticle(Map *map, xmlNodePtr animationNode,
                                     const std::string& dyePalettes):
    ImageParticle(map, nullptr),
    mAnimation(new SimpleAnimation(animationNode, dyePalettes))
{
}

AnimationParticle::~AnimationParticle()
{
    delete mAnimation;
    mImage = nullptr;
}

bool AnimationParticle::update()
{
    mAnimation->update(10); // particle engine is updated every 10ms
    mImage = mAnimation->getCurrentImage();

    return Particle::update();
}
