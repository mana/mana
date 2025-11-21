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

#include "resources/attributes.h"

#include "log.h"
#include "playerinfo.h"

#include "gui/statuswindow.h"

#include "resources/itemdb.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <list>
#include <map>

#define DEFAULT_ATTRIBUTESDB_FILE "attributes.xml"
#define DEFAULT_POINTS 30
#define DEFAULT_MIN_PTS 1
#define DEFAULT_MAX_PTS 9

namespace Attributes {

    struct Attribute
    {
        unsigned int id;
        std::string name;
        std::string description;
        /** Whether the attribute value can be modified by the player */
        bool modifiable;
        /**< Attribute scope. */
        std::string scope;
        /** The playerInfo core Id the attribute is linked with or -1 if not */
        int playerInfoId;
    };

    /** Map for attributes. */
    static std::map<unsigned int, Attribute> attributes;

    /** tags = effects on attributes. */
    static std::map<std::string, std::string> tags;

    /** List of modifiable attribute names used at character's creation. */
    static std::vector<std::string> attributeLabels;

    /** Characters creation points. */
    static unsigned int creationPoints = DEFAULT_POINTS;
    static unsigned int attributeMinimum = DEFAULT_MIN_PTS;
    static unsigned int attributeMaximum = DEFAULT_MAX_PTS;

    unsigned int getCreationPoints()
    {
        return creationPoints;
    }

    unsigned int getAttributeMinimum()
    {
        return attributeMinimum;
    }

    unsigned int getAttributeMaximum()
    {
        return attributeMaximum;
    }

    std::vector<std::string>& getLabels()
    {
        return attributeLabels;
    }

    /**
     * Fills the list of base attribute labels.
     */
    static void fillLabels()
    {
        // Fill up the modifiable attribute label list.
        attributeLabels.clear();
        for (const auto &[_, attribute] : attributes)
        {
            if (attribute.modifiable &&
               (attribute.scope == "character" || attribute.scope == "being"))
                attributeLabels.push_back(attribute.name + ":");
        }
    }

    /**
     * Fills the list of base attribute labels.
     */
    static int getPlayerInfoIdFromAttrType(std::string attrType)
    {
        toLower(attrType);
        if (attrType == "level")
            return ::LEVEL;
        else if (attrType == "hp")
            return ::HP;
        else if (attrType == "max-hp")
            return ::MAX_HP;
        else if (attrType == "mp")
            return ::MP;
        else if (attrType == "max-mp")
            return ::MAX_MP;
        else if (attrType == "exp")
            return ::EXP;
        else if (attrType == "exp-needed")
            return ::EXP_NEEDED;
        else if (attrType == "money")
            return ::MONEY;
        else if (attrType == "total-weight")
            return ::TOTAL_WEIGHT;
        else if (attrType == "max-weight")
            return ::MAX_WEIGHT;
        else if (attrType == "skill-points")
            return ::SKILL_POINTS;
        else if (attrType == "char-points")
            return ::CHAR_POINTS;
        else if (attrType == "corr-points")
            return ::CORR_POINTS;
        else if (attrType == "none")
            return -2; // Used to hide the attribute display.

        return -1; // Not linked to a playerinfo stat.
    }

    int getPlayerInfoIdFromAttrId(int attrId)
    {
        auto it = attributes.find(attrId);
        if (it != attributes.end())
        {
            return it->second.playerInfoId;
        }

        return -1;
    }

    static void loadBuiltins()
    {
        {
            Attribute a;
            a.id = 16;
            a.name = _("Strength");
            a.modifiable = true;
            a.scope = "character";
            a.playerInfoId = -1;

            attributes[a.id] = a;
            tags.insert(std::make_pair("str", _("Strength %+.1f")));
        }

        {
            Attribute a;
            a.id = 17;
            a.name = _("Agility");
            a.modifiable = true;
            a.scope = "character";
            a.playerInfoId = -1;

            attributes[a.id] = a;
            tags.insert(std::make_pair("agi", _("Agility %+.1f")));
        }

        {
            Attribute a;
            a.id = 18;
            a.name = _("Dexterity");
            a.modifiable = true;
            a.scope = "character";
            a.playerInfoId = -1;

            attributes[a.id] = a;
            tags.insert(std::make_pair("dex", _("Dexterity %+.1f")));
        }

        {
            Attribute a;
            a.id = 19;
            a.name = _("Vitality");
            a.modifiable = true;
            a.scope = "character";
            a.playerInfoId = -1;

            attributes[a.id] = a;
            tags.insert(std::make_pair("vit", _("Vitality %+.1f")));
        }

        {
            Attribute a;
            a.id = 20;
            a.name = _("Intelligence");
            a.modifiable = true;
            a.scope = "character";
            a.playerInfoId = -1;

            attributes[a.id] = a;
            tags.insert(std::make_pair("int", _("Intelligence %+.1f")));
        }

        {
            Attribute a;
            a.id = 21;
            a.name = _("Willpower");
            a.modifiable = true;
            a.scope = "character";
            a.playerInfoId = -1;

            attributes[a.id] = a;
            tags.insert(std::make_pair("wil", _("Willpower %+.1f")));
        }
    }

