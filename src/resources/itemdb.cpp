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
 *  $Id: itemmanager.cpp 2650 2006-09-03 15:00:47Z b_lindeijer $
 */

#include "itemdb.h"

#include <libxml/tree.h>

#include "iteminfo.h"
#include "resourcemanager.h"

#include "../log.h"

#include "../utils/dtor.h"

#define READ_PROP(node, prop, name, target, cast) \
        prop = xmlGetProp(node, BAD_CAST name); \
        if (prop) { \
            target = cast((const char*)prop); \
            xmlFree(prop); \
        }
namespace ItemDB
{
    ItemInfos mItemInfos;
    ItemInfo mUnknown;
}


void ItemDB::load()
{
    mUnknown.setName("Unknown item");

    ResourceManager *resman = ResourceManager::getInstance();
    int size;
    char *data = (char*)resman->loadFile("items.xml", size);

    if (!data) {
        logger->error("Item Manager: Could not find items.xml!");
    }

    xmlDocPtr doc = xmlParseMemory(data, size);
    free(data);

    if (!doc)
    {
        logger->error("Item Manager: Error while parsing item database (items.xml)!");
    }

    xmlNodePtr node = xmlDocGetRootElement(doc);
    if (!node || !xmlStrEqual(node->name, BAD_CAST "items"))
    {
        logger->error("Item Manager: items.xml is not a valid database file!");
    }

    for (node = node->xmlChildrenNode; node != NULL; node = node->next)
    {
        int id = 0, art = 0, type = 0, weight = 0, slot = 0;
        std::string name = "", description = "", effect = "", image = "";

        if (!xmlStrEqual(node->name, BAD_CAST "item")) {
            continue;
        }

        xmlChar *prop = NULL;
        READ_PROP(node, prop, "id", id, atoi);
        READ_PROP(node, prop, "image", image, );
        READ_PROP(node, prop, "art", art, atoi);
        READ_PROP(node, prop, "name", name, );
        READ_PROP(node, prop, "description", description, );
        READ_PROP(node, prop, "effect", effect, );
        READ_PROP(node, prop, "type", type, atoi);
        READ_PROP(node, prop, "weight", weight, atoi);
        READ_PROP(node, prop, "slot", slot, atoi);


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
            logger->log("Item Manager: An item has no ID in items.xml!");
        }
        if (name == "")
        {
            logger->log("Item Manager: An item has no name in items.xml!");
        }

        if (image == "")
        {
            logger->log("Item Manager: Missing image parameter for item: %i. %s",
                    id, name.c_str());
        }
        /*if (art == 0)
        {
            logger->log("Item Manager: Missing art parameter for item: %i. %s",
                    id, name.c_str());
        }*/
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
        /*if (type == 0)
        {
            logger->log("Item Manager: Missing type parameter for item: %i. %s",
                    id, name.c_str());
        }*/
        if (weight == 0)
        {
            logger->log("Item Manager: Missing weight parameter for item: %i. %s",
                    id, name.c_str());
        }
        if (slot == 0)
        {
            logger->log("Item Manager: Missing slot parameter for item: %i. %s",
                    id, name.c_str());
        }

        /*logger->log("Item: %i %i %i %s %s %i %i %i", id,
          getImage(id), getArt(id), getName(id).c_str(),
          getDescription(id).c_str(), getType(id), getWeight(id),
          getSlot(id));*/
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
