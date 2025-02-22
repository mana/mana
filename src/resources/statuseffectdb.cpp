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

#include "statuseffectdb.h"

bool StatusEffectDB::mLoaded = false;
std::map<int, StatusEffect> StatusEffectDB::mStatusEffects;
StatusEffectDB::OptionsMap StatusEffectDB::mOpt0ToIdMap;
StatusEffectDB::OptionsMap StatusEffectDB::mOpt1ToIdMap;
StatusEffectDB::OptionsMap StatusEffectDB::mOpt2ToIdMap;
StatusEffectDB::OptionsMap StatusEffectDB::mOpt3ToIdMap;


const StatusEffect *StatusEffectDB::getStatusEffect(int id)
{
    auto it = mStatusEffects.find(id);
    if (it == mStatusEffects.end())
        return nullptr;
    return &it->second;
}

void StatusEffectDB::init()
{
    if (mLoaded)
        unload();
}

void StatusEffectDB::readStatusEffectNode(XML::Node node, const std::string &/* filename */)
{
    const int id = node.getProperty("id", -1);

    const int opt0 = node.getProperty("option", 0);
    const int opt1 = node.getProperty("opt1", 0);
    const int opt2 = node.getProperty("opt2", 0);
    const int opt3 = node.getProperty("opt3", 0);
    if (opt0 != 0 && opt0 <= UINT16_MAX)
        mOpt0ToIdMap[opt0] = id;
    if (opt1 != 0 && opt1 <= UINT16_MAX)
        mOpt1ToIdMap[opt1] = id;
    if (opt2 != 0 && opt2 <= UINT16_MAX)
        mOpt2ToIdMap[opt2] = id;
    if (opt3 != 0 && opt3 <= UINT16_MAX)
        mOpt3ToIdMap[opt3] = id;

    auto &effect = mStatusEffects[id];

    node.attribute("start-message", effect.start.message);
    node.attribute("start-audio", effect.start.sfx);
    node.attribute("start-particle", effect.start.particleEffect);

    // For now we don't support separate particle effect for "already applied"
    // status effects.
    if (effect.start.particleEffect.empty())
        node.attribute("particle", effect.start.particleEffect);

    node.attribute("end-message", effect.end.message);
    node.attribute("end-audio", effect.end.sfx);
    node.attribute("end-particle", effect.end.particleEffect);

    node.attribute("icon", effect.icon);
    node.attribute("persistent-particle-effect", effect.persistentParticleEffect);
}

void StatusEffectDB::checkStatus()
{
    mLoaded = true;
}

void StatusEffectDB::unload()
{
    if (!mLoaded)
        return;

    mStatusEffects.clear();
    mLoaded = false;
}
