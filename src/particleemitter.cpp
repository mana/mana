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
#include "imageparticle.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "particleemitter.h"
#include "rotationalparticle.h"

#include "resources/dye.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include <cmath>

#define SIN45 0.707106781f
#define DEG_RAD_FACTOR 0.017453293f

ParticleEmitter::ParticleEmitter(XML::Node emitterNode, Particle *target,
                                 Map *map, int rotation,
                                 const std::string &dyePalettes)
{
    mMap = map;
    mParticleTarget = target;

    // Initializing default values
    mParticlePosX.set(0.0f);
    mParticlePosY.set(0.0f);
    mParticlePosZ.set(0.0f);
    mParticleAngleHorizontal.set(0.0f);
    mParticleAngleVertical.set(0.0f);
    mParticlePower.set(0.0f);
    mParticleGravity.set(0.0f);
    mParticleRandomness.set(0);
    mParticleBounce.set(0.0f);
    mParticleFollow = false;
    mParticleAcceleration.set(0.0f);
    mParticleDieDistance.set(-1.0f);
    mParticleMomentum.set(1.0f);
    mParticleLifetime.set(-1);
    mParticleFadeOut.set(0);
    mParticleFadeIn.set(0);
    mOutput.set(1);
    mOutputPause.set(0);
    mParticleAlpha.set(1.0f);

    for (auto propertyNode : emitterNode.children())
    {
        if (propertyNode.name() == "property")
        {
            std::string name = propertyNode.getProperty("name", "");

            if (name == "position-x")
            {
                mParticlePosX = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "position-y")
            {

                mParticlePosY = readParticleEmitterProp(propertyNode, 0.0f);
                mParticlePosY.minVal *= SIN45;
                mParticlePosY.maxVal *= SIN45;
                mParticlePosY.changeAmplitude *= SIN45;
             }
            else if (name == "position-z")
            {
                mParticlePosZ = readParticleEmitterProp(propertyNode, 0.0f);
                mParticlePosZ.minVal *= SIN45;
                mParticlePosZ.maxVal *= SIN45;
                mParticlePosZ.changeAmplitude *= SIN45;
            }
            else if (name == "image")
            {
                std::string image = propertyNode.getProperty("value", "");
                // Don't leak when multiple images are defined
                if (!image.empty() && !mParticleImage)
                {
                    if (!dyePalettes.empty())
                        Dye::instantiate(image, dyePalettes);

                    ResourceManager *resman = ResourceManager::getInstance();
                    mParticleImage = resman->getImage(image);
                }
            }
            else if (name == "horizontal-angle")
            {
                mParticleAngleHorizontal = readParticleEmitterProp(propertyNode, 0.0f);
                mParticleAngleHorizontal.minVal += rotation;
                mParticleAngleHorizontal.minVal *= DEG_RAD_FACTOR;
                mParticleAngleHorizontal.maxVal += rotation;
                mParticleAngleHorizontal.maxVal *= DEG_RAD_FACTOR;
                mParticleAngleHorizontal.changeAmplitude *= DEG_RAD_FACTOR;
            }
            else if (name == "vertical-angle")
            {
                mParticleAngleVertical = readParticleEmitterProp(propertyNode, 0.0f);
                mParticleAngleVertical.minVal *= DEG_RAD_FACTOR;
                mParticleAngleVertical.maxVal *= DEG_RAD_FACTOR;
                mParticleAngleVertical.changeAmplitude *= DEG_RAD_FACTOR;
            }
            else if (name == "power")
            {
                mParticlePower = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "gravity")
            {
                mParticleGravity = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "randomnes" || name == "randomness") // legacy bug
            {
                mParticleRandomness = readParticleEmitterProp(propertyNode, 0);
            }
            else if (name == "bounce")
            {
                mParticleBounce = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "lifetime")
            {
                mParticleLifetime = readParticleEmitterProp(propertyNode, 0);
                mParticleLifetime.minVal += 1;
            }
            else if (name == "output")
            {
                mOutput = readParticleEmitterProp(propertyNode, 0);
                mOutput.maxVal +=1;
            }
            else if (name == "output-pause")
            {
                mOutputPause = readParticleEmitterProp(propertyNode, 0);
                mOutputPauseLeft = mOutputPause.value(0);
            }
            else if (name == "acceleration")
            {
                mParticleAcceleration = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "die-distance")
            {
                mParticleDieDistance = readParticleEmitterProp(propertyNode, 0.0f);
            }
            else if (name == "momentum")
            {
                mParticleMomentum = readParticleEmitterProp(propertyNode, 1.0f);
            }
            else if (name == "fade-out")
            {
                mParticleFadeOut = readParticleEmitterProp(propertyNode, 0);
            }
            else if (name == "fade-in")
            {
                mParticleFadeIn = readParticleEmitterProp(propertyNode, 0);
            }
            else if (name == "alpha")
            {
                mParticleAlpha = readParticleEmitterProp(propertyNode, 1.0f);
            }
            else if (name == "follow-parent")
            {
                mParticleFollow = propertyNode.getBoolProperty("value", true);
            }
            else
            {
                Log::info("Particle Engine: Warning, unknown emitter property \"%s\"",
                          name.c_str());
            }
        }
        else if (propertyNode.name() == "emitter")
        {
            ParticleEmitter newEmitter(propertyNode, mParticleTarget, map,
                                       rotation, dyePalettes);
            mParticleChildEmitters.push_back(newEmitter);
        }
        else if (propertyNode.name() == "rotation")
        {
            mParticleRotation = Animation::fromXML(propertyNode);
        }
        else if (propertyNode.name() == "animation")
        {
            mParticleAnimation = Animation::fromXML(propertyNode);
        }
        else if (propertyNode.name() == "deatheffect")
        {
            mDeathEffect = propertyNode.textContent();
            mDeathEffectConditions = 0x00;
            if (propertyNode.getBoolProperty("on-floor", true))
            {
                mDeathEffectConditions += Particle::DEAD_FLOOR;
            }
            if (propertyNode.getBoolProperty("on-sky", true))
            {
                mDeathEffectConditions += Particle::DEAD_SKY;
            }
            if (propertyNode.getBoolProperty("on-other", false))
            {
                mDeathEffectConditions += Particle::DEAD_OTHER;
            }
            if (propertyNode.getBoolProperty("on-impact", true))
            {
                mDeathEffectConditions += Particle::DEAD_IMPACT;
            }
            if (propertyNode.getBoolProperty("on-timeout", true))
            {
                mDeathEffectConditions += Particle::DEAD_TIMEOUT;
            }
        }
    }
}

