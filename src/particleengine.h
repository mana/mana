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

#include "guichanfwd.h"
#include "particle.h"

#include <string>

class Map;

/**
 * Owns the top-level particles and spawns new particle effects.
 */
class ParticleEngine
{
    public:
        ~ParticleEngine();

        /**
         * Sets the map that spawned particles belong to.
         */
        void setMap(Map *map)
        { mMap = map; }

        /**
         * Advances all particles by one game tick.
         */
        void update();

        /**
         * Deletes all particles.
         */
        void clear();

        /**
         * Creates a blank particle. Useful for creating target particles.
         */
        Particle *createParticle();

        /**
         * Creates a particle effect described in the particleEffectFile.
         *
         * @return the last root particle of the effect, or nullptr when the
         *         effect could not be loaded.
         */
        Particle *addEffect(const std::string &particleEffectFile,
                            int pixelX, int pixelY, int rotation = 0);

        /**
         * Creates a standalone text particle that jumps up and falls down.
         */
        Particle *addTextSplashEffect(const std::string &text, int x, int y,
                const gcn::Color *color, gcn::Font *font,
                bool outline = false);

        /**
         * Creates a standalone text particle that rises while fading out.
         */
        Particle *addTextRiseFadeOutEffect(const std::string &text,
                int x, int y, const gcn::Color *color, gcn::Font *font,
                bool outline = false);

    private:
        Map *mMap = nullptr;    /**< Map the particles are spawned on */
        Particles mParticles;   /**< Top-level particles */
};

extern ParticleEngine *particleEngine;
