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

#include "actor.h"
#include "vector.h"

#include "resources/particleeffectdef.h"

#include <list>
#include <string>

class Map;
class Particle;

using Particles = std::list<Particle *>;

/**
 * Every Particle can have one or more particle emitters that create new
 * particles when they are updated.
 *
 * An emitter instance only holds the state that differs between instances of
 * the same effect. Its properties are read from the shared ParticleEmitterDef,
 * which is kept alive by the reference to the ParticleEffectDef.
 */
class ParticleEmitter
{
    public:
        /**
         * @param effect   The effect definition the emitter definition is
         *                 part of, kept alive by this emitter.
         * @param def      The emitter definition.
         * @param target   The target of the particles that are created.
         * @param map      The map the particles are spawned on.
         * @param rotation Rotation of the effect in degrees.
         */
        ParticleEmitter(ResourceRef<ParticleEffectDef> effect,
                        const ParticleEmitterDef &def,
                        Particle *target,
                        Map *map,
                        int rotation = 0);

        /**
         * Spawns new particles
         * @return: a list of created particles
         */
        Particles createParticles(int tick);

        /**
         * Sets the target of the particles that are created
         */
        void setTarget(Particle *target)
        { mTarget = target; }

        /**
         * Changes the size of the emitter so that the effect fills a
         * rectangle of this size
         */
        void adjustSize(int w, int h);

    private:
        ResourceRef<ParticleEffectDef> mEffect;
        const ParticleEmitterDef *mDef;

        Particle *mTarget;      /**< The particle the spawned particles move towards */
        Map *mMap;              /**< Map the particles are spawned on */
        int mRotation;          /**< Rotation of the effect in degrees */
        int mOutputPauseLeft;   /**< Ticks left until the next spawn */

        /*
         * Copies of the properties that adjustSize can change, so that the
         * shared definition stays untouched.
         */
        ParticleEmitterProp<float> mPosX, mPosY;
        ParticleEmitterProp<int> mOutput;
};

/**
 * A particle spawned by a ParticleEmitter.
 */
class Particle : public Actor
{
    public:
        enum AliveStatus : unsigned char
        {
            ALIVE = 0,
            DEAD_TIMEOUT = 1,
            DEAD_FLOOR = 2,
            DEAD_SKY = 4,
            DEAD_IMPACT = 8,
            DEAD_OTHER = 16,
            DEAD_LONG_AGO = 128
        };
        static const float PARTICLE_SKY; /**< Maximum Z position of particles */
        static int particleCount;        /**< Current number of particles */

        /**
         * @param effect The effect definition this particle refers to, if
         *               any. Kept alive by the particle.
         */
        explicit Particle(ResourceRef<ParticleEffectDef> effect = nullptr);
        ~Particle() override;

        /**
         * Creates a particle from the shared part of its definition: the
         * kind of particle, its death effect and its emitters.
         *
         * @param effect   The effect definition \a def is part of.
         * @param def      The definition to instantiate.
         * @param map      The map the particle belongs to.
         * @param rotation Rotation of the effect in degrees.
         * @param target   The target of the particles spawned by the
         *                 emitters, or nullptr for the created particle.
         */
        static Particle *create(const ResourceRef<ParticleEffectDef> &effect,
                                const ParticleBaseDef &def,
                                Map *map,
                                int rotation,
                                Particle *target = nullptr);

        /**
         * Deletes all child particles and emitters.
         */
        void clear();

        /**
         * Updates particle position, returns false when the particle should
         * be deleted.
         */
        virtual bool update();

        /**
         * Updates the given particles. Particles that follow their parent are
         * first moved by the change in position of the parent. Particles that
         * are finished are deleted and removed from the list.
         */
        static void updateParticles(Particles &particles,
                                    const Vector &parentChange);

        /**
         * Draws the particle image.
         */
        bool draw(Graphics *graphics, int offsetX, int offsetY) const override;

        /**
         * Do not draw particles when behind other objects.
         */
        bool drawnWhenBehind() const override
        { return false; }

        /**
         * Adds an emitter to the particle.
         */
        void addEmitter(const ParticleEmitter &emitter)
        { mChildEmitters.push_back(emitter); }

        void addEmitter(ParticleEmitter &&emitter)
        { mChildEmitters.push_back(std::move(emitter)); }

        /**
         * Sets the position in 3 dimensional space in pixels relative to map.
         */
        void moveTo(const Vector &pos)
        { moveBy(pos - mPos); }

        /**
         * Sets the position in 2 dimensional space in pixels relative to map.
         */
        void moveTo(float x, float y);

        /**
         * Changes the particle position relative
         */
        void moveBy(const Vector &change);

        /**
         * Sets the time in game ticks until the particle is destroyed.
         */
        void setLifetime(int lifetime)
        { mLifetimeLeft = lifetime; mLifetimePast = 0; }

        /**
         * Sets the age of the pixel in game ticks where the particle has
         * faded in completely.
         */
        void setFadeOut(int fadeOut)
        { mFadeOut = fadeOut; }

        /**
         * Sets the remaining particle lifetime where the particle starts to
         * fade out.
         */
        void setFadeIn(int fadeIn)
        { mFadeIn = fadeIn; }

        /**
         * Sets the current velocity in 3 dimensional space.
         */
        void setVelocity(float x, float y, float z)
        { mVelocity.x = x; mVelocity.y = y; mVelocity.z = z; }

