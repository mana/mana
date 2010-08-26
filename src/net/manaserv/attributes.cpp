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

#include "net/manaserv/attributes.h"

#include "log.h"

#include "gui/statuswindow.h"

#include "resources/itemdb.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <list>
#include <map>

#define DEFAULT_ATTRIBUTESDB_FILE "attributes.xml"
#define DEFAULT_POINTS 60
#define DEFAULT_MIN_PTS 1
#define DEFAULT_MAX_PTS 20

namespace ManaServ {
namespace Attributes {
    typedef struct {
        unsigned int id;
        std::string name;
        std::string description;
        bool modifiable;
    } Attribute;

    // tag -> effect
    typedef std::map< std::string, std::string > TagMap;

    typedef std::map<unsigned int, Attribute> AttributeMap;
    AttributeMap attributes;

    TagMap tags;

    // List of modifiable attribute names used at character's creation.
    static std::vector<std::string> attributeLabels;

    static unsigned int creationPoints = 0;
    static unsigned int attributeMinimum = 0;
    static unsigned int attributeMaximum = 0;

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

    static void loadBuiltins()
    {
        {
            Attribute a;
            a.id = 16;
            a.name = _("Strength");
            a.description = "";
            a.modifiable = true;

            attributes[a.id] = a;
            tags.insert(std::make_pair("str", _("Strength %+.1f")));
        }

        {
            Attribute a;
            a.id = 17;
            a.name = _("Agility");
            a.description = "";
            a.modifiable = true;

            attributes[a.id] = a;
            tags.insert(std::make_pair("agi", _("Agility %+.1f")));
        }

        {
            Attribute a;
            a.id = 18;
            a.name = _("Dexterity");
            a.description = "";
            a.modifiable = true;

            attributes[a.id] = a;
            tags.insert(std::make_pair("dex", _("Dexterity %+.1f")));
        }

        {
            Attribute a;
            a.id = 19;
            a.name = _("Vitality");
            a.description = "";
            a.modifiable = true;

            attributes[a.id] = a;
            tags.insert(std::make_pair("vit", _("Vitality %+.1f")));
        }

        {
            Attribute a;
            a.id = 20;
            a.name = _("Intelligence");
            a.description = "";
            a.modifiable = true;

            attributes[a.id] = a;
            tags.insert(std::make_pair("int", _("Intelligence %+.1f")));
        }

        {
            Attribute a;
            a.id = 21;
            a.name = _("Willpower");
            a.description = "";
            a.modifiable = true;

            attributes[a.id] = a;
            tags.insert(std::make_pair("wil", _("Willpower %+.1f")));
        }
    }

    void load()
    {
        logger->log("Initializing attributes database...");

        XML::Document doc(DEFAULT_ATTRIBUTESDB_FILE);
        xmlNodePtr rootNode = doc.rootNode();

        if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "attributes"))
        {
            logger->log("Attributes: Error while loading "
                        DEFAULT_ATTRIBUTESDB_FILE ". Using Built-ins.");
            loadBuiltins();
            return;
        }

        for_each_xml_child_node(node, rootNode)
        {
            if (xmlStrEqual(node->name, BAD_CAST "attribute"))
            {
                int id = XML::getProperty(node, "id", 0);

                if (!id)
                {
                    logger->log("Attributes: Invalid or missing stat ID in "
                                DEFAULT_ATTRIBUTESDB_FILE "!");
                    continue;
                }
                else if (attributes.find(id) != attributes.end())
                {
                    logger->log("Attributes: Redefinition of stat ID %d", id);
                }

                std::string name = XML::getProperty(node, "name", "");

                if (name.empty())
                {
                    logger->log("Attributes: Invalid or missing stat name in "
                                DEFAULT_ATTRIBUTESDB_FILE "!");
                    continue;
                }

                Attribute a;
                a.id = id;
                a.name = name;
                a.description = XML::getProperty(node, "desc", "");
                a.modifiable = XML::getProperty(node, "modifiable", "false")
                              == "true";

                attributes[id] = a;

                unsigned int count = 0;
                for_each_xml_child_node(effectNode, node)
                 {
                    if (!xmlStrEqual(effectNode->name, BAD_CAST "modifier"))
                         continue;
                    ++count;
                    std::string tag = XML::getProperty(effectNode, "tag", "");
                    if (tag.empty())
                    {
                        if (name.empty())
                        {
                            logger->log("Attribute modifier in attribute %u:%s: "
                                        "Empty name definition "
                                        "on empty tag definition, skipping.",
                                        a.id, a.name.c_str());
                            --count;
                            continue;
                        }
                        tag = name.substr(0, name.size() > 3 ? 3 : name.size());
                        tag = toLower(tag) + toString(count);
                     }

                    std::string effect = XML::getProperty(effectNode, "effect", "");
                    if (effect.empty())
                     {
                        if (name.empty())
                        {
                            logger->log("Attribute modifier in attribute %u:%s: "
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
                logger->log("Found %d tags for attribute %d.", count, id);

            }// End attribute
            else if (xmlStrEqual(node->name, BAD_CAST "points"))
            {
                creationPoints = XML::getProperty(node, "start",DEFAULT_POINTS);
                attributeMinimum = XML::getProperty(node, "minimum",
                                                               DEFAULT_MIN_PTS);
                attributeMaximum = XML::getProperty(node, "maximum",
                                                               DEFAULT_MAX_PTS);
                logger->log("Loaded points: start: %i, min: %i, max: %i.",
                            creationPoints, attributeMinimum, attributeMaximum);
            }
            else
            {
                continue;
            }
        }
        logger->log("Found %d tags for %d attributes.", int(tags.size()),
                                                        int(attributes.size()));

        // Fill up the modifiable attribute label list.
        attributeLabels.clear();
        AttributeMap::const_iterator it, it_end;
        for (it = attributes.begin(), it_end = attributes.end(); it != it_end;
                                                                           it++)
        {
            if (it->second.modifiable)
                attributeLabels.push_back(it->second.name + ":");
        }

        // Sanity checks on starting points
        float modifiableAttributeCount = (float) attributeLabels.size();
        float averageValue = ((float) creationPoints) / modifiableAttributeCount;
        if (averageValue > attributeMaximum || averageValue < attributeMinimum
            || creationPoints < 1)
        {
            logger->log("Attributes: Character's point values make "
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
        std::list<ItemDB::Stat> dbStats;

        TagMap::const_iterator it, it_end;
        for (it = tags.begin(), it_end = tags.end(); it != it_end; ++it)
            dbStats.push_back(ItemDB::Stat(it->first,
                                           it->second));

        itemDb->setStatsList(dbStats);
    }

    void informStatusWindow()
    {
        AttributeMap::const_iterator it, it_end;
        for (it = attributes.begin(), it_end = attributes.end(); it != it_end;
                                                                           it++)
            statusWindow->addAttribute(it->second.id, it->second.name,
                                       it->second.modifiable,
                                       it->second.description);
    }

} // namespace Attributes
} // namespace ManaServ
