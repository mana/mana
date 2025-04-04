/*
 *  The Mana Client
 *  Copyright (C) 2010-2013  The Mana Developers
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
#include "utils/xml.h"

struct AbilityInfo
{
    enum TargetMode
    {
        TARGET_BEING,       // target any being
        TARGET_POINT,       // target map location
        TARGET_DIRECTION    // target a direction
    };
    int id;
    std::string name; // displayed name of ability
    std::string icon; // filename of graphical icon
    std::string useAction; // action when using the ability

    TargetMode targetMode;

    bool rechargeable; // true when the ability has a recharge bar
    int rechargeNeeded; // maximum recharge when applicable
    int rechargeCurrent; // current recharge when applicable
};

/**
 * Ability information database.
 */
namespace AbilityDB
{
    void init();

    void readAbilityNode(XML::Node node, const std::string &filename);

    void checkStatus();

    void unload();

    /** Gets the ability info for ID. Will return nullptr when it is
     *  a server-specific ability.
     */
    AbilityInfo *get(int id);

    /**
     * Finds an ability by name. Returns nullptr when the ability could not be
     * found.
     */
    AbilityInfo *find(std::string_view name);
}