ParticleEmitter::ParticleEmitter(const ParticleEmitter &o)
{
    *this = o;
}

ParticleEmitter & ParticleEmitter::operator=(const ParticleEmitter &o)
{
    mParticlePosX = o.mParticlePosX;
    mParticlePosY = o.mParticlePosY;
    mParticlePosZ = o.mParticlePosZ;
    mParticleAngleHorizontal = o.mParticleAngleHorizontal;
    mParticleAngleVertical = o.mParticleAngleVertical;
    mParticlePower = o.mParticlePower;
    mParticleGravity = o.mParticleGravity;
    mParticleRandomness = o.mParticleRandomness;
    mParticleBounce = o.mParticleBounce;
    mParticleFollow = o.mParticleFollow;
    mParticleTarget = o.mParticleTarget;
    mParticleAcceleration = o.mParticleAcceleration;
    mParticleDieDistance = o.mParticleDieDistance;
    mParticleMomentum = o.mParticleMomentum;
    mParticleLifetime = o.mParticleLifetime;
    mParticleFadeOut = o.mParticleFadeOut;
    mParticleFadeIn = o.mParticleFadeIn;
    mParticleAlpha = o.mParticleAlpha;
    mMap = o.mMap;
    mOutput = o.mOutput;
    mOutputPause = o.mOutputPause;
    mParticleImage = o.mParticleImage;
    mParticleAnimation = o.mParticleAnimation;
    mParticleRotation = o.mParticleRotation;
    mParticleChildEmitters = o.mParticleChildEmitters;

    mOutputPauseLeft = 0;

    return *this;
}


ParticleEmitter::~ParticleEmitter() = default;


