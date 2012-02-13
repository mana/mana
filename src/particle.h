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

#ifndef PARTICLE_H
#define PARTICLE_H

#include "actor.h"
#include "guichanfwd.h"
#include "vector.h"

#include <list>
#include <string>

class Map;
class Particle;
class ParticleEmitter;

typedef std::list<Particle *> Particles;
typedef Particles::iterator ParticleIterator;
typedef std::list<ParticleEmitter *> Emitters;
typedef Emitters::iterator EmitterIterator;

/**
 * A particle spawned by a ParticleEmitter.
 */
class Particle : public Actor
{
    public:
        enum AliveStatus
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
        static int fastPhysics;          /**< Mode of squareroot calculation */
        static int particleCount;        /**< Current number of particles */
        static int maxCount;             /**< Maximum number of particles */
        static int emitterSkip;          /**< Duration of pause between two emitter updates in ticks */
        static bool enabled;   /**< true when non-crucial particle effects are disabled */

        /**
         * Constructor.
         *
         * @param map the map this particle will add itself to, may be NULL
         */
        Particle(Map *map);

        ~Particle();

        /**
         * Deletes all child particles and emitters.
         */
        void clear();

        /**
         * Gives a particle the properties of an engine root particle and loads
         * the particle-related config settings.
         */
        void setupEngine();

        /**
         * Updates particle position, returns false when the particle should
         * be deleted.
         */
        virtual bool update();

        /**
         * Draws the particle image.
         */
        virtual bool draw(Graphics *graphics, int offsetX, int offsetY) const;

        /**
         * Do not draw particles when beind other objects
         */
        virtual bool drawnWhenBehind() const
        { return false; }

        /**
         * Creates a blank particle as a child of the current particle
         * Useful for creating target particles
         */
        Particle *createChild();

        /**
         * Creates a child particle that hosts some emitters described in the
         * particleEffectFile.
         */
        Particle *addEffect(const std::string &particleEffectFile,
                            int pixelX, int pixelY, int rotation = 0);

        /**
         * Creates a standalone text particle.
         */
        Particle *addTextSplashEffect(const std::string &text, int x, int y,
                const gcn::Color *color, gcn::Font *font,
                bool outline = false);

        /**
         * Creates a standalone text particle.
         */
        Particle *addTextRiseFadeOutEffect(const std::string &text,
                int x, int y, const gcn::Color *color, gcn::Font *font,
                bool outline = false);

        /**
         * Adds an emitter to the particle.
         */
        void addEmitter (ParticleEmitter* emitter)
        { mChildEmitters.push_back(emitter); }

        /**
         * Sets the position in 3 dimensional space in pixels relative to map.
         */
        void moveTo(const Vector &pos)
        { moveBy (pos - mPos);}

        /**
         * Sets the position in 2 dimensional space in pixels relative to map.
         */
        void moveTo(float x, float y);

        /**
         * Changes the particle position relative
         */
        void moveBy (const Vector &change);

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
        bool doesFollow()
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

        /** We consider particles (at least for now) to be one layer-sprites */
        virtual int getNumberOfLayers() const
        { return 1; }

        virtual float getAlpha() const
        { return 1.0f; }

        virtual void setAlpha(float alpha) {}

        virtual void setDeathEffect(const std::string &effectFile, char conditions)
        { mDeathEffect = effectFile; mDeathEffectConditions = conditions; }

    protected:
        /** Opacity of the graphical representation of the particle */
        float mAlpha;

        /** Calculates the current alpha transparency taking current fade status into account*/
        float getCurrentAlpha() const;

        int mLifetimeLeft;          /**< Lifetime left in game ticks*/
        int mLifetimePast;          /**< Age of the particle in game ticks*/
        int mFadeOut;               /**< Lifetime in game ticks left where fading out begins*/
        int mFadeIn;                /**< Age in game ticks where fading in is finished*/
        Vector mVelocity;           /**< Speed in pixels per game-tick. */

    private:
        AliveStatus mAlive;                 /**< Is the particle supposed to be drawn and updated?*/
        // generic properties
        bool mAutoDelete;           /**< May the particle request its deletion by the parent particle? */
        Emitters mChildEmitters;    /**< List of child emitters. */
        Particles mChildParticles;  /**< List of particles controlled by this particle */
        bool mAllowSizeAdjust;      /**< Can the effect size be adjusted by the object props in the map file? */
        std::string mDeathEffect;   /**< Particle effect file to be spawned when the particle dies */
        char mDeathEffectConditions;/**< Bitfield of death conditions which trigger spawning of the death particle */

        // dynamic particle
        float mGravity;             /**< Downward acceleration in pixels per game-tick. */
        int mRandomness;            /**< Ammount of random vector change */
        float mBounce;              /**< How much the particle bounces off when hitting the ground */
        bool mFollow;               /**< is this particle moved when its parent particle moves? */

        // follow-point particles
        Particle *mTarget;          /**< The particle that attracts this particle*/
        float mAcceleration;        /**< Acceleration towards the target particle in pixels per game-tick*/
        float mInvDieDistance;      /**< Distance in pixels from the target particle that causes the destruction of the particle*/
        float mMomentum;            /**< How much speed the particle retains after each game tick*/
};

extern Particle *particleEngine;

#endif
