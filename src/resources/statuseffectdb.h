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

#ifndef STATUSEFFECTDB_H
#define STATUSEFFECTDB_H

#include "statuseffect.h"
#include "utils/xml.h"

#include <cstdint>
#include <map>

class StatusEffectDB
{
public:
    /**
     * Retrieves a status effect.
     *
     * \param id ID of the status effect.
     */
    static const StatusEffect *getStatusEffect(int id);

    using OptionsMap = std::map<uint16_t, int>;

    /**
     * These map flags or indexes to their corresponding status effect ID.
     * This is tmwAthena-specific.
     */
    static const OptionsMap &opt0ToIdMap() { return mOpt0ToIdMap; }
    static const OptionsMap &opt1ToIdMap() { return mOpt1ToIdMap; }
    static const OptionsMap &opt2ToIdMap() { return mOpt2ToIdMap; }
    static const OptionsMap &opt3ToIdMap() { return mOpt3ToIdMap; }

    static void init();
    static void readStatusEffectNode(XML::Node node, const std::string &filename);
    static void checkStatus();
    static void unload();

private:
    static bool mLoaded;

    static std::map<int, StatusEffect> mStatusEffects;
    static OptionsMap mOpt0ToIdMap;
    static OptionsMap mOpt1ToIdMap;
    static OptionsMap mOpt2ToIdMap;
    static OptionsMap mOpt3ToIdMap;
};

#endif // STATUSEFFECTDB_H
