/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include <libxml/tree.h>

#include "iteminfo.h"
#include "itemmanager.h"
#include "resourcemanager.h"

#include "../log.h"

ItemManager::ItemManager()
{
    ResourceManager *resman = ResourceManager::getInstance();
    int size;
    char *data = (char*)resman->loadFile("items.xml", size);

    if (!data) {
        logger->error("Item Manager: Could not find items.xml!");
    }

    xmlDocPtr doc = xmlParseMemory(data, size);
    free(data);

    if (doc)
    {
        xmlNodePtr node = xmlDocGetRootElement(doc);

        if (!node || !xmlStrEqual(node->name, BAD_CAST "items"))
        {
            logger->error("Item Manager: items.xml is not a valid database file!");
        }
        else
        {
            for (node = node->xmlChildrenNode; node != NULL; node = node->next)
            {
                int id = 0, image = 0, art = 0, type = 0, weight = 0, slot = 0;
                std::string name = "", description = "", effect = "";

                if (xmlStrEqual(node->name, BAD_CAST "item"))
                {
                    xmlChar *prop = NULL;
                    // Item id
                    prop = xmlGetProp(node, BAD_CAST "id");
                    if (prop) id = atoi((const char*)prop);
                    xmlFree(prop);
                    // Image id
                    prop = xmlGetProp(node, BAD_CAST "image");
                    if (prop) image = atoi((const char*)prop);
                    xmlFree(prop);
                    // Art id
                    prop = xmlGetProp(node, BAD_CAST "art");
                    if (prop) art = atoi((const char*)prop);
                    xmlFree(prop);
                    // Name
                    prop = xmlGetProp(node, BAD_CAST "name");
                    if (prop) name = (const char*)prop;
                    xmlFree(prop);
                    // Description
                    prop = xmlGetProp(node, BAD_CAST "description");
                    if (prop) description = (const char*)prop;
                    xmlFree(prop);
                    // Effect
                    prop = xmlGetProp(node, BAD_CAST "effect");
                    if (prop) effect = (const char*)prop;
                    xmlFree(prop);
                    // Type id
                    prop = xmlGetProp(node, BAD_CAST "type");
                    if (prop) type = atoi((const char*)prop);
                    xmlFree(prop);
                    // Weight
                    prop = xmlGetProp(node, BAD_CAST "weight");
                    if (prop) weight = atoi((const char*)prop);
                    xmlFree(prop);
                    // Slot
                    prop = xmlGetProp(node, BAD_CAST "slot");
                    if (prop) slot = atoi((const char*)prop);
                    xmlFree(prop);

                    if (id && name != "")
                    {
                        ItemInfo *itemInfo = new ItemInfo();
                        itemInfo->setImage(image);
                        itemInfo->setArt(art);
                        itemInfo->setName(name);
                        itemInfo->setDescription(description);
                        itemInfo->setEffect(effect);
                        itemInfo->setType(type);
                        itemInfo->setWeight(weight);
                        itemInfo->setSlot(slot);
                        db[id] = itemInfo;
                    }

                    if (id == 0)
                    {
                        logger->log("Item Manager: An item has no ID in items.xml!");
                    }
                    if (name == "")
                    {
                        logger->log("Item Manager: An item has no name in items.xml!");
                    }

                    if (image == 0)
                    {
                        logger->log("Item Manager: Missing image parameter for item: %i. %s",
                        id, name.c_str());
                    }
                    if (art == 0)
                    {
                        logger->log("Item Manager: Missing art parameter for item: %i. %s",
                        id, name.c_str());
                    }
                    if (description == "")
                    {
                        logger->log("Item Manager: Missing description parameter for item: %i. %s",
                        id, name.c_str());
                    }
                    if (effect == "")
                    {
                        logger->log("Item Manager: Missing effect parameter for item: %i. %s",
                        id, name.c_str());
                    }
                    if (type == 0)
                    {
                        logger->log("Item Manager: Missing type parameter for item: %i. %s",
                        id, name.c_str());
                    }
                    if (weight == 0)
                    {
                        logger->log("Item Manager: Missing weight parameter for item: %i. %s",
                        id, name.c_str());
                    }
                    if (slot == 0)
                    {
                        logger->log("Item Manager: Missing image parameter for item: %i. %s",
                        id, name.c_str());
                    }

                    /*logger->log("Item: %i %i %i %s %s %i %i %i", id,
                        getImage(id), getArt(id), getName(id).c_str(),
                        getDescription(id).c_str(), getType(id), getWeight(id),
                        getSlot(id));*/
                }
            }
        }

        xmlFreeDoc(doc);
    }
    else {
        logger->error("Item Manager: Error while parsing item database (items.xml)!");
    }

    unknown = new ItemInfo();
    unknown->setName("Unknown item");
}

ItemManager::~ItemManager()
{
    std::map<int, ItemInfo*>::iterator i;
    for (i = db.begin(); i != db.end(); i++)
    {
        delete (*i).second;
    }
    db.clear();

    delete unknown;
}

ItemInfo*
ItemManager::getItemInfo(int id)
{
    if (db.find(id) != db.end())
    {
        return db[id];
    }
    else
    {
        return unknown;
    }
}
