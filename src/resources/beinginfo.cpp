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

static BeingInfo *createUnknownBeingInfo()
{
    auto info = new BeingInfo;
    info->name = _("unnamed");

    SpriteReference errorSprite { paths.getStringValue("spriteErrorFile"), 0 };
    info->display.sprites.push_back(std::move(errorSprite));

    return info;
}

BeingInfo *BeingInfo::Unknown = createUnknownBeingInfo();

static std::optional<ActorSprite::TargetCursorSize> targetCursorSizeFromString(const std::string &cursor)
{
    if (cursor == "small")      return ActorSprite::TC_SMALL;
    if (cursor == "medium")     return ActorSprite::TC_MEDIUM;
    if (cursor == "large")      return ActorSprite::TC_LARGE;

    return {};
}

static std::optional<Cursor> cursorFromString(const std::string &cursor)
{
    if (cursor == "pointer")    return Cursor::Pointer;
    if (cursor == "attack")     return Cursor::Fight;
    if (cursor == "pickup")     return Cursor::PickUp;
    if (cursor == "talk")       return Cursor::Talk;
    if (cursor == "action")     return Cursor::Action;
    if (cursor == "left")       return Cursor::Left;
    if (cursor == "up")         return Cursor::Up;
    if (cursor == "right")      return Cursor::Right;
    if (cursor == "down")       return Cursor::Down;

    return {};
}

BeingInfo::BeingInfo() = default;
BeingInfo::~BeingInfo() = default;

void BeingInfo::setTargetCursorSize(const std::string &size)
{
    const auto cursorSize = targetCursorSizeFromString(size);
    if (!cursorSize)
    {
        logger->log("Unknown targetCursor value \"%s\" for %s",
                    size.c_str(), name.c_str());
    }
    targetCursorSize = cursorSize.value_or(ActorSprite::TC_MEDIUM);
}

void BeingInfo::setHoverCursor(const std::string &cursorName)
{
    const auto cursor = cursorFromString(cursorName);
    if (!cursor)
    {
        logger->log("Unknown hoverCursor value \"%s\" for %s",
                    cursorName.c_str(), name.c_str());
    }
    hoverCursor = cursor.value_or(Cursor::Pointer);
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
