/*
 *  The Mana World
 *  Copyright 2006 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifndef _PARTICLEEMITTER_H
#define _PARTICLEEMITTER_H

#include <list>

#include "utils/xml.h"
#include "utils/minmax.h"

#include "resources/animation.h"

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
        ParticleEmitter(xmlNodePtr emitterNode,  Particle *target, Map *map);

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
        std::list<Particle *> createParticles();

        /**
         * Sets the target of the particles that are created
         */
        void
        setTarget(Particle *target)
        { mParticleTarget = target; };

    private:
        template <typename T> MinMax<T> readMinMax(xmlNodePtr propertyNode, T def);

        /**
         * initial position of particles:
         */
        MinMax<float> mParticlePosX, mParticlePosY, mParticlePosZ;

        /**
         * initial vector of particles:
         */
        MinMax<float> mParticleAngleHorizontal, mParticleAngleVertical;

        /**
         * Initial velocity of particles
         */
        MinMax<float> mParticlePower;

        /*
         * Vector changing of particles:
         */
        MinMax<float> mParticleGravity;
        MinMax<int> mParticleRandomnes;
        MinMax<float> mParticleBounce;
        bool mParticleFollow;

        /*
         * Properties of targeting particles:
         */
        Particle *mParticleTarget;
        MinMax<float> mParticleAcceleration;
        MinMax<float> mParticleDieDistance;
        MinMax<float> mParticleMomentum;

        /*
         * Behavior over time of the particles:
         */
        MinMax<int> mParticleLifetime;
        MinMax<int> mParticleFadeOut;
        MinMax<int> mParticleFadeIn;

        Map *mMap;             /**< Map the particles are spawned on */

        MinMax<int> mOutput;   /**< Number of particles spawned per update */
        MinMax<int> mOutputSkip; /**< Pause in frames between two spawns */
        int mOutputSkipPause;

        /*
         * Graphical representation of the particle
         */
        Image *mParticleImage; /**< Particle image, if used */
        Animation mParticleAnimation; /**< Filename of particle animation file */
        MinMax<float> mParticleAlpha; /**< Opacity of the graphical representation of the particles */

        /** List of emitters the spawned particles are equipped with */
        std::list<ParticleEmitter> mParticleChildEmitters;
};
#endif