    void init()
    {
        if (!attributes.empty())
            unload();
    }

    /**
     * Read attribute node
     */
    void readAttributeNode(XML::Node node, const std::string &filename)
    {
        int id = node.getProperty("id", 0);
        if (!id)
        {
            Log::info("Attributes: Invalid or missing stat ID in "
                      DEFAULT_ATTRIBUTESDB_FILE "!");
            return;
        }

        if (attributes.find(id) != attributes.end())
        {
            Log::info("Attributes: Redefinition of stat ID %d", id);
        }

        std::string name = node.getProperty("name", "");

        if (name.empty())
        {
            Log::info("Attributes: Invalid or missing stat name in "
                      DEFAULT_ATTRIBUTESDB_FILE "!");
            return;
        }

        // Create the attribute.
        Attribute &a = attributes[id];
        a.id = id;
        a.name = name;
        a.description = node.getProperty("desc", std::string());
        a.modifiable = node.getBoolProperty("modifiable", false);
        a.scope = node.getProperty("scope", "none");
        a.playerInfoId = getPlayerInfoIdFromAttrType(
                         node.getProperty("player-info", ""));

        unsigned int count = 0;
        for (auto effectNode : node.children())
        {
            if (effectNode.name() != "modifier")
                 continue;
            ++count;
            std::string tag = effectNode.getProperty("tag", "");
            if (tag.empty())
            {
                if (name.empty())
                {
                    Log::info("Attribute modifier in attribute %u:%s: "
                              "Empty name definition "
                              "on empty tag definition, skipping.",
                              a.id, a.name.c_str());
                    --count;
                    continue;
                }
                tag = name.substr(0, name.size() > 3 ? 3 : name.size());
                tag = toLower(tag) + toString(count);
             }

            std::string effect = effectNode.getProperty("effect", "");
            if (effect.empty())
             {
                if (name.empty())
                {
                    Log::info("Attribute modifier in attribute %u:%s: "
                              "Empty name definition "
                              "on empty effect definition, skipping.",
                              a.id, a.name.c_str());
                    --count;
                    continue;
                }
                else
                    effect = name + " %+f";
             }
            tags.insert(std::make_pair(tag, effect));
         }
        Log::info("Found %d tags for attribute %d.", count, id);
    }

    /**
     * Read points node
     */
    void readPointsNode(XML::Node node, const std::string &filename)
    {
        creationPoints = node.getProperty("start",DEFAULT_POINTS);
        attributeMinimum = node.getProperty("minimum",
                                                       DEFAULT_MIN_PTS);
        attributeMaximum = node.getProperty("maximum",
                                                       DEFAULT_MAX_PTS);
        Log::info("Loaded points: start: %i, min: %i, max: %i.",
                  creationPoints, attributeMinimum, attributeMaximum);
    }

    /**
     * Check if all the data loaded by readPointsNode and readAttributeNode is ok
     */
    void checkStatus()
    {
        Log::info("Found %d tags for %d attributes.", int(tags.size()),
                                                      int(attributes.size()));

        if (attributes.empty())
            loadBuiltins();

        fillLabels();

        // Sanity checks on starting points
        auto modifiableAttributeCount = (float) attributeLabels.size();
        float averageValue = ((float) creationPoints) / modifiableAttributeCount;
        if (averageValue > attributeMaximum || averageValue < attributeMinimum
            || creationPoints < 1)
        {
            Log::info("Attributes: Character's point values make "
                      "the character's creation impossible. "
                      "Switch back to defaults.");
            creationPoints = DEFAULT_POINTS;
            attributeMinimum = DEFAULT_MIN_PTS;
            attributeMaximum = DEFAULT_MAX_PTS;
        }
    }

    void unload()
    {
        attributes.clear();
    }

    void informItemDB()
    {
        std::list<ItemStat> dbStats;

        for (const auto &[tag, format] : tags)
            dbStats.emplace_back(tag, format);

        setStatsList(std::move(dbStats));
    }

    void informStatusWindow()
    {
        for (const auto &[_, attribute] : attributes)
        {
            if (attribute.playerInfoId == -1 &&
                (attribute.scope == "character" || attribute.scope == "being"))
            {
                statusWindow->addAttribute(attribute.id,
                                           attribute.name,
                                           attribute.modifiable,
                                           attribute.description);
            }
        }
    }

} // namespace Attributes
