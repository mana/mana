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

#include "resources/beinginfo.h"

#include "log.h"
#include "configuration.h"

#include "utils/gettext.h"

#include <optional>

BeingInfo *BeingInfo::Unknown = new BeingInfo;

static std::optional<ActorSprite::TargetCursorSize> targetCursorSizeFromString(const std::string &cursor)
{
    if (cursor == "small")      return ActorSprite::TC_SMALL;
    if (cursor == "medium")     return ActorSprite::TC_MEDIUM;
    if (cursor == "large")      return ActorSprite::TC_LARGE;

    return {};
}

static std::optional<Cursor> cursorFromString(const std::string &cursor)
{
    if (cursor == "pointer")    return Cursor::POINTER;
    if (cursor == "attack")     return Cursor::FIGHT;
    if (cursor == "pickup")     return Cursor::PICKUP;
    if (cursor == "talk")       return Cursor::TALK;
    if (cursor == "action")     return Cursor::ACTION;
    if (cursor == "left")       return Cursor::LEFT;
    if (cursor == "up")         return Cursor::UP;
    if (cursor == "right")      return Cursor::RIGHT;
    if (cursor == "down")       return Cursor::DOWN;

    return {};
}

BeingInfo::BeingInfo():
    mName(_("unnamed")),
    mWalkMask(Map::BLOCKMASK_WALL | Map::BLOCKMASK_CHARACTER | Map::BLOCKMASK_MONSTER)
{
    SpriteDisplay display;

    SpriteReference errorSprite { paths.getStringValue("spriteErrorFile"), 0 };
    display.sprites.push_back(errorSprite);

    setDisplay(std::move(display));
}

BeingInfo::~BeingInfo() = default;

void BeingInfo::setDisplay(SpriteDisplay display)
{
    mDisplay = std::move(display);
}

void BeingInfo::setTargetCursorSize(const std::string &size)
{
    const auto targetCursorSize = targetCursorSizeFromString(size);
    if (!targetCursorSize)
    {
        logger->log("Unknown targetCursor value \"%s\" for %s",
                    size.c_str(), getName().c_str());
    }
    setTargetCursorSize(targetCursorSize.value_or(ActorSprite::TC_MEDIUM));
}

void BeingInfo::setHoverCursor(const std::string &cursorName)
{
    const auto cursor = cursorFromString(cursorName);
    if (!cursor)
    {
        logger->log("Unknown hoverCursor value \"%s\" for %s",
                    cursorName.c_str(), getName().c_str());
    }
    setHoverCursor(cursor.value_or(Cursor::POINTER));
}

void BeingInfo::addSound(SoundEvent event, const std::string &filename)
{
    mSounds[event].push_back("sfx/" + filename);
}

const std::string &BeingInfo::getSound(SoundEvent event) const
{
    static const std::string empty;

    auto i = mSounds.find(event);
    return i == mSounds.end() ? empty :
                                i->second.at(rand() % i->second.size());
}

const Attack &BeingInfo::getAttack(int id) const
{
    static const Attack empty {
        SpriteAction::ATTACK,
        -1, // Default strike effect on monster
        paths.getIntValue("hitEffectId"),
        paths.getIntValue("criticalHitEffectId"),
        std::string()
    };

    auto it = mAttacks.find(id);
    return it == mAttacks.end() ? empty : it->second;
}

void BeingInfo::addAttack(int id, Attack attack)
{
    mAttacks[id] = std::move(attack);
}
