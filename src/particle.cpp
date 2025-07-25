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

#include "particle.h"

#include "animationparticle.h"
#include "configuration.h"
#include "imageparticle.h"
#include "log.h"
#include "map.h"
#include "particleemitter.h"
#include "rotationalparticle.h"
#include "textparticle.h"

#include "resources/dye.h"
#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"
#include "utils/mathutils.h"
#include "utils/xml.h"

#include <guichan/color.hpp>

#include <cmath>

#define SIN45 0.707106781f

class Graphics;
class Image;

int Particle::particleCount = 0;
int Particle::maxCount = 0;
int Particle::fastPhysics = 0;
int Particle::emitterSkip = 1;
bool Particle::enabled = true;
const float Particle::PARTICLE_SKY = 800.0f;

Particle::Particle(Map *map)
{
    setMap(map);
    Particle::particleCount++;
}

Particle::~Particle()
{
    // Delete child emitters and child particles
    clear();
    //update particle count
    Particle::particleCount--;
}

void Particle::setupEngine()
{
    Particle::maxCount = config.particleMaxCount;
    Particle::fastPhysics = config.particleFastPhysics;
    Particle::emitterSkip = config.particleEmitterSkip + 1;
    Particle::enabled = config.particleEffects;
    Log::info("Particle engine set up");
}

bool Particle::draw(Graphics *, int, int) const
{
    return false;
}

bool Particle::update()
{
    if (!mMap)
        return false;

    if (mLifetimeLeft == 0 && mAlive == ALIVE)
        mAlive = DEAD_TIMEOUT;

    Vector oldPos = mPos;

    if (mAlive == ALIVE)
    {
        //calculate particle movement
        if (mMomentum != 1.0f)
        {
            mVelocity *= mMomentum;
        }

        if (mTarget && mAcceleration != 0.0f)
        {
            Vector dist = mPos - mTarget->getPosition();
            dist.x *= SIN45;
            float invHypotenuse;

            switch (Particle::fastPhysics)
            {
                case 1:
                    invHypotenuse = fastInvSqrt(
                        dist.x * dist.x + dist.y * dist.y + dist.z * dist.z);
                    break;
                case 2:
                    invHypotenuse = 2.0f /
                        fabs(dist.x) + fabs(dist.y) + fabs(dist.z);
                    break;
                default:
                    invHypotenuse = 1.0f / sqrt(
                        dist.x * dist.x + dist.y * dist.y + dist.z * dist.z);
                    break;
            }

            if (invHypotenuse)
            {
                if (mInvDieDistance > 0.0f && invHypotenuse > mInvDieDistance)
                {
                    mAlive = DEAD_IMPACT;
                }
                float accFactor = invHypotenuse * mAcceleration;
                mVelocity -= dist * accFactor;
            }
        }

        if (mRandomness > 0)
        {
            const int rand2 = mRandomness * 2;
            mVelocity.x += (rand() % rand2 - mRandomness) / 1000.0f;
            mVelocity.y += (rand() % rand2 - mRandomness) / 1000.0f;
            mVelocity.z += (rand() % rand2 - mRandomness) / 1000.0f;
        }

        mVelocity.z -= mGravity;

        // Update position
        mPos.x += mVelocity.x;
        mPos.y += mVelocity.y * SIN45;
        mPos.z += mVelocity.z * SIN45;

        // Update other stuff
        if (mLifetimeLeft > 0)
        {
            mLifetimeLeft--;
        }
        mLifetimePast++;

        if (mPos.z < 0.0f)
        {
            if (mBounce > 0.0f)
            {
                mPos.z *= -mBounce;
                mVelocity *= mBounce;
                mVelocity.z = -mVelocity.z;
            }
            else
            {
                mAlive = DEAD_FLOOR;
            }
        }
        else if (mPos.z > PARTICLE_SKY)
        {
                mAlive = DEAD_SKY;
        }

        // Update child emitters
        if ((mLifetimePast-1)%Particle::emitterSkip == 0)
        {
            for (auto &childEmitter : mChildEmitters)
            {
                Particles newParticles = childEmitter->createParticles(mLifetimePast);
                for (auto &newParticle : newParticles)
                {
                    newParticle->moveBy(mPos);
                    mChildParticles.push_back(newParticle);
                }
            }
        }
    }

    // create death effect when the particle died
    if (mAlive != ALIVE && mAlive != DEAD_LONG_AGO)
    {
        if ((mAlive & mDeathEffectConditions) > 0x00 && !mDeathEffect.empty())
        {
            Particle* deathEffect = particleEngine->addEffect(mDeathEffect, 0, 0);
            deathEffect->moveBy(mPos);
        }
        mAlive = DEAD_LONG_AGO;
    }

    Vector change = mPos - oldPos;

    // Update child particles

    for (auto p = mChildParticles.begin(); p != mChildParticles.end(); )
    {
        auto particle = *p;
        //move particle with its parent if desired
        if (particle->doesFollow())
        {
            particle->moveBy(change);
        }
        if (particle->update())
        {
            p++;
        }
        else
        {
            delete particle;
            p = mChildParticles.erase(p);
        }
    }

    return isAlive() || !mChildParticles.empty() || !mAutoDelete;
}

