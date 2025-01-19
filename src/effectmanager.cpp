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
        if (node.name() == "effect")
        {
            EffectDescription &ed = mEffects.emplace_back();
            ed.id = node.getProperty("id", -1);
            ed.GFX = node.getProperty("particle", "");
            ed.SFX = node.getProperty("audio", "");
        }
    }
}

EffectManager::~EffectManager()
{
}

bool EffectManager::trigger(int id, Being* being, int rotation)
{
    bool rValue = false;
    for (auto &effect : mEffects)
    {
        if (effect.id == id)
        {
            rValue = true;
            if (!effect.GFX.empty())
            {
                Particle *selfFX;
                selfFX = particleEngine->addEffect(effect.GFX, 0, 0, rotation);
                being->controlParticle(selfFX);
            }
            if (!effect.SFX.empty())
                sound.playSfx(effect.SFX);
            break;
        }
    }
    return rValue;
}

bool EffectManager::trigger(int id, int x, int y, int rotation)
{
    bool rValue = false;
    for (auto &effect : mEffects)
    {
        if (effect.id == id)
        {
            rValue = true;
            if (!effect.GFX.empty())
                particleEngine->addEffect(effect.GFX, x, y, rotation);
            if (!effect.SFX.empty())
                sound.playSfx(effect.SFX);
            break;
        }
    }
    return rValue;
}
