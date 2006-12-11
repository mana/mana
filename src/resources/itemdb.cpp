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
 *  $Id:
 */

#include "itemdb.h"

#include <libxml/tree.h>

#include "iteminfo.h"
#include "resourcemanager.h"

#include "../log.h"

#include "../utils/dtor.h"
#include "../utils/xml.h"

namespace
{
    ItemDB::ItemInfos mItemInfos;
    ItemInfo mUnknown;
}


void ItemDB::load()
{
    logger->log("Initializing item database...");
    mUnknown.setName("Unknown item");

    ResourceManager *resman = ResourceManager::getInstance();
    int size;
    char *data = (char*)resman->loadFile("items.xml", size);

    if (!data) {
        logger->error("ItemDB: Could not find items.xml!");
    }

    xmlDocPtr doc = xmlParseMemory(data, size);
    free(data);

    if (!doc)
    {
        logger->error("ItemDB: Error while parsing item database (items.xml)!");
    }

    xmlNodePtr node = xmlDocGetRootElement(doc);
    if (!node || !xmlStrEqual(node->name, BAD_CAST "items"))
    {
        logger->error("ItemDB: items.xml is not a valid database file!");
    }

    for (node = node->xmlChildrenNode; node != NULL; node = node->next)
    {
        if (!xmlStrEqual(node->name, BAD_CAST "item")) {
            continue;
        }

        int id = XML::getProperty(node, "id", 0);
        int art = XML::getProperty(node, "art", 0);
        int type = XML::getProperty(node, "type", 0);
        int weight = XML::getProperty(node, "weight", 0);
        int slot = XML::getProperty(node, "slot", 0);

        std::string name = XML::getProperty(node, "name", "");
        std::string image = XML::getProperty(node, "image", "");
        std::string description = XML::getProperty(node, "description", "");
        std::string effect = XML::getProperty(node, "effect", "");

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
            mItemInfos[id] = itemInfo;
        }


        if (id == 0)
        {
            logger->log("ItemDB: An item has no ID in items.xml!");
        }
        if (name == "")
        {
            logger->log("ItemDB: Missing name for item %d!", id);
        }

        if (image == "")
        {
            logger->log("ItemDB: Missing image parameter for item: %i. %s",
                    id, name.c_str());
        }
        /*
        if (art == 0)
        {
            logger->log("Item Manager: Missing art parameter for item: %i. %s",
                    id, name.c_str());
        }
        if (description == "")
        {
            logger->log("ItemDB: Missing description parameter for item: %i. %s",
                    id, name.c_str());
        }
        if (effect == "")
        {
            logger->log("ItemDB: Missing effect parameter for item: %i. %s",
                    id, name.c_str());
        }
        if (type == 0)
        {
            logger->log("Item Manager: Missing type parameter for item: %i. %s",
                    id, name.c_str());
        }
        */
        if (weight == 0)
        {
            logger->log("Item Manager: Missing weight parameter for item: %i. %s",
                    id, name.c_str());
        }
        /*
        if (slot == 0)
        {
            logger->log("Item Manager: Missing slot parameter for item: %i. %s",
                    id, name.c_str());
        }
        */
    }

    xmlFreeDoc(doc);
}

void ItemDB::unload()
{
    for (ItemInfoIterator i = mItemInfos.begin(); i != mItemInfos.end(); i++)
    {
        delete i->second;
    }
    mItemInfos.clear();
}

const ItemInfo&
ItemDB::get(int id)
{
    ItemInfoIterator i = mItemInfos.find(id);

    return (i != mItemInfos.end()) ? *(i->second) : mUnknown;
}