void Particle::moveBy(const Vector &change)
{
    mPos += change;

    for (auto &childParticle : mChildParticles)
        if (childParticle->doesFollow())
            childParticle->moveBy(change);
}

void Particle::moveTo(float x, float y)
{
    moveTo(Vector(x, y, mPos.z));
}

Particle *Particle::createChild()
{
    auto *newParticle = new Particle(mMap);
    mChildParticles.push_back(newParticle);
    return newParticle;
}

Particle *Particle::addEffect(const std::string &particleEffectFile,
                              int pixelX, int pixelY, int rotation)
{
    Particle *newParticle = nullptr;

    std::string::size_type pos = particleEffectFile.find('|');
    std::string dyePalettes;
    if (pos != std::string::npos)
        dyePalettes = particleEffectFile.substr(pos + 1);

    XML::Document doc(particleEffectFile.substr(0, pos));
    XML::Node rootNode = doc.rootNode();

    if (!rootNode || rootNode.name() != "effect")
    {
        Log::info("Error loading particle: %s", particleEffectFile.c_str());
        return nullptr;
    }

    ResourceManager *resman = ResourceManager::getInstance();

    // Parse particles
    for (auto effectChildNode : rootNode.children())
    {
        // We're only interested in particles
        if (effectChildNode.name() != "particle")
            continue;

        // Determine the exact particle type
        XML::Node node;

        // Animation
        if ((node = effectChildNode.findFirstChildByName("animation")))
        {
            newParticle = new AnimationParticle(mMap, node, dyePalettes);
        }
        // Rotational
        else if ((node = effectChildNode.findFirstChildByName("rotation")))
        {
            newParticle = new RotationalParticle(mMap, node, dyePalettes);
        }
        // Image
        else if ((node = effectChildNode.findFirstChildByName("image")))
        {
            std::string imageSrc { node.textContent() };
            if (!imageSrc.empty() && !dyePalettes.empty())
                Dye::instantiate(imageSrc, dyePalettes);

            auto img = resman->getImage(imageSrc);
            newParticle = new ImageParticle(mMap, img);
        }
        // Other
        else
        {
            newParticle = new Particle(mMap);
        }

        // Read and set the basic properties of the particle
        float offsetX = effectChildNode.getFloatProperty("position-x", 0);
        float offsetY = effectChildNode.getFloatProperty("position-y", 0);
        float offsetZ = effectChildNode.getFloatProperty("position-z", 0);
        Vector position(mPos.x + (float)pixelX + offsetX,
                        mPos.y + (float)pixelY + offsetY,
                        mPos.z + offsetZ);
        newParticle->moveTo(position);

        int lifetime = effectChildNode.getProperty("lifetime", -1);
        newParticle->setLifetime(lifetime);
        bool resizeable = "false" != effectChildNode.getProperty("size-adjustable", "false");
        newParticle->setAllowSizeAdjust(resizeable);

        // Look for additional emitters for this particle
        for (auto emitterNode : effectChildNode.children())
        {
            if (emitterNode.name() == "emitter")
            {
                ParticleEmitter *newEmitter;
                newEmitter = new ParticleEmitter(emitterNode, newParticle, mMap,
                                                 rotation, dyePalettes);
                newParticle->addEmitter(newEmitter);
            }
            else if (emitterNode.name() == "deatheffect")
            {
                std::string deathEffect { emitterNode.textContent() };
                char deathEffectConditions = 0x00;
                if (emitterNode.getBoolProperty("on-floor", true))
                {
                    deathEffectConditions += Particle::DEAD_FLOOR;
                }
                if (emitterNode.getBoolProperty("on-sky", true))
                {
                    deathEffectConditions += Particle::DEAD_SKY;
                }
                if (emitterNode.getBoolProperty("on-other", false))
                {
                    deathEffectConditions += Particle::DEAD_OTHER;
                }
                if (emitterNode.getBoolProperty("on-impact", true))
                {
                    deathEffectConditions += Particle::DEAD_IMPACT;
                }
                if (emitterNode.getBoolProperty("on-timeout", true))
                {
                    deathEffectConditions += Particle::DEAD_TIMEOUT;
                }
                newParticle->setDeathEffect(deathEffect, deathEffectConditions);
            }
        }

        mChildParticles.push_back(newParticle);
    }

    return newParticle;
}

