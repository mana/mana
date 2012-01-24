/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#ifndef SPECIAL_DB_H
#define SPECIAL_DB_H

#include <string>
#include <map>

struct SpecialInfo
{
    enum TargetMode
    {
        TARGET_SELF,    // no target selection
        TARGET_FRIEND,  // target friendly being
        TARGET_ENEMY,   // target hostile being
        TARGET_BEING,   // target any being
        TARGET_POINT    // target map location
    };
    int id;
    std::string set; // tab on which the special is shown
    std::string name; // displayed name of special
    std::string icon; // filename of graphical icon

    bool isActive; // true when the special can be used
    TargetMode targetMode; // target mode

    bool hasLevel; // true when the special has levels
    int level; // level of special when applicable

    bool hasRechargeBar; // true when the special has a recharge bar
    int rechargeNeeded; // maximum recharge when applicable
    int rechargeCurrent; // current recharge when applicable
};

/**
 * Special information database.
 */
namespace SpecialDB
{
    void load();

    void unload();

    /** gets the special info for ID. Will return 0 when it is
     *  a server-specific special.
     */
    SpecialInfo *get(int id);

    SpecialInfo::TargetMode targetModeFromString(const std::string& str);
}

typedef std::map<int, SpecialInfo *> SpecialInfos;

#endif