        /**
         * Sets the downward acceleration.
         */
        void setGravity(float gravity)
        { mGravity = gravity; }

        /**
         * Sets the ammount of random vector changes
         */
        void setRandomness(int r)
        { mRandomness = r; }

        /**
         * Sets the ammount of velocity particles retain after
         * hitting the ground.
         */
        void setBounce(float bouncieness)
        { mBounce = bouncieness; }

        /**
         * Sets the flag if the particle is supposed to be moved by its parent
         */
        void setFollow(bool follow)
        { mFollow = follow; }

        /**
         * Gets the flag if the particle is supposed to be moved by its parent
         */
        bool doesFollow() const
        { return mFollow; }

        /**
         * Makes the particle move toward another particle with a
         * given acceleration and momentum
         */
        void setDestination(Particle *target, float accel, float moment)
        { mTarget = target; mAcceleration = accel; mMomentum = moment; }

        /**
         * Sets the distance in pixel the particle can come near the target
         * particle before it is destroyed. Does only make sense after a target
         * particle has been set using setDestination.
         */
        void setDieDistance(float dist)
        { mInvDieDistance = 1.0f / dist; }

        /**
         * Changes the size of the emitters so that the effect fills a
         * rectangle of this size
         */
        void adjustEmitterSize(int w, int h);

        void setAllowSizeAdjust(bool adjust)
        { mAllowSizeAdjust = adjust; }

        bool isAlive() const
        { return mAlive == ALIVE; }

        /**
         * Determines whether the particle and its children are all dead
         */
        bool isExtinct() const
        { return !isAlive() && mChildParticles.empty(); }

        /**
         * Manually marks the particle for deletion.
         */
        void kill()
        { mAlive = DEAD_OTHER; mAutoDelete = true; }

        /**
         * After calling this function the particle will only request
         * deletion when kill() is called
         */
        void disableAutoDelete()
        { mAutoDelete = false; }

        float getAlpha() const override
        { return 1.0f; }

        void setAlpha(float alpha) override {}

        /**
         * Sets the effect to spawn when the particle dies. The death effect
         * needs to outlive the particle, which is the case when it is part of
         * the effect definition passed to the constructor.
         */
        void setDeathEffect(const DeathEffect *deathEffect)
        { mDeathEffect = deathEffect; }

    protected:
        /** Opacity of the graphical representation of the particle */
        float mAlpha = 1.0f;

        /** Calculates the current alpha transparency taking current fade status into account*/
        float getCurrentAlpha() const;

        int mLifetimeLeft = -1;         /**< Lifetime left in game ticks*/
        int mLifetimePast = 0;          /**< Age of the particle in game ticks*/
        int mFadeOut = 0;               /**< Lifetime in game ticks left where fading out begins*/
        int mFadeIn = 0;                /**< Age in game ticks where fading in is finished*/
        Vector mVelocity;               /**< Speed in pixels per game-tick. */

    private:
        AliveStatus mAlive = ALIVE;     /**< Is the particle supposed to be drawn and updated?*/
        // generic properties
        bool mAutoDelete = true;        /**< May the particle request its deletion by the parent particle? */
        std::list<ParticleEmitter> mChildEmitters;  /**< List of child emitters. */
        Particles mChildParticles;      /**< List of particles controlled by this particle */
        bool mAllowSizeAdjust = false;  /**< Can the effect size be adjusted by the object props in the map file? */
        ResourceRef<ParticleEffectDef> mEffect; /**< Effect definition this particle refers to */
        const DeathEffect *mDeathEffect = nullptr;  /**< Effect to be spawned when the particle dies */

        // dynamic particle
        float mGravity = 0.0f;          /**< Downward acceleration in pixels per game-tick. */
        int mRandomness = 0;            /**< Ammount of random vector change */
        float mBounce = 0.0f;           /**< How much the particle bounces off when hitting the ground */
        bool mFollow = false;           /**< is this particle moved when its parent particle moves? */

        // follow-point particles
        Particle *mTarget = nullptr;    /**< The particle that attracts this particle*/
        float mAcceleration = 0.0f;     /**< Acceleration towards the target particle in pixels per game-tick*/
        float mInvDieDistance = -1.0f;  /**< Distance in pixels from the target particle that causes the destruction of the particle*/
        float mMomentum = 1.0f;         /**< How much speed the particle retains after each game tick*/
};

/**
 * A handle on a particle. The handle prevents automatic deletion of the
 * particle by its parent and kills the particle when the handle is destroyed.
 */
class ParticleHandle
{
    public:
        explicit ParticleHandle(Particle *particle = nullptr):
            mParticle(particle)
        {
            if (mParticle)
                mParticle->disableAutoDelete();
        }

        ParticleHandle(const ParticleHandle &) = delete;

        ParticleHandle(ParticleHandle &&other):
            mParticle(other.mParticle)
        {
            other.mParticle = nullptr;
        }

        ~ParticleHandle()
        {
            if (mParticle)
                mParticle->kill();
        }

        ParticleHandle &operator=(const ParticleHandle &) = delete;

        ParticleHandle &operator=(ParticleHandle &&other)
        {
            if (this != &other)
            {
                if (mParticle)
                    mParticle->kill();
                mParticle = other.mParticle;
                other.mParticle = nullptr;
            }
            return *this;
        }

        Particle *operator->() const { return mParticle; }
        operator Particle *() const { return mParticle; }

    private:
        Particle *mParticle;
};
