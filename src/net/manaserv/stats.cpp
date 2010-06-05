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

#include "net/manaserv/stats.h"

#include "log.h"

#include "gui/statuswindow.h"

#include "resources/itemdb.h"

#include "utils/gettext.h"
#include "utils/xml.h"

#include <list>
#include <map>

namespace ManaServ {
namespace Stats {
    typedef struct {
        unsigned int id;
        std::string name;
        std::string tag;
        std::string effect;
        std::string description;
        bool modifiable;
    } Stat;

    typedef std::map<unsigned int, Stat> StatMap;
    StatMap stats;

    static void loadBuiltins()
    {
        {
            Stat s;
            s.id = 16;
            s.name = _("Strength");
            s.tag = "str";
            s.effect = _("Strength %+d");
            s.description = "";
            s.modifiable = true;

            stats[s.id] = s;
        }

        {
            Stat s;
            s.id = 17;
            s.name = _("Agility");
            s.tag = "agi";
            s.effect = _("Agility %+d");
            s.description = "";
            s.modifiable = true;

            stats[s.id] = s;
        }

        {
            Stat s;
            s.id = 18;
            s.name = _("Dexterity");
            s.tag = "dex";
            s.effect = _("Dexterity %+d");
            s.description = "";
            s.modifiable = true;

            stats[s.id] = s;
        }

        {
            Stat s;
            s.id = 19;
            s.name = _("Vitality");
            s.tag = "vit";
            s.effect = _("Vitality %+d");
            s.description = "";
            s.modifiable = true;

            stats[s.id] = s;
        }

        {
            Stat s;
            s.id = 20;
            s.name = _("Intelligence");
            s.tag = "int";
            s.effect = _("Intelligence %+d");
            s.description = "";
            s.modifiable = true;

            stats[s.id] = s;
        }

        {
            Stat s;
            s.id = 21;
            s.name = _("Willpower");
            s.tag = "will";
            s.effect = _("Willpower %+d");
            s.description = "";
            s.modifiable = true;

            stats[s.id] = s;
        }
    }

    void load()
    {
        XML::Document doc("stats.xml");
        xmlNodePtr rootNode = doc.rootNode();

        if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "stats"))
        {
            logger->log("Stats: Error while loading stats.xml!");
            loadBuiltins();
            return;
        }

        for_each_xml_child_node(node, rootNode)
        {
            if (!xmlStrEqual(node->name, BAD_CAST "stat"))
                continue;

            int id = XML::getProperty(node, "id", 0);

            if (id == 0)
            {
                logger->log("Stats: Invalid or missing stat ID in stats.xml!");
                continue;
            }
            else if (stats.find(id) != stats.end())
            {
                logger->log("Stats: Redefinition of stat ID %d", id);
            }

            std::string name = XML::getProperty(node, "name", "");

            if (name.empty())
            {
                logger->log("Stats: Invalid or missing stat name in "
                            "stats.xml!");
                continue;
            }

            Stat s;
            s.id = id;
            s.name = name;
            s.tag = XML::getProperty(node, "tag", "");
            s.effect = XML::getProperty(node, "effect", "");
            s.description = XML::getProperty(node, "desc", "");
            s.modifiable = XML::getProperty(node, "modifiable", "false")
                           == "true";

            stats[id] = s;
        }
    }

    void unload()
    {
        stats.clear();
    }

    void informItemDB()
    {
        std::list<ItemDB::Stat> dbStats;

        StatMap::const_iterator it, it_end;
        for (it = stats.begin(), it_end = stats.end(); it != it_end; it++)
            if (!it->second.tag.empty())
                dbStats.push_back(ItemDB::Stat(it->second.tag,
                                               it->second.effect));

        ItemDB::setStatsList(dbStats);
    }

    void informStatusWindow()
    {
        StatMap::const_iterator it, it_end;
        for (it = stats.begin(), it_end = stats.end(); it != it_end; it++)
            statusWindow->addAttribute(it->second.id, it->second.name,
                                       it->second.modifiable,
                                       it->second.description);
    }

    std::vector<std::string> getLabelVector()
    {
        std::vector<std::string> attributes;
        StatMap::const_iterator it, it_end;
        for (it = stats.begin(), it_end = stats.end(); it != it_end; it++)
            if (it->second.modifiable)
                attributes.push_back(it->second.name + ":");

        return attributes;
    }
} // namespace Stats
} // namespace ManaServ
