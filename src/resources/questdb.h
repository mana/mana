/*
 *  The Mana Client
 *  Copyright (C) 2025  The Mana Developers
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

#include "utils/xml.h"

#include <map>
#include <string>
#include <vector>

/**
 * A map that returns a default value for non-existent keys.
 */
template<typename Key, typename Value, Value def = Value()>
class MapWithDefault
{
public:
    void set(Key key, Value value)
    {
        mVars[key] = value;
    }

    Value get(Key key) const
    {
        auto it = mVars.find(key);
        return it != mVars.end() ? it->second : def;
    }

    void clear()
    {
        mVars.clear();
    }

private:
    std::map<Key, Value> mVars;
};

struct QuestEffect
{
    std::vector<int> values;        // Quest variable values to which the effect applies
    std::string map;                // Map name the NPC is located on
    int npcId = 0;
    int statusEffectId = 0;
};

// Map of quest variables, from variable ID to value
using QuestVars = MapWithDefault<int, int>;

// Map of quest effects, from NPC ID to status effect ID
using QuestEffectMap = MapWithDefault<int, int>;

enum class QuestRowType
{
    Text,
    Name,
    Reward,
    Giver,
    Coordinates,
    NPC
};

struct QuestRow
{
    QuestRow(QuestRowType type)
        : type(type)
    {}

    QuestRowType type;
    std::string text;
    int x = 0;
    int y = 0;
};

struct QuestState
{
    std::string name;               // Name of the quest in this state
    std::string group;              // Group name of the quest in this state
    std::vector<int> incomplete;    // Quest variable values for this state (quest incomplete)
    std::vector<int> complete;      // Quest variable values for this state (quest complete)
    std::vector<QuestRow> rows;     // Rows of text in the Quests window for this state
};

struct Quest
{
    std::vector<QuestEffect> effects;
    std::vector<QuestState> states;
};

struct QuestEntry
{
    int varId;
    bool completed;
    const QuestState *state;

    const std::string &name() const { return state->name; }
    const std::vector<QuestRow> &rows() const { return state->rows; }
};

enum class QuestChange
{
    None,
    New,
    Completed
};

namespace QuestDB
{
    void readQuestVarNode(XML::Node node, const std::string &filename);
    void unload();

    bool hasQuests();

    QuestEffectMap getActiveEffects(const QuestVars &questVars,
                                    const std::string &mapName);

    std::vector<QuestEntry> getQuestsEntries(const QuestVars &questVars,
                                             bool skipCompleted = false);

    QuestChange questChange(int varId, int oldValue, int newValue);
};
