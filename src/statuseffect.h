/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef STATUS_EFFECT_H
#define STATUS_EFFECT_H

#include "resources/animation.h"
#include "particle.h"
#include "animatedsprite.h"
#include "sound.h"

class StatusEffect
{
public:
    StatusEffect();
    ~StatusEffect();

    /**
     * Plays the sound effect associated with this status effect, if possible.
     */
    void playSFX();

    /**
     * Delivers the chat message associated with this status effect, if
     * possible.
     */
    void deliverMessage();

    /**
     * Creates the particle effect associated with this status effect, if
     * possible.
     */
    Particle *getParticle();

    /**
     * Retrieves the status icon for this effect, if applicable
     */
    AnimatedSprite *getIcon();

    /**
     * Retrieves an action to perform, or ACTION_INVALID
     */
    SpriteAction getAction();

    /**
     * Determines whether the particle effect should be restarted when the
     * being changes maps
     */
    bool particleEffectIsPersistent() const { return mPersistentParticleEffect; }


    /**
     * Retrieves a status effect.
     *
     * \param index Index of the status effect.
     * \param enabling Whether to retrieve the activating effect (true) or
     *    the deactivating effect (false).
     */
    static StatusEffect *getStatusEffect(int index, bool enabling);

    /**
     * Retrieves a stun effect.
     *
     * \param index Index of the stun effect.
     * \param enabling Whether to retrieve the activating effect (true) or
     *    the deactivating effect (false).
     */
    static StatusEffect *getStunEffect(int index, bool enabling);

    /**
     * Maps a block effect index to its corresponding effect index.  Block
     * effect indices are used for opt2/opt3/status.option blocks; their
     * mapping to regular effect indices is handled in the config file.
     *
     * Returns -1 on failure.
     */
    static int blockEffectIndexToEffectIndex(int blocKIndex);

    static void load();

    static void unload();
private:

    std::string mMessage;
    std::string mSFXEffect;
    std::string mParticleEffect;
    std::string mIcon;
    std::string mAction;
    bool mPersistentParticleEffect;
};

#endif // !defined(STATUS_EFFECT_H)
