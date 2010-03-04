/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H

#include "particleemitterprop.h"

#include "resources/animation.h"

#include "utils/xml.h"

#include <list>

class Image;
class Map;
class Particle;

/**
 * Every Particle can have one or more particle emitters that create new
 * particles when they are updated
 */
class ParticleEmitter
{
    public:
        /**
         * Constructor.
         */
        ParticleEmitter(xmlNodePtr emitterNode,  Particle *target, Map *map, int rotation = 0);

        /**
         * Copy Constructor (necessary for reference counting of particle images)
         */
        ParticleEmitter(const ParticleEmitter &o);

        /**
         * Assignment operator that calls the copy constructor
         */
        ParticleEmitter & operator=(const ParticleEmitter &o);

        /**
         * Destructor.
         */
        ~ParticleEmitter();

        /**
         * Spawns new particles
         * @return: a list of created particles
         */
        std::list<Particle *> createParticles(int tick);

        /**
         * Sets the target of the particles that are created
         */
        void setTarget(Particle *target)
        { mParticleTarget = target; };

        /**
         * Changes the size of the emitter so that the effect fills a
         * rectangle of this size
         */
        void adjustSize(int w, int h);

    private:
        template <typename T> ParticleEmitterProp<T> readParticleEmitterProp(xmlNodePtr propertyNode, T def);

        /**
         * initial position of particles:
         */
        ParticleEmitterProp<float> mParticlePosX, mParticlePosY, mParticlePosZ;

        /**
         * initial vector of particles:
         */
        ParticleEmitterProp<float> mParticleAngleHorizontal, mParticleAngleVertical;

        /**
         * Initial velocity of particles
         */
        ParticleEmitterProp<float> mParticlePower;

        /*
         * Vector changing of particles:
         */
        ParticleEmitterProp<float> mParticleGravity;
        ParticleEmitterProp<int> mParticleRandomness;
        ParticleEmitterProp<float> mParticleBounce;
        bool mParticleFollow;

        /*
         * Properties of targeting particles:
         */
        Particle *mParticleTarget;
        ParticleEmitterProp<float> mParticleAcceleration;
        ParticleEmitterProp<float> mParticleDieDistance;
        ParticleEmitterProp<float> mParticleMomentum;

        /*
         * Behavior over time of the particles:
         */
        ParticleEmitterProp<int> mParticleLifetime;
        ParticleEmitterProp<int> mParticleFadeOut;
        ParticleEmitterProp<int> mParticleFadeIn;

        Map *mMap;             /**< Map the particles are spawned on */

        ParticleEmitterProp<int> mOutput;   /**< Number of particles spawned per update */
        ParticleEmitterProp<int> mOutputPause; /**< Pause in frames between two spawns */
        int mOutputPauseLeft;

        /*
         * Graphical representation of the particle
         */
        Image *mParticleImage; /**< Particle image, if used */
        Animation mParticleAnimation; /**< Filename of particle animation file */
        Animation mParticleRotation; /**< Filename of particle rotation file */
        ParticleEmitterProp<float> mParticleAlpha; /**< Opacity of the graphical representation of the particles */

        /** List of emitters the spawned particles are equipped with */
        std::list<ParticleEmitter> mParticleChildEmitters;
};
#endif