template <typename T> ParticleEmitterProp<T>
ParticleEmitter::readParticleEmitterProp(XML::Node propertyNode, T def)
{
    ParticleEmitterProp<T> retval;

    def = (T) propertyNode.getFloatProperty("value", (double) def);
    retval.set((T) propertyNode.getFloatProperty("min", (double) def),
        (T) propertyNode.getFloatProperty("max", (double) def));

    std::string change = propertyNode.getProperty("change-func", "none");
    T amplitude = (T) propertyNode.getFloatProperty("change-amplitude", 0.0);
    int period = propertyNode.getProperty("change-period", 0);
    int phase = propertyNode.getProperty("change-phase", 0);
    if (change == "saw" || change == "sawtooth")
        retval.setFunction(FUNC_SAW, amplitude, period, phase);
    else if (change == "sine" || change == "sinewave")
        retval.setFunction(FUNC_SINE, amplitude, period, phase);
    else if (change == "triangle")
        retval.setFunction(FUNC_TRIANGLE, amplitude, period, phase);
    else if (change == "square")
        retval.setFunction(FUNC_SQUARE, amplitude, period, phase);

    return retval;
}


std::list<Particle *> ParticleEmitter::createParticles(int tick)
{
    std::list<Particle *> newParticles;

    if (mOutputPauseLeft > 0)
    {
        mOutputPauseLeft--;
        return newParticles;
    }
    mOutputPauseLeft = mOutputPause.value(tick);

    for (int i = mOutput.value(tick); i > 0; i--)
    {
        // Limit maximum particles
        if (Particle::particleCount > Particle::maxCount) break;

        Particle *newParticle;
        if (mParticleImage)
        {
            newParticle = new ImageParticle(mMap, mParticleImage);
        }
        else if (mParticleRotation.getLength() > 0)
        {
            newParticle = new RotationalParticle(mMap, mParticleRotation);
        }
        else if (mParticleAnimation.getLength() > 0)
        {
            newParticle = new AnimationParticle(mMap, mParticleAnimation);
        }
        else
        {
            newParticle = new Particle(mMap);
        }

        Vector position(mParticlePosX.value(tick),
                        mParticlePosY.value(tick),
                        mParticlePosZ.value(tick));
        newParticle->moveTo(position);

        float angleH = mParticleAngleHorizontal.value(tick);
        float angleV = mParticleAngleVertical.value(tick);
        float power = mParticlePower.value(tick);
        newParticle->setVelocity(
                cos(angleH) * cos(angleV) * power,
                sin(angleH) * cos(angleV) * power,
                sin(angleV) * power);

        newParticle->setRandomness(mParticleRandomness.value(tick));
        newParticle->setGravity(mParticleGravity.value(tick));
        newParticle->setBounce(mParticleBounce.value(tick));
        newParticle->setFollow(mParticleFollow);

        newParticle->setDestination(mParticleTarget,
                                    mParticleAcceleration.value(tick),
                                    mParticleMomentum.value(tick)
                                   );
        newParticle->setDieDistance(mParticleDieDistance.value(tick));

        newParticle->setLifetime(mParticleLifetime.value(tick));
        newParticle->setFadeOut(mParticleFadeOut.value(tick));
        newParticle->setFadeIn(mParticleFadeIn.value(tick));
        newParticle->setAlpha(mParticleAlpha.value(tick));

        for (auto &particleChildEmitter : mParticleChildEmitters)
        {
            newParticle->addEmitter(new ParticleEmitter(particleChildEmitter));
        }

        if (!mDeathEffect.empty())
        {
            newParticle->setDeathEffect(mDeathEffect, mDeathEffectConditions);
        }

        newParticles.push_back(newParticle);
    }

    return newParticles;
}

void ParticleEmitter::adjustSize(int w, int h)
{
    if (w == 0 || h == 0) return; // new dimensions are illegal

    // calculate the old rectangle
    int oldWidth = mParticlePosX.maxVal - mParticlePosX.minVal;
    int oldHeight = mParticlePosX.maxVal - mParticlePosY.minVal;
    int oldArea = oldWidth * oldHeight;
    if (oldArea == 0)
    {
        //when the effect has no dimension it is
        //not designed to be resizeable
        return;
    }

    // set the new dimensions
    mParticlePosX.set(0, w);
    mParticlePosY.set(0, h);
    int newArea = w * h;
    // adjust the output so that the particle density stays the same
    float outputFactor = (float)newArea / (float)oldArea;
    mOutput.minVal *= outputFactor;
    mOutput.maxVal *= outputFactor;
}