Particle *Particle::addTextSplashEffect(const std::string &text, int x, int y,
                                        const gcn::Color *color,
                                        gcn::Font *font, bool outline)
{
    Particle *newParticle = new TextParticle(mMap, text, color, font, outline);
    newParticle->moveTo(x, y);
    newParticle->setVelocity(((rand() % 100) - 50) / 200.0f,    // X
                             ((rand() % 100) - 50) / 200.0f,    // Y
                             ((rand() % 100) / 200.0f) + 4.0f); // Z
    newParticle->setGravity(0.1f);
    newParticle->setBounce(0.5f);
    newParticle->setLifetime(200);
    newParticle->setFadeOut(100);

    mChildParticles.push_back(newParticle);

    return newParticle;
}

Particle *Particle::addTextRiseFadeOutEffect(const std::string &text,
                                             int x, int y,
                                             const gcn::Color *color,
                                             gcn::Font *font, bool outline)
{
    Particle *newParticle = new TextParticle(mMap, text, color, font, outline);
    newParticle->moveTo(x, y);
    newParticle->setVelocity(0.0f, 0.0f, 0.5f);
    newParticle->setGravity(0.0015f);
    newParticle->setLifetime(300);
    newParticle->setFadeOut(50);
    newParticle->setFadeIn(200);

    mChildParticles.push_back(newParticle);

    return newParticle;
}

void Particle::adjustEmitterSize(int w, int h)
{
    if (!mAllowSizeAdjust)
        return;

    for (auto &childEmitter : mChildEmitters)
        childEmitter->adjustSize(w, h);
}

float Particle::getCurrentAlpha() const
{
    float alpha = mAlpha;

    if (mLifetimeLeft > -1 && mLifetimeLeft < mFadeOut)
        alpha *= (float)mLifetimeLeft / (float)mFadeOut;

    if (mLifetimePast < mFadeIn)
        alpha *= (float)mLifetimePast / (float)mFadeIn;

    return alpha;
}

void Particle::clear()
{
    delete_all(mChildEmitters);
    mChildEmitters.clear();

    delete_all(mChildParticles);
    mChildParticles.clear();
}
