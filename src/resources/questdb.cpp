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
                logger->log("Warning: effect node for var %d is missing required attributes", varId);
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
                logger->log("Warning: quest node for var %d ('%s') has neither 'complete' nor 'incomplete' values",
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
                    logger->log("Warning: unknown quest row type '%s' for var %d ('%s')",
                                tag.data(), varId, state.name.c_str());
                    continue;
                }

                QuestRow &row = state.rows.emplace_back(rowType);
                row.text = questChild.textContent();
            }
        }
    }
}

void unload()
{
    quests.clear();
}

const Quest &get(int var)
{
    static Quest emptyQuest;
    auto it = quests.find(var);
    return it == quests.end() ? emptyQuest : it->second;
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

    for (auto &[var, quest] : quests)
    {
        auto value = questVars.get(var);

        for (auto &effect : quest.effects)
        {
            if (baseName(effect.map) != mapName)
                continue;
            if (std::find(effect.values.begin(), effect.values.end(), value) == effect.values.end())
                continue;

            activeEffects.set(effect.npcId, effect.statusEffectId);
        }
    }

    return activeEffects;
}

} // namespace QuestDB
