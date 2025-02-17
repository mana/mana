/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2013  The Mana Developers
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

#include "event.h"
#include "sound.h"

#include "configuration.h"

#include <map>

#define STATUS_EFFECTS_FILE "status-effects.xml"

bool StatusEffect::mLoaded = false;

StatusEffect::StatusEffect() = default;
StatusEffect::~StatusEffect() = default;

void StatusEffect::playSFX()
{
    if (!mSFXEffect.empty())
        sound.playSfx(mSFXEffect);
}

void StatusEffect::deliverMessage()
{
    if (!mMessage.empty())
        serverNotice(mMessage);
}

Particle *StatusEffect::getParticle()
{
    if (mParticleEffect.empty())
        return nullptr;
    return particleEngine->addEffect(mParticleEffect, 0, 0);
}

Sprite *StatusEffect::getIcon()
{
    if (mIcon.empty())
        return nullptr;

    Sprite *sprite = Sprite::load(
                paths.getStringValue("sprites") + mIcon);
    if (false && sprite)
    {
        sprite->play(SpriteAction::DEFAULT);
        sprite->reset();
    }
    return sprite;
}

std::string StatusEffect::getAction() const
{
    if (mAction.empty())
        return SpriteAction::INVALID;
    return mAction;
}


// -- initialisation and static parts --


typedef std::map<int, StatusEffect *> status_effect_map[2];

static status_effect_map statusEffects;
static status_effect_map stunEffects;
static std::map<int, int> blockEffectIndexMap;

int StatusEffect::blockEffectIndexToEffectIndex(int blockIndex)
{
    if (blockEffectIndexMap.find(blockIndex) == blockEffectIndexMap.end())
        return -1;
    return blockEffectIndexMap[blockIndex];
}

StatusEffect *StatusEffect::getStatusEffect(int index, bool enabling)
{
    return statusEffects[enabling][index];
}

StatusEffect *StatusEffect::getStunEffect(int index, bool enabling)
{
    return stunEffects[enabling][index];
}

void StatusEffect::init()
{
    if (mLoaded)
        unload();
}

void StatusEffect::readStatusEffectNode(XML::Node node, const std::string &filename)
{
    status_effect_map *the_map = nullptr;
    int index = atoi(node.getProperty("id", "-1").c_str());
    if (node.name() == "status-effect")
    {
        the_map = &statusEffects;
        int block_index = atoi(node.getProperty("block-id", "-1").c_str());

        if (index >= 0 && block_index >= 0)
            blockEffectIndexMap[block_index] = index;
    }
    else if (node.name() == "stun-effect")
        the_map = &stunEffects;

    if (the_map)
    {
        auto *startEffect = new StatusEffect;
        auto *endEffect = new StatusEffect;

        startEffect->mMessage = node.getProperty("start-message", "");
        startEffect->mSFXEffect = node.getProperty("start-audio", "");
        startEffect->mParticleEffect = node.getProperty("start-particle", "");
        startEffect->mIcon = node.getProperty("icon", "");
        startEffect->mAction = node.getProperty("action", "");
        startEffect->mPersistentParticleEffect = (node.getProperty("persistent-particle-effect", "no")) != "no";

        endEffect->mMessage = node.getProperty("end-message", "");
        endEffect->mSFXEffect = node.getProperty("end-audio", "");
        endEffect->mParticleEffect = node.getProperty("end-particle", "");

        (*the_map)[1][index] = startEffect;
        (*the_map)[0][index] = endEffect;
    }

}

void StatusEffect::checkStatus()
{
    mLoaded = true;
}

void unloadMap(std::map<int, StatusEffect *> map)
{
    for (auto &[_, effect] : map)
        delete effect;

    map.clear();
}

void StatusEffect::unload()
{
    if (!mLoaded)
        return;

    unloadMap(statusEffects[0]);
    unloadMap(statusEffects[1]);
    unloadMap(stunEffects[0]);
    unloadMap(stunEffects[1]);

    mLoaded = false;
}
