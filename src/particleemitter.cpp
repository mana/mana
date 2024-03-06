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
#include "resources/imageset.h"
#include "resources/resourcemanager.h"

#include "utils/stringutils.h"

#include <cmath>

#define SIN45 0.707106781f
#define DEG_RAD_FACTOR 0.017453293f

ParticleEmitter::ParticleEmitter(xmlNodePtr emitterNode, Particle *target,
                                 Map *map, int rotation,
                                 const std::string& dyePalettes):
    mOutputPauseLeft(0),
    mParticleImage(nullptr)
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

    for_each_xml_child_node(propertyNode, emitterNode)
    {
        if (xmlStrEqual(propertyNode->name, BAD_CAST "property"))
        {
            std::string name = XML::getProperty(propertyNode, "name", "");

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
                std::string image = XML::getProperty(propertyNode, "value", "");
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
                mParticleFollow = true;
            }
            else
            {
                logger->log("Particle Engine: Warning, unknown emitter property \"%s\"",
                            name.c_str()
                           );
            }
        }
        else if (xmlStrEqual(propertyNode->name, BAD_CAST "emitter"))
        {
            ParticleEmitter newEmitter(propertyNode, mParticleTarget, map,
                                       rotation, dyePalettes);
            mParticleChildEmitters.push_back(newEmitter);
        }
        else if (xmlStrEqual(propertyNode->name, BAD_CAST "rotation"))
        {
            ImageSet *imageset = ResourceManager::getInstance()->getImageSet(
                XML::getProperty(propertyNode, "imageset", ""),
                XML::getProperty(propertyNode, "width", 0),
                XML::getProperty(propertyNode, "height", 0)
            );

            // Get animation frames
            for_each_xml_child_node(frameNode, propertyNode)
            {
                int delay = XML::getProperty(frameNode, "delay", 0);
                int offsetX = XML::getProperty(frameNode, "offsetX", 0);
                int offsetY = XML::getProperty(frameNode, "offsetY", 0);
                if (mMap)
                {
                    offsetX -= imageset->getWidth() / 2
                               - mMap->getTileWidth() / 2;
                    offsetY -= imageset->getHeight() - mMap->getTileHeight();
                }

                if (xmlStrEqual(frameNode->name, BAD_CAST "frame"))
                {
                    int index = XML::getProperty(frameNode, "index", -1);

                    if (index < 0)
                    {
                        logger->log("No valid value for 'index'");
                        continue;
                    }

                    Image *img = imageset->get(index);

                    if (!img)
                    {
                        logger->log("No image at index %d", index);
                        continue;
                    }

                    mParticleRotation.addFrame(img, delay, offsetX, offsetY);
                }
                else if (xmlStrEqual(frameNode->name, BAD_CAST "sequence"))
                {
                    int start = XML::getProperty(frameNode, "start", -1);
                    int end = XML::getProperty(frameNode, "end", -1);

                    if (start < 0 || end < 0)
                    {
                        logger->log("No valid value for 'start' or 'end'");
                        continue;
                    }

                    while (end >= start)
                    {
                        Image *img = imageset->get(start);

                        if (!img)
                        {
                            logger->log("No image at index %d", start);
                            continue;
                        }

                        mParticleRotation.addFrame(img, delay, offsetX, offsetY);
                        start++;
                    }
                }
                else if (xmlStrEqual(frameNode->name, BAD_CAST "end"))
                {
                    mParticleRotation.addTerminator();
                }
            } // for frameNode
        }
        else if (xmlStrEqual(propertyNode->name, BAD_CAST "animation"))
        {
            std::string imagesetPath =
                    XML::getProperty(propertyNode, "imageset", "");
            ImageSet *imageset = ResourceManager::getInstance()->getImageSet(
                imagesetPath,
                XML::getProperty(propertyNode, "width", 0),
                XML::getProperty(propertyNode, "height", 0)
            );

            if (!imageset)
                logger->error(strprintf("Failed to load \"%s\"",
                                        imagesetPath.c_str()));

            // Get animation frames
            for_each_xml_child_node(frameNode, propertyNode)
            {
                int delay = XML::getProperty(frameNode, "delay", 0);
                int offsetX = XML::getProperty(frameNode, "offsetX", 0);
                int offsetY = XML::getProperty(frameNode, "offsetY", 0);
                offsetY -= imageset->getHeight() - 32;
                offsetX -= imageset->getWidth() / 2 - 16;

                if (xmlStrEqual(frameNode->name, BAD_CAST "frame"))
                {
                    int index = XML::getProperty(frameNode, "index", -1);

                    if (index < 0)
                    {
                        logger->log("No valid value for 'index'");
                        continue;
                    }

                    Image *img = imageset->get(index);

                    if (!img)
                    {
                        logger->log("No image at index %d", index);
                        continue;
                    }

                    mParticleAnimation.addFrame(img, delay, offsetX, offsetY);
                }
                else if (xmlStrEqual(frameNode->name, BAD_CAST "sequence"))
                {
                    int start = XML::getProperty(frameNode, "start", -1);
                    int end = XML::getProperty(frameNode, "end", -1);

                    if (start < 0 || end < 0)
                    {
                        logger->log("No valid value for 'start' or 'end'");
                        continue;
                    }

                    while (end >= start)
                    {
                        Image *img = imageset->get(start);

                        if (!img)
                        {
                            logger->log("No image at index %d", start);
                            continue;
                        }

                        mParticleAnimation.addFrame(img, delay, offsetX, offsetY);
                        start++;
                    }
                }
                else if (xmlStrEqual(frameNode->name, BAD_CAST "end"))
                {
                    mParticleAnimation.addTerminator();
                }
            } // for frameNode
        } else if (xmlStrEqual(propertyNode->name, BAD_CAST "deatheffect"))
        {
            mDeathEffect = (const char*)propertyNode->xmlChildrenNode->content;
            mDeathEffectConditions = 0x00;
            if (XML::getBoolProperty(propertyNode, "on-floor", true))
            {
                mDeathEffectConditions += Particle::DEAD_FLOOR;
            }
            if (XML::getBoolProperty(propertyNode, "on-sky", true))
            {
                mDeathEffectConditions += Particle::DEAD_SKY;
            }
            if (XML::getBoolProperty(propertyNode, "on-other", false))
            {
                mDeathEffectConditions += Particle::DEAD_OTHER;
            }
            if (XML::getBoolProperty(propertyNode, "on-impact", true))
            {
                mDeathEffectConditions += Particle::DEAD_IMPACT;
            }
            if (XML::getBoolProperty(propertyNode, "on-timeout", true))
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

    if (mParticleImage) mParticleImage->incRef();

    return *this;
}


ParticleEmitter::~ParticleEmitter()
{
    if (mParticleImage) mParticleImage->decRef();
}


template <typename T> ParticleEmitterProp<T>
ParticleEmitter::readParticleEmitterProp(xmlNodePtr propertyNode, T def)
{
    ParticleEmitterProp<T> retval;

    def = (T) XML::getFloatProperty(propertyNode, "value", (double) def);
    retval.set((T) XML::getFloatProperty(propertyNode, "min", (double) def),
               (T) XML::getFloatProperty(propertyNode, "max", (double) def));

    std::string change = XML::getProperty(propertyNode, "change-func", "none");
    T amplitude = (T) XML::getFloatProperty(propertyNode, "change-amplitude", 0.0);
    int period = XML::getProperty(propertyNode, "change-period", 0);
    int phase = XML::getProperty(propertyNode, "change-phase", 0);
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
