/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#pragma once

#include "actorsprite.h"
#include "map.h"

#include "gui/gui.h"
#include "resources/spritedef.h"

#include <map>
#include <string>
#include <vector>

struct Attack
{
    std::string action = SpriteAction::ATTACK;
    int effectId = 0;
    int hitEffectId = 0;
    int criticalHitEffectId = 0;
    std::string missileParticleFilename;
};

enum class SoundEvent
{
    Hit,
    Miss,
    Hurt,
    Die
};

/**
 * Holds information about a certain type of monster. This includes the name
 * of the monster, the sprite to display and the sounds the monster makes.
 *
 * @see MonsterDB
 * @see NPCDB
 */
class BeingInfo
{
public:
    static BeingInfo *Unknown;

    BeingInfo();
    ~BeingInfo();

    std::string name;
    SpriteDisplay display;
    ActorSprite::TargetCursorSize targetCursorSize = ActorSprite::TC_MEDIUM;
    Cursor hoverCursor = Cursor::Pointer;
    unsigned char walkMask = Map::BLOCKMASK_ALL;
    Map::BlockType blockType = Map::BLOCKTYPE_CHARACTER;
    bool targetSelection = true;

    void setTargetCursorSize(const std::string &size);
    void setHoverCursor(const std::string &cursorName);

    void addSound(SoundEvent event, const std::string &filename);
    const std::string &getSound(SoundEvent event) const;

    void addAttack(int id, Attack attack);
    const Attack &getAttack(int id) const;

private:
    std::map<SoundEvent, std::vector<std::string>> mSounds;
    std::map<int, Attack> mAttacks;
};
