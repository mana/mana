/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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
#include "compoundsprite.h"
#include "map.h"
#include "particle.h"

class SimpleAnimation;

class ActorSprite : public Actor
{
public:
    enum Type
    {
        UNKNOWN,
        PLAYER,
        NPC,
        MONSTER,
        FLOOR_ITEM,
        PORTAL
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

    ~ActorSprite() override;

    int getId() const
    { return mId; }

    void setId(int id) { mId = id; }

    /**
     * Returns the type of the ActorSprite.
     */
    virtual Type getType() const { return UNKNOWN; }

    int getDrawOrder() const override;

    bool draw(Graphics *graphics, int offsetX, int offsetY) const override;

    bool drawSpriteAt(Graphics *graphics, int x, int y) const;

    virtual void logic();

    void setMap(Map* map) override;

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
    void untarget() { mUsedTargetCursor = nullptr; }

    void setAlpha(float alpha) override { mSprites.setAlpha(alpha); }
    float getAlpha() const override { return mSprites.getAlpha(); }

    int getWidth() const { return mSprites.getWidth(); }
    int getHeight() const { return mSprites.getHeight(); }

    static void load();
    static void unload();

protected:
    void setupSpriteDisplay(const SpriteDisplay &display,
                            bool forceDisplay = true);

    int mId;
    std::vector<ParticleHandle> mChildParticleEffects;

    CompoundSprite mSprites;

private:
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
    static ResourceRef<ImageSet> targetCursorImages[NUM_TCT][NUM_TC];

    /** Animated target cursors. */
    static SimpleAnimation *targetCursor[NUM_TCT][NUM_TC];

    static bool loaded;

    /** Target cursor being used */
    SimpleAnimation *mUsedTargetCursor = nullptr;
};
