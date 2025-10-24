/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2025  The Mana Developers
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

#include "statuseffect.h"

#include "configuration.h"
#include "event.h"
#include "particle.h"
#include "sound.h"
#include "sprite.h"

/**
 * Plays the sound effect associated with this status effect, if possible.
 */
void StatusEffect::playSfx(bool enabled) const
{
    auto &sfx = enabled ? start.sfx : end.sfx;
    if (!sfx.empty())
        sound.playSfx(sfx);
}

/**
 * Delivers the chat message associated with this status effect, if
 * possible.
 */
void StatusEffect::deliverMessage(bool enabled) const
{
    auto &message = enabled ? start.message : end.message;
    if (!message.empty())
        serverNotice(message);
}

/**
 * Creates the particle effect associated with this status effect, if
 * possible.
 */
Particle *StatusEffect::getParticle(bool enabled) const
{
    auto &particleEffect = enabled ? start.particleEffect : end.particleEffect;
    if (particleEffect.empty())
        return nullptr;
    return particleEngine->addEffect(particleEffect, 0, 0);
}

/**
 * Retrieves the status icon for this effect, if applicable.
 */
Sprite *StatusEffect::getIconSprite() const
{
    if (icon.empty())
        return nullptr;
    return Sprite::load(paths.getStringValue("sprites") + icon);
}
