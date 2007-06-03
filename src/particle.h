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

#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <list>
#include <string>

#include <guichan/color.hpp>

#include "guichanfwd.h"
#include "sprite.h"


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
class Particle : public Sprite
{
    public:
        static const float PARTICLE_SKY; /**< Maximum Z position of particles */
        static int fastPhysics;          /**< Mode of squareroot calculation */
        static int particleCount;        /**< Current number of particles */
        static int maxCount;             /**< Maximum number of particles */
        static int emitterSkip;          /**< Duration of pause between two emitter updates in ticks */

        Particle(Map *map);

        ~Particle();

        /**
         * Deletes all child particles and emitters
         */
        void
        clear();

        /**
         * Gives a particle the properties of an engine root particle and loads
         * the particle-related config settings
         */
        void
        setupEngine();

        /**
         * Updates particle position, returns false when the particle should
         * be deleted
         */
        virtual bool
        update();

        /**
         * Draws the particle image
         */
        virtual void
        draw(Graphics *graphics, int offsetX, int offsetY) const;

        /**
         * Necessary for sorting with the other sprites
         */
        virtual int
        getPixelY() const
        {
            return (int)(mPosY + mPosZ) - 64;
        };

        /*
            Basic Particle properties:
         */

        /**
         * Sets the map the particle is on.
         */
        void setMap(Map *map);

        /**
         * Creates a child particle that hosts some emitters described in the
         * particleEffectFile.
         */
        Particle*
        addEffect(std::string particleEffectFile, int pixelX, int pixelY);

        /**
         * Creates a standalone text particle.
         */
        Particle*
        addTextSplashEffect(std::string text, int colorR, int colorG, int colorB,
                            gcn::Font *font, int x, int y);

        /**
         * Adds an emitter to the particle.
         */
        void
        addEmitter (ParticleEmitter* emitter)
        { mChildEmitters.push_back(emitter); }

        /**
         * Sets the position in 3 dimensional space in pixels relative to map
         */
        void
        setPosition(float x, float y, float z)
        { mPosX = x; mPosY = y; mPosZ = z; }

        /**
         * Sets the position in 2 dimensional space in pixels relative to map
         */
        void
        setPosition(float x, float y)
        { mPosX = x; mPosY = y; }

        float getPosX() const
        { return mPosX; }

        float getPosY() const
        { return mPosY; }

        float getPosZ() const
        { return mPosZ; }

        /**
         * Changes the particle position relative
         */
        void
        moveBy(float x, float y, float z)
        { mPosX += x; mPosY += y; mPosZ += z; }

        /**
         * Sets the time in game ticks until the particle is destroyed.
         */
        void
        setLifetime(int lifetime)
        { mLifetimeLeft = lifetime; mLifetimePast = 0; }

        /**
         * Sets the age of the pixel in game ticks where the particle has
         * faded in completely
         */
        void
        setFadeOut (int fadeOut)
        { mFadeOut = fadeOut; }

        /**
         * Sets the remaining particle lifetime where the particle starts to
         * fade out
         */
        void
        setFadeIn (int fadeIn)
        { mFadeIn = fadeIn; }

        /**
         * Sets the sprite iterator of the particle on the current map to make
         * it easier to remove the particle from the map when it is destroyed
         */
        void
        setSpriteIterator(std::list<Sprite*>::iterator spriteIterator)
        { mSpriteIterator = spriteIterator; }

        /**
         * Gets the sprite iterator of the particle on the current map
         */
        std::list<Sprite*>::iterator
        getSpriteIterator() const
        { return mSpriteIterator; }

        /**
         * Sets the current velocity in 3 dimensional space
         */
        void
        setVector(float x, float y, float z)
        { mVectorX = x; mVectorY = y; mVectorZ = z; }

        /**
         * Sets the downward acceleration
         */
        void
        setGravity(float g)
        { mGravity = g; }

        /**
         * Sets the ammount of random vector changes
         */
        void
        setRandomnes(int r)
        { mRandomnes = r; }

        /**
         * Sets the ammount of velocity particles retain after
         * hitting the ground.
         */
        void
        setBounce(float bouncieness)
        { mBounce = bouncieness; }

        /**
         * Makes the particle move toward another particle with a
         * given acceleration and momentum
         */
        void setDestination(Particle *target, float accel, float moment)
        { mTarget = target; mAcceleration = accel; mMomentum = moment; }

        /**
         * Sets the distance in pixel the particle can come near the target
         * particle before it is destroyed. Does only make sense after a
         * target particle has been set using setDestination.
         */
        void setDieDistance(float dist)
        { mInvDieDistance = 1.0f / dist; }

        /**
         * Manually marks the particle for deletion
         */
        void kill()
        { mAlive = false; mAutoDelete = true; }

        /**
         * After calling this function the particle will only request
         * deletion when kill() is called
         */
        void disableAutoDelete()
        { mAutoDelete = false; }

    protected:
        bool mAlive;                /**< Is the particle supposed to be drawn and updated?*/
        float mPosX, mPosY, mPosZ;  /**< Position in 3 dimensonal space - pixel based relative to map */
        int mLifetimeLeft;          /**< Lifetime left in game ticks*/
        int mLifetimePast;          /**< Age of the particle in game ticks*/
        int mFadeOut;               /**< Lifetime in game ticks left where fading out begins*/
        int mFadeIn;                /**< Age in game ticks where fading in is finished*/

    private:
        // generic properties
        bool mAutoDelete;           /**< May the particle request its deletion by the parent particle?*/
        Map *mMap;                  /**< Map the particle is on*/
        std::list<Sprite*>::iterator mSpriteIterator;   /**< iterator of the particle on the current map */
        Emitters mChildEmitters;    /**< List of child emitters*/
        Particles mChildParticles;  /**< List of particles controlled by this particle*/
        //dynamic particle
        float mVectorX, mVectorY, mVectorZ; /**< Speed in 3 dimensional space in pixels per game-tick */
        float mGravity;             /**< Downward acceleration in pixels per game-tick²*/
        int mRandomnes;             /**< Ammount of random vector change*/
        float mBounce;              /**< How much the particle bounces off when hitting the ground*/
        //follow-point particles
        Particle *mTarget;          /**< The particle that attracts this particle*/
        float mAcceleration;        /**< Acceleration towards the target particle in pixels per game-tick²*/
        float mInvDieDistance;      /**< Distance in pixels from the target particle that causes the destruction of the particle*/
        float mMomentum;            /**< How much speed the particle retains after each game tick*/
};

extern Particle *particleEngine;

#endif
