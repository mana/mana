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

#include "particle.h"

#include "animationparticle.h"
#include "configuration.h"
#include "imageparticle.h"
#include "particleengine.h"
#include "rotationalparticle.h"

#include "utils/dtor.h"
#include "utils/mathutils.h"

#include <cmath>

#define SIN45 0.707106781f
#define DEG_RAD_FACTOR 0.017453293f

class Graphics;

int Particle::particleCount = 0;
const float Particle::PARTICLE_SKY = 800.0f;

ParticleEmitter::ParticleEmitter(ResourceRef<ParticleEffectDef> effect,
                                 const ParticleEmitterDef &def,
                                 Particle *target,
                                 Map *map,
                                 int rotation)
    : mEffect(std::move(effect))
    , mDef(&def)
    , mTarget(target)
    , mMap(map)
    , mRotation(rotation)
    , mOutputPauseLeft(def.outputPause.value(0))
    , mPosX(def.posX)
    , mPosY(def.posY)
    , mOutput(def.output)
{
}

Particles ParticleEmitter::createParticles(int tick)
{
    Particles newParticles;

    if (mOutputPauseLeft > 0)
    {
        mOutputPauseLeft--;
        return newParticles;
    }
    mOutputPauseLeft = mDef->outputPause.value(tick);

    for (int i = mOutput.value(tick); i > 0; i--)
    {
        // Limit maximum particles
        if (Particle::particleCount > config.particleMaxCount) break;

        Particle *newParticle = Particle::create(mEffect, *mDef, mMap,
                                                 mRotation, mTarget);

        Vector position(mPosX.value(tick),
                        mPosY.value(tick),
                        mDef->posZ.value(tick));
        newParticle->moveTo(position);

        float angleH = mDef->angleHorizontal.value(tick) + mRotation * DEG_RAD_FACTOR;
        float angleV = mDef->angleVertical.value(tick);
        float power = mDef->power.value(tick);
        newParticle->setVelocity(
                cos(angleH) * cos(angleV) * power,
                sin(angleH) * cos(angleV) * power,
                sin(angleV) * power);

        newParticle->setRandomness(mDef->randomness.value(tick));
        newParticle->setGravity(mDef->gravity.value(tick));
        newParticle->setBounce(mDef->bounce.value(tick));
        newParticle->setFollow(mDef->follow);

        newParticle->setDestination(mTarget,
                                    mDef->acceleration.value(tick),
                                    mDef->momentum.value(tick));
        newParticle->setDieDistance(mDef->dieDistance.value(tick));

        newParticle->setLifetime(mDef->lifetime.value(tick));
        newParticle->setFadeOut(mDef->fadeOut.value(tick));
        newParticle->setFadeIn(mDef->fadeIn.value(tick));
        newParticle->setAlpha(mDef->alpha.value(tick));

        newParticles.push_back(newParticle);
    }

    return newParticles;
}

void ParticleEmitter::adjustSize(int w, int h)
{
    if (w == 0 || h == 0) return; // new dimensions are illegal

    // calculate the old rectangle
    int oldWidth = mPosX.maxVal - mPosX.minVal;
    int oldHeight = mPosY.maxVal - mPosY.minVal;
    int oldArea = oldWidth * oldHeight;

    // when the effect has no dimension it is not designed to be resizeable
    if (oldArea == 0)
        return;

    // set the new dimensions
    mPosX.set(0, w);
    mPosY.set(0, h);
    int newArea = w * h;
    // adjust the output so that the particle density stays the same
    float outputFactor = (float) newArea / oldArea;
    mOutput.minVal *= outputFactor;
    mOutput.maxVal *= outputFactor;
}

Particle::Particle(ResourceRef<ParticleEffectDef> effect)
    : mEffect(std::move(effect))
{
    Particle::particleCount++;
}

Particle::~Particle()
{
    // Delete child emitters and child particles
    clear();
    //update particle count
    Particle::particleCount--;
}

Particle *Particle::create(const ResourceRef<ParticleEffectDef> &effect,
                           const ParticleBaseDef &def,
                           Map *map,
                           int rotation,
                           Particle *target)
{
    Particle *particle;

    if (def.image)
        particle = new ImageParticle(effect, def.image);
    else if (def.rotation.getLength() > 0)
        particle = new RotationalParticle(effect, &def.rotation);
    else if (def.animation.getLength() > 0)
        particle = new AnimationParticle(effect, &def.animation);
    else
        particle = new Particle(effect);

    particle->setMap(map);
    particle->setDeathEffect(&def.deathEffect);

    if (!target)
        target = particle;

    for (const ParticleEmitterDef &emitterDef : def.emitters)
        particle->addEmitter(ParticleEmitter(effect, emitterDef, target, map, rotation));

    return particle;
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

            switch (config.particleFastPhysics)
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
        if ((mLifetimePast - 1) % (config.particleEmitterSkip + 1) == 0)
        {
            for (auto &childEmitter : mChildEmitters)
            {
                Particles newParticles = childEmitter.createParticles(mLifetimePast);
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
        if (mDeathEffect && (mAlive & mDeathEffect->conditions) > 0x00
            && !mDeathEffect->effect.empty())
        {
            if (Particle *deathEffect = particleEngine->addEffect(mDeathEffect->effect, 0, 0))
                deathEffect->moveBy(mPos);
        }
        mAlive = DEAD_LONG_AGO;
    }

    updateParticles(mChildParticles, mPos - oldPos);

    return isAlive() || !mChildParticles.empty() || !mAutoDelete;
}

void Particle::updateParticles(Particles &particles, const Vector &parentChange)
{
    for (auto p = particles.begin(); p != particles.end(); )
    {
        Particle *particle = *p;

        // Move particle with its parent if desired
        if (particle->doesFollow())
            particle->moveBy(parentChange);

        if (particle->update())
        {
            ++p;
        }
        else
        {
            delete particle;
            p = particles.erase(p);
        }
    }
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

void Particle::adjustEmitterSize(int w, int h)
{
    if (!mAllowSizeAdjust)
        return;

    for (auto &childEmitter : mChildEmitters)
        childEmitter.adjustSize(w, h);
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
    delete_all(mChildParticles);
    mChildParticles.clear();
}
