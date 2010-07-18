/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <map>
#include <string>

enum Attribute
{
    LEVEL,
    HP, MAX_HP,
    MP, MAX_MP,
    EXP, EXP_NEEDED,
    MONEY,
    TOTAL_WEIGHT, MAX_WEIGHT,
    SKILL_POINTS,
    CHAR_POINTS, CORR_POINTS
};

struct Stat
{
    int base;
    int mod;
    int exp;
    int expneed;
};

typedef std::map<int, int> IntMap;
typedef std::map<int, Stat> StatMap;

struct PlayerInfoBackend
{
    public:
    IntMap mAttributes;
    StatMap mStats;
};

/**
 * A database like class which holds global info about the localplayer
 */
class PlayerInfo
{
    // NOTE: All agruements for 'bool notify' is to determine if
    // a event is to be triggered
    public:
        static void setBackend(const PlayerInfoBackend &backend);

        /**
         * Attributes for things like money and exp
         */
        static int getAttribute(int id);

        static void setAttribute(int id, int value, bool notify = true);


        /**
         * Stats are modifiable attributes basicilly, like str, crit, trade
         */

        static int getStatBase(int id);

        static void setStatBase(int id, int value, bool notify = true);

        static int getStatMod(int id);

        static void setStatMod(int id, int value, bool notify = true);

        // Base + mod
        static int getStatEffective(int id);

        static void setStatLevel(int id, int value, bool notify = true);

        static std::pair<int, int> getStatExperience(int id);

        static void setStatExperience(int id, int have, int need, bool notify = true);

    private:
        // Triggers send events for action.
        static void triggerAttr(int id);

        static void triggerStat(int id);

        static PlayerInfoBackend mData;
};

#endif
