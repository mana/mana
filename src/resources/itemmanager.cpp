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
 
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <iostream>

#include "itemmanager.h"
#include "../main.h"
#include "../log.h"

// MSVC libxml2 at the moment doesn't work right when using MinGW, missing this
// function at link time.
#ifdef WIN32
#undef xmlFree
#define xmlFree(x) ;
#endif

ItemManager::ItemManager()
{
    // Check that file exists before trying to parse it
    std::fstream dbFile;
    dbFile.open("./data/items.xml", std::ios::in);
    if (!dbFile.is_open()) {
        logger.log("Cannot find item database!");
        return;
    }
    dbFile.close();
    
    xmlDocPtr doc = xmlParseFile("./data/items.xml");

    if (doc) {
        xmlNodePtr node = xmlDocGetRootElement(doc);

        if (!node || !xmlStrEqual(node->name, BAD_CAST "items")) {
            logger.log("Warning: Not a valid database file!");
        } else {
            for (node = node->xmlChildrenNode; node != NULL; node = node->next)
            {
                if (xmlStrEqual(node->name, BAD_CAST "item"))
                {
                    xmlChar *prop;
                    prop = xmlGetProp(node, BAD_CAST "id");
                    int id = atoi((const char*)prop);
                    xmlFree(prop);
                    prop = xmlGetProp(node, BAD_CAST "image");
                    int image = atoi((const char*)prop);
                    xmlFree(prop);
                    prop = xmlGetProp(node, BAD_CAST "art");
                    int art = atoi((const char*)prop);
                    xmlFree(prop);
                    prop = xmlGetProp(node, BAD_CAST "name");
                    std::string name((const char*)prop);
                    xmlFree(prop);
                    prop = xmlGetProp(node, BAD_CAST "description");
                    std::string description((const char*)prop);
                    xmlFree(prop);
                    prop = xmlGetProp(node, BAD_CAST "type");
                    int type = atoi((const char*)prop);
                    xmlFree(prop);
                    prop = xmlGetProp(node, BAD_CAST "weight");
                    int weight = atoi((const char*)prop);
                    xmlFree(prop);
                    prop = xmlGetProp(node, BAD_CAST "slot");
                    int slot = atoi((const char*)prop);
                    xmlFree(prop);
                    
                    ItemInfo *itemInfo = new ItemInfo();
                    itemInfo->setImage(image);
                    itemInfo->setArt(art);
                    itemInfo->setName(name);
                    itemInfo->setDescription(description);
                    itemInfo->setType(type);
                    itemInfo->setWeight(weight);
                    itemInfo->setSlot(slot);
                    db[id] = itemInfo;
                    
                    /*logger.log("Item: %i %i %i %s %s %i %i %i", id,
                        getImage(id), getArt(id), getName(id).c_str(),
                        getDescription(id).c_str(), getType(id), getWeight(id),
                        getSlot(id));*/
                }
            }
        }
        
        xmlFreeDoc(doc);
    } else {
        logger.log("Error while parsing item database!");
    }
}

ItemManager::~ItemManager()
{
}

ItemInfo *ItemManager::getItemInfo(int id)
{
    if (db.find(id) != db.end())
        return db[id];
    else
        return NULL;
}

