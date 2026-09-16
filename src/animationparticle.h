/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "imageparticle.h"
#include "simpleanimation.h"

#include "resources/particleeffectdef.h"

class Map;

class AnimationParticle : public ImageParticle
{
    public:
        /**
         * @param effect    The effect definition the animation is part of.
         * @param animation The animation to play, owned by the effect.
         */
        AnimationParticle(ResourceRef<ParticleEffectDef> effect,
                          const Animation *animation);

        ~AnimationParticle() override;

        bool update() override;

    private:
        SimpleAnimation mAnimation; /**< Used animation for this particle */
};
