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

#include "resources/questdb.h"
#include "log.h"

#include <algorithm>
#include <unordered_map>
#include <utility>

namespace QuestDB {

// The quests are stored in a map using their variable ID as the key
static std::unordered_map<int, Quest> quests;

// Helper function to check if a container contains a value
template<typename Container, typename Value>
static bool contains(const Container &container, const Value &value)
{
    return std::find(container.begin(), container.end(), value) != container.end();
}

void readQuestVarNode(XML::Node node, const std::string &filename)
{
    int varId = 0;
    if (!node.attribute("id", varId))
        return;

    Quest &quest = quests[varId];

    for (auto child : node.children())
    {
        if (child.name() == "effect")
        {
            QuestEffect &effect = quest.effects.emplace_back();
            child.attribute("map", effect.map);
            child.attribute("npc", effect.npcId);
            child.attribute("effect", effect.statusEffectId);
            child.attribute("value", effect.values);

            if (effect.map.empty() || effect.npcId == 0 || effect.statusEffectId == 0 || effect.values.empty())
            {
                Log::warn("effect node for var %d is missing required attributes", varId);
            }
        }
        else if (child.name() == "quest")
        {
            QuestState &state = quest.states.emplace_back();
            child.attribute("name", state.name);
            child.attribute("group", state.group);
            child.attribute("incomplete", state.incomplete);
            child.attribute("complete", state.complete);

            if (state.incomplete.empty() && state.complete.empty())
            {
                Log::warn("quest node for var %d ('%s') has neither 'complete' nor 'incomplete' values",
                          varId, state.name.c_str());
                continue;
            }

            for (auto questChild : child.children())
            {
                QuestRowType rowType;
                std::string_view tag = questChild.name();
                if (tag == "text")
                    rowType = QuestRowType::Text;
                else if (tag == "name")
                    rowType = QuestRowType::Name;
                else if (tag == "reward")
                    rowType = QuestRowType::Reward;
                else if (tag == "questgiver" || tag == "giver")
                    rowType = QuestRowType::Giver;
                else if (tag == "coordinates")
                    rowType = QuestRowType::Coordinates;
                else if (tag == "npc")
                    rowType = QuestRowType::NPC;
                else
                {
                    Log::warn("unknown quest row type '%s' for var %d ('%s')",
                              tag.data(), varId, state.name.c_str());
                    continue;
                }

                QuestRow &row = state.rows.emplace_back(rowType);
                row.text = questChild.textContent();

                if (rowType == QuestRowType::Coordinates)
                {
                    questChild.attribute("x", row.x);
                    questChild.attribute("y", row.y);
                }
            }
        }
    }
}

void unload()
{
    quests.clear();
}

bool hasQuests()
{
    return !quests.empty();
}

// In quests, the map name may include the file extension. This is discouraged
// but supported for compatibility.
static std::string_view baseName(const std::string &fileName)
{
    auto pos = fileName.find_last_of('.');
    return pos == std::string::npos ? fileName : std::string_view(fileName.data(), pos);
}

QuestEffectMap getActiveEffects(const QuestVars &questVars,
                                const std::string &mapName)
{
    QuestEffectMap activeEffects;

    for (auto &[var, quest] : std::as_const(quests))
    {
        auto value = questVars.get(var);

        for (auto &effect : quest.effects)
        {
            if (baseName(effect.map) != mapName)
                continue;
            if (!contains(effect.values, value))
                continue;

            activeEffects.set(effect.npcId, effect.statusEffectId);
        }
    }

    return activeEffects;
}

std::vector<QuestEntry> getQuestsEntries(const QuestVars &questVars,
                                         bool skipCompleted)
{
    std::vector<QuestEntry> activeQuests;

    for (auto &[varId, quest] : std::as_const(quests))
    {
        auto value = questVars.get(varId);

        for (auto &state : quest.states)
        {
            bool matchesIncomplete = contains(state.incomplete, value);
            bool matchesComplete = contains(state.complete, value);

            if (skipCompleted && matchesComplete)
                continue;

            if (matchesIncomplete || matchesComplete)
            {
                QuestEntry &entry = activeQuests.emplace_back();
                entry.varId = varId;
                entry.completed = matchesComplete;
                entry.state = &state;
            }
        }
    }

    return activeQuests;
}

static std::pair<int, int> countQuestEntries(const Quest &quest, int value)
{
    int totalEntries = 0;
    int completedEntries = 0;

    for (const auto &state : quest.states)
    {
        bool matchesIncomplete = contains(state.incomplete, value);
        bool matchesComplete = contains(state.complete, value);

        if (matchesIncomplete || matchesComplete)
        {
            totalEntries++;
            if (matchesComplete)
                completedEntries++;
        }
    }

    return { totalEntries, completedEntries };
}

QuestChange questChange(int varId, int oldValue, int newValue)
{
    if (newValue == oldValue)
        return QuestChange::None;

    auto questIt = quests.find(varId);
    if (questIt == quests.end())
        return QuestChange::None;

    const Quest &quest = questIt->second;

    auto [oldQuestEntries, oldCompletedEntries] = countQuestEntries(quest, oldValue);
    auto [newQuestEntries, newCompletedEntries] = countQuestEntries(quest, newValue);

    if (newCompletedEntries > oldCompletedEntries)
        return QuestChange::Completed;
    if (newQuestEntries > oldQuestEntries)
        return QuestChange::New;
    return QuestChange::None;
}

} // namespace QuestDB
