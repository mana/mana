/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#ifndef ACTORSPRITE_H
#define ACTORSPRITE_H

#include "actor.h"
#include "compoundsprite.h"
#include "map.h"
#include "particlecontainer.h"

#include <cstdint>
#include <set>
#include <list>

class SimpleAnimation;
class StatusEffect;

class ActorSprite : public CompoundSprite, public Actor
{
public:
    enum Type
    {
        UNKNOWN,
        PLAYER,
        NPC,
        MONSTER,
        FLOOR_ITEM
    };

    enum TargetCursorSize
    {
        TC_SMALL = 0,
        TC_MEDIUM,
        TC_LARGE,
        NUM_TC
    };

    enum TargetCursorType
    {
        TCT_NONE = -1,
        TCT_NORMAL = 0,
        TCT_IN_RANGE,
        NUM_TCT
    };

    ActorSprite(int id);

    ~ActorSprite();

    int getId() const
    { return mId; }

    void setId(int id) { mId = id; }

    /**
     * Returns the type of the ActorSprite.
     */
    virtual Type getType() const { return UNKNOWN; }

    virtual bool draw(Graphics *graphics, int offsetX, int offsetY) const;

    virtual bool drawSpriteAt(Graphics *graphics, int x, int y) const;

    virtual void logic();

    static void actorLogic();

    void setMap(Map* map);

    /**
     * Gets the way the object blocks pathfinding for other objects
     */
    virtual Map::BlockType getBlockType() const
    { return Map::BLOCKTYPE_NONE; }

    /**
     * Take control of a particle.
     */
    void controlParticle(Particle *particle);

    /**
     * Returns the required size of a target cursor for this being.
     */
    virtual TargetCursorSize getTargetCursorSize() const
    { return TC_MEDIUM; }

    /**
     * Sets the target animation for this actor.
     */
    void setTargetType(TargetCursorType type);

    /**
     * Untargets the actor.
     */
    void untarget() { mUsedTargetCursor = NULL; }

    /**
     * Triggers a visual effect, such as `level up'. Only draws the visual
     * effect, does not play sound effects.
     *
     * \param effectId ID of the effect to trigger
     */
    virtual void triggerEffect(int effectId)
    {
        internalTriggerEffect(effectId, false, true);
    }

    /**
     * Sets the actor's stun mode. If zero, the being is `normal', otherwise it
     * is `stunned' in some fashion.
     */
    void setStunMode(int stunMode)
    {
        if (mStunMode != stunMode)
            updateStunMode(mStunMode, stunMode);
        mStunMode = stunMode;
    }

    void setStatusEffect(int index, bool active);

    /**
     * A status effect block is a 16 bit mask of status effects. We assign each
     * such flag a block ID of offset + bitnr.
     *
     * These are NOT the same as the status effect indices.
     */
    void setStatusEffectBlock(int offset, uint16_t flags);

    virtual void setAlpha(float alpha)
    { CompoundSprite::setAlpha(alpha); }

    virtual float getAlpha() const
    { return CompoundSprite::getAlpha(); }

    virtual int getWidth() const
    { return CompoundSprite::getWidth(); }

    virtual int getHeight() const
    { return CompoundSprite::getHeight(); }

    static void load();

    static void unload();

protected:
    /**
     * Trigger visual effect, with components
     *
     * \param effectId ID of the effect to trigger
     * \param sfx Whether to trigger sound effects
     * \param gfx Whether to trigger graphical effects
     */
    void internalTriggerEffect(int effectId, bool sfx, bool gfx);

    /**
     * Notify self that the stun mode has been updated. Invoked by
     * setStunMode if something changed.
     */
    virtual void updateStunMode(int oldMode, int newMode);

    /**
     * Notify self that a status effect has flipped.
     * The new flag is passed.
     */
    virtual void updateStatusEffect(int index, bool newStatus);

    /**
     * Handle an update to a status or stun effect
     *
     * \param effect   The StatusEffect to effect
     * \param effectId -1 for stun, otherwise the effect index
     */
    virtual void handleStatusEffect(StatusEffect *effect, int effectId);

    void setupSpriteDisplay(const SpriteDisplay &display,
                            bool forceDisplay = true);

    int mId;
    uint16_t mStunMode;               /**< Stun mode; zero if not stunned */
    std::set<int> mStatusEffects;   /**< set of active status effects */

    ParticleList mStunParticleEffects;
    ParticleVector mStatusParticleEffects;
    ParticleList mChildParticleEffects;

private:
    /** Reset particle status effects on next redraw? */
    bool mMustResetParticles;

    /** Load the target cursors into memory */
    static void initTargetCursor();

    /** Remove the target cursors from memory */
    static void cleanupTargetCursors();

    /**
     * Helper function for loading target cursors
     */
    static void loadTargetCursor(const std::string &filename,
                                 int width, int height, int type, int size);

    /** Images of the target cursor. */
    static ImageSet *targetCursorImages[NUM_TCT][NUM_TC];

    /** Animated target cursors. */
    static SimpleAnimation *targetCursor[NUM_TCT][NUM_TC];

    static bool loaded;

    /** Target cursor being used */
    SimpleAnimation *mUsedTargetCursor;
};

#endif // ACTORSPRITE_H
