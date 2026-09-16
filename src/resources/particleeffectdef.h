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

#include "vector.h"

#include "resources/animation.h"
#include "resources/resource.h"

#include <cmath>
#include <string>
#include <vector>

class Image;

static const double PI = 3.14159265;

enum ChangeFunc
{
    FUNC_NONE,
    FUNC_SINE,
    FUNC_SAW,
    FUNC_TRIANGLE,
    FUNC_SQUARE
};

template <typename T> struct ParticleEmitterProp
{
    explicit ParticleEmitterProp(T value = T())
        : ParticleEmitterProp(value, value)
    {}

    ParticleEmitterProp(T min, T max)
        : minVal(min), maxVal(max)
    {}

    void set(T min, T max)
    {
        minVal = min;
        maxVal = max;
    }

    void setFunction(ChangeFunc func, T amplitude, int period, int phase)
    {
        changeFunc = func;
        changeAmplitude = amplitude;
        changePeriod = period;
        changePhase = phase;
    }

    T value(int tick) const
    {
        tick += changePhase;
        T val = (T) (minVal + (maxVal - minVal) * (rand() / ((double) RAND_MAX + 1)));

        switch (changeFunc)
        {
            case FUNC_SINE:
                val += (T) std::sin(PI * 2 * ((double)(tick % changePeriod) / (double)changePeriod)) * changeAmplitude;
                break;
            case FUNC_SAW:
                val += (T) (changeAmplitude * ((double)(tick % changePeriod) / (double)changePeriod)) * 2 - changeAmplitude;
                break;
            case FUNC_TRIANGLE:
                if ((tick % changePeriod) * 2 < changePeriod)
                {
                    val += changeAmplitude - (T)((tick % changePeriod) / (double)changePeriod) * changeAmplitude * 4;
                }
                else
                {
                    val += changeAmplitude * -3 + (T)((tick % changePeriod) / (double)changePeriod) * changeAmplitude * 4;
                    // I have no idea why this works but it does
                }
                break;
            case FUNC_SQUARE:
                if ((tick % changePeriod) * 2 < changePeriod)
                    val += changeAmplitude;
                else
                    val -= changeAmplitude;
                break;
            case FUNC_NONE:
            default:
                //nothing
                break;
        }

        return val;
    }

    T minVal;
    T maxVal;

    ChangeFunc changeFunc = FUNC_NONE;
    T changeAmplitude = T();
    int changePeriod = 0;
    int changePhase = 0;
};

/**
 * A particle effect to spawn when a particle dies, together with the death
 * conditions (a bitfield of Particle::AliveStatus) that trigger it.
 */
struct DeathEffect
{
    std::string effect;
    unsigned char conditions = 0;
};

struct ParticleEmitterDef;

/**
 * The part of a particle definition shared by the root particles of an
 * effect and by the particles spawned by emitters. See Particle::create for
 * how it is turned into a particle.
 */
struct ParticleBaseDef
{
    /*
     * Graphical representation of the particle. At most one of these is
     * expected to be set.
     */
    ResourceRef<Image> image;
    Animation animation;
    Animation rotation;     /**< One frame per direction of movement */

    DeathEffect deathEffect;

    /** Emitters the particle is equipped with. */
    std::vector<ParticleEmitterDef> emitters;
};

/**
 * The immutable description of a particle emitter, as read from XML. The
 * runtime state of an emitter lives in ParticleEmitter.
 */
struct ParticleEmitterDef : ParticleBaseDef
{
    /** Initial position of the particles. */
    ParticleEmitterProp<float> posX, posY, posZ;

    /** Initial direction of the particles, in radians. */
    ParticleEmitterProp<float> angleHorizontal, angleVertical;

    /** Initial velocity of the particles. */
    ParticleEmitterProp<float> power;

    /** Velocity changes of the particles. */
    ParticleEmitterProp<float> gravity;
    ParticleEmitterProp<int> randomness;
    ParticleEmitterProp<float> bounce;
    bool follow = false;

    /** Properties of targeting particles. */
    ParticleEmitterProp<float> acceleration;
    ParticleEmitterProp<float> dieDistance { -1.0f };
    ParticleEmitterProp<float> momentum { 1.0f };

    /** Behavior over time of the particles. */
    ParticleEmitterProp<int> lifetime { -1 };
    ParticleEmitterProp<int> fadeOut;
    ParticleEmitterProp<int> fadeIn;

    /** Number of particles spawned per update. */
    ParticleEmitterProp<int> output { 1 };

    /** Pause in frames between two spawns. */
    ParticleEmitterProp<int> outputPause;

    ParticleEmitterProp<float> alpha { 1.0f };
};

/**
 * The immutable description of a root particle of a particle effect.
 */
struct ParticleDef : ParticleBaseDef
{
    Vector offset;
    int lifetime = -1;
    bool sizeAdjustable = false;
};

/**
 * A particle effect definition, loaded from an XML file. Cached by the
 * ResourceManager and shared by all instances of the effect.
 */
class ParticleEffectDef : public Resource
{
    public:
        /**
         * Loads a particle effect from the given file. The file name may be
         * followed by a '|' and a set of dye palettes to apply to its images.
         */
        static ParticleEffectDef *load(const std::string &effectFile);

        const std::vector<ParticleDef> &particles() const
        { return mParticles; }

    private:
        ParticleEffectDef() = default;
        ~ParticleEffectDef() override = default;

        std::vector<ParticleDef> mParticles;
};
