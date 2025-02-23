/*
 *  An effects manager
 *  Copyright (C) 2008  Fate <fate.tmw@googlemail.com>
 *  Copyright (C) 2008  Chuck Miller <shadowmil@gmail.com>
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

#include "being.h"
#include "effectmanager.h"
#include "log.h"
#include "particle.h"
#include "sound.h"

#include "utils/xml.h"

EffectManager::EffectManager()
{
    XML::Document doc("effects.xml");
    XML::Node root = doc.rootNode();

    if (!root || root.name() != "effects")
    {
        // Handle old naming until the 0.5.x versions are obsolete.
        if (!root || root.name() != "being-effects")
        {
            logger->log("Error loading being effects file: effects.xml");
            return;
        }
    }
    else
    {
        logger->log("Effects are now loading");
    }

    for (auto node : root.children())
    {
        int effectId;

        if (node.name() == "effect" && node.attribute("id", effectId))
        {
            EffectDescription &ed = mEffects[effectId];
            node.attribute("particle", ed.particle);
            node.attribute("audio", ed.sfx);
        }
    }
}

EffectManager::~EffectManager() = default;

bool EffectManager::trigger(int id, Being *being, int rotation)
{
    auto it = mEffects.find(id);
    if (it == mEffects.end())
    {
        logger->log("EffectManager::trigger: effect %d not found", id);
        return false;
    }

    EffectDescription &effect = it->second;

    if (!effect.particle.empty())
    {
        if (Particle *selfFX = particleEngine->addEffect(effect.particle, 0, 0, rotation))
            being->controlParticle(selfFX);
    }

    if (!effect.sfx.empty())
        sound.playSfx(effect.sfx);

    return true;
}

bool EffectManager::trigger(int id, int x, int y, int rotation)
{
    auto it = mEffects.find(id);
    if (it == mEffects.end())
    {
        logger->log("EffectManager::trigger: effect %d not found", id);
        return false;
    }

    EffectDescription &effect = it->second;

    if (!effect.particle.empty())
        particleEngine->addEffect(effect.particle, x, y, rotation);
    if (!effect.sfx.empty())
        sound.playSfx(effect.sfx);

    return true;
}
