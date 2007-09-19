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

#include "particleemitter.h"

#include "animationparticle.h"
#include "imageparticle.h"
#include "log.h"
#include "particle.h"

#include "resources/animation.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"
#include "resources/imageset.h"

#include <cmath>

#define SIN45 0.707106781f
#define DEG_RAD_FACTOR 0.017453293f

ParticleEmitter::ParticleEmitter(xmlNodePtr emitterNode, Particle *target, Map *map):
    mParticleImage(0)
{
    mMap = map;
    mParticleTarget = target;

    //initializing default values
    mParticlePosX.set(0.0f);
    mParticlePosY.set(0.0f);
    mParticlePosZ.set(0.0f);
    mParticleAngleHorizontal.set(0.0f);
    mParticleAngleVertical.set(0.0f);
    mParticlePower.set(0.0f);
    mParticleGravity.set(0.0f);
    mParticleRandomnes.set(0);
    mParticleBounce.set(0.0f);
    mParticleAcceleration.set(0.0f);
    mParticleDieDistance.set(-1.0f);
    mParticleMomentum.set(1.0f);
    mParticleLifetime.set(-1);
    mParticleFadeOut.set(0);
    mParticleFadeIn.set(0);
    mOutput.set(1);

    for_each_xml_child_node(propertyNode, emitterNode)
    {
        if (xmlStrEqual(propertyNode->name, BAD_CAST "property"))
        {
            std::string name = XML::getProperty(propertyNode, "name", "");

            if (name == "position-x")
            {
                mParticlePosX = readMinMax(propertyNode, 0.0f);
            }
            else if (name == "position-y")
            {

                mParticlePosY = readMinMax(propertyNode, 0.0f);
                mParticlePosY.minVal *= SIN45;
                mParticlePosY.maxVal *= SIN45;
             }
            else if (name == "position-z")
            {
                mParticlePosZ = readMinMax(propertyNode, 0.0f);
                mParticlePosZ.minVal *= SIN45;
                mParticlePosZ.maxVal *= SIN45;
            }
            else if (name == "image")
            {
                std::string image = XML::getProperty(propertyNode, "value", "");
                // Don't leak when multiple images are defined
                if (image != "" && !mParticleImage)
                {
                    ResourceManager *resman = ResourceManager::getInstance();
                    mParticleImage = resman->getImage(image);
                }
            }
            else if (name == "horizontal-angle")
            {
                mParticleAngleHorizontal = readMinMax(propertyNode, 0.0f);
                mParticleAngleHorizontal.minVal *= DEG_RAD_FACTOR;
                mParticleAngleHorizontal.maxVal *= DEG_RAD_FACTOR;
            }
            else if (name == "vertical-angle")
            {
                mParticleAngleVertical = readMinMax(propertyNode, 0.0f);
                mParticleAngleVertical.minVal *= DEG_RAD_FACTOR;
                mParticleAngleVertical.maxVal *= DEG_RAD_FACTOR;
            }
            else if (name == "power")
            {
                mParticlePower = readMinMax(propertyNode, 0.0f);
            }
            else if (name == "gravity")
            {
                mParticleGravity = readMinMax(propertyNode, 0.0f);
            }
            else if (name == "randomnes")
            {
                mParticleRandomnes = readMinMax(propertyNode, 0);
            }
            else if (name == "bounce")
            {
                mParticleBounce = readMinMax(propertyNode, 0.0f);
            }
            else if (name == "lifetime")
            {
                mParticleLifetime = readMinMax(propertyNode, 0);
                mParticleLifetime.minVal += 1;
            }
            else if (name == "output")
            {
                mOutput = readMinMax(propertyNode, 0);
                mOutput.maxVal +=1;
            }
            else if (name == "acceleration")
            {
                mParticleAcceleration = readMinMax(propertyNode, 0.0f);
            }
            else if (name == "die-distance")
            {
                mParticleDieDistance = readMinMax(propertyNode, 0.0f);
            }
            else if (name == "momentum")
            {
                mParticleMomentum = readMinMax(propertyNode, 1.0f);
            }
            else if (name == "fade-out")
            {
                mParticleFadeOut = readMinMax(propertyNode, 0);
            }
            else if (name == "fade-in")
            {
                mParticleFadeIn = readMinMax(propertyNode, 0);
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
            ParticleEmitter newEmitter(propertyNode, mParticleTarget, map);
            mParticleChildEmitters.push_back(newEmitter);
        }
        else if (xmlStrEqual(propertyNode->name, BAD_CAST "animation"))
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
                        logger->log("No image at index " + (index));
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
                            logger->log("No image at index " +
                                    (start));
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
        }
    }
}


ParticleEmitter::~ParticleEmitter()
{
}


template <typename T> MinMax<T>
ParticleEmitter::readMinMax(xmlNodePtr propertyNode, T def)
{
    MinMax<T> retval;

    def = (T) XML::getFloatProperty(propertyNode, "value", (double) def);
    retval.set((T) XML::getFloatProperty(propertyNode, "min", (double) def),
               (T) XML::getFloatProperty(propertyNode, "max", (double) def));

    return retval;
}


std::list<Particle *>
ParticleEmitter::createParticles()
{
    std::list<Particle *> newParticles;

    for (int i = mOutput.value(); i > 0; i--)
    {
        // Limit maximum particles
        if (Particle::particleCount > Particle::maxCount) break;

        Particle *newParticle;
        if (mParticleImage)
        {
            newParticle = new ImageParticle(mMap, mParticleImage);
        }
        else if (mParticleAnimation.getLength() > 0)
        {
            Animation *newAnimation = new Animation(mParticleAnimation);
            newParticle = new AnimationParticle(mMap, newAnimation);
        }
        else
        {
            newParticle = new Particle(mMap);
        }


        newParticle->setPosition(
                mParticlePosX.value(),
                mParticlePosY.value(),
                mParticlePosZ.value());

        float angleH = mParticleAngleHorizontal.value();
        float angleV = mParticleAngleVertical.value();
        float power = mParticlePower.value();
        newParticle->setVelocity(
                cos(angleH) * cos(angleV) * power,
                sin(angleH) * cos(angleV) * power,
                sin(angleV) * power);

        newParticle->setRandomnes(mParticleRandomnes.value());
        newParticle->setGravity(mParticleGravity.value());
        newParticle->setBounce(mParticleBounce.value());

        newParticle->setDestination(mParticleTarget,
                                    mParticleAcceleration.value(),
                                    mParticleMomentum.value()
                                   );
        newParticle->setDieDistance(mParticleDieDistance.value());

        newParticle->setLifetime(mParticleLifetime.value());
        newParticle->setFadeOut(mParticleFadeOut.value());
        newParticle->setFadeIn(mParticleFadeIn.value());

        for (std::list<ParticleEmitter>::iterator i = mParticleChildEmitters.begin();
             i != mParticleChildEmitters.end();
             i++)
        {
            newParticle->addEmitter(new ParticleEmitter(*i));
        }

        newParticles.push_back(newParticle);
    }

    return newParticles;
}
