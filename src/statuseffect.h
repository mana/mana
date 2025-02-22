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

#pragma once

#include <string>

class Particle;
class Sprite;

class StatusEffect
{
public:
    struct Event
    {
        std::string message;
        std::string sfx;
        std::string particleEffect;
    };

    Event start;
    Event end;
    std::string icon;

    /**
     * Determines whether the particle effect should be restarted when the
     * being changes maps.
     */
    bool persistentParticleEffect = false;

    StatusEffect() = default;

    void playSfx(bool enabled) const;
    void deliverMessage(bool enabled) const;
    Particle *getParticle(bool enabled) const;
    Sprite *getIconSprite() const;
};
