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
 *  $Id: itemdb.cpp 4347 2008-06-12 09:06:01Z b_lindeijer $
 */

#include <cassert>
#include <libxml/tree.h>

#include "itemdb.h"

#include "iteminfo.h"
#include "resourcemanager.h"

#include "../log.h"

#include "../utils/dtor.h"
#include "../utils/xml.h"

namespace
{
    ItemDB::ItemInfos mItemInfos;
    ItemInfo *mUnknown;
    bool mLoaded = false;
}

// Forward declarations
static void loadSpriteRef(ItemInfo *itemInfo, xmlNodePtr node);
static void loadSoundRef(ItemInfo *itemInfo, xmlNodePtr node);

void ItemDB::load()
{
    if (mLoaded)
        return;

    logger->log("Initializing item database...");

    mUnknown = new ItemInfo();
    mUnknown->setName("Unknown item");
    mUnknown->setImageName("");
    mUnknown->setSprite("error.xml", 0);
    mUnknown->setSprite("error.xml", 1);

    XML::Document doc("items.xml");
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "items"))
    {
        logger->error("ItemDB: Error while loading items.xml!");
    }

    for_each_xml_child_node(node, rootNode)
    {
        if (!xmlStrEqual(node->name, BAD_CAST "item"))
            continue;

        int id = XML::getProperty(node, "id", 0);

        if (id == 0)
        {
            logger->log("ItemDB: Invalid or missing item ID in items.xml!");
            continue;
        }
        else if (mItemInfos.find(id) != mItemInfos.end())
        {
            logger->log("ItemDB: Redefinition of item ID %d", id);
        }

        int type = XML::getProperty(node, "type", 0);
        //int weight = XML::getProperty(node, "weight", 0);
        int view = XML::getProperty(node, "view", 0);

        std::string name = XML::getProperty(node, "name", "");
        std::string image = XML::getProperty(node, "image", "");
        std::string description = XML::getProperty(node, "description", "");
        std::string effect = XML::getProperty(node, "effect", "");
        //int weaponType = XML::getProperty(node, "weapon_type", 0);// Not used by Aethyra

        if (id)
        {
            ItemInfo *itemInfo = new ItemInfo();
            itemInfo->setImageName(image);
            itemInfo->setName((name == "") ? "Unnamed" : name);
            itemInfo->setDescription(description);
            itemInfo->setEffect(effect);
            itemInfo->setType(type);
            itemInfo->setView(view);
            //itemInfo->setWeight(weight);
            //itemInfo->setWeaponType(weaponType);

            for_each_xml_child_node(itemChild, node)
            {
                if (xmlStrEqual(itemChild->name, BAD_CAST "sprite"))
                {
                    loadSpriteRef(itemInfo, itemChild);
                }
                else if (xmlStrEqual(itemChild->name, BAD_CAST "sound"))
                {
                    loadSoundRef(itemInfo, itemChild);
                }
            }

            mItemInfos[id] = itemInfo;
        }

#define CHECK_PARAM(param, error_value) \
        if (param == error_value) \
            logger->log("ItemDB: Missing " #param " attribute for item %i!",id)

        CHECK_PARAM(name, "");
        CHECK_PARAM(image, "");
        CHECK_PARAM(description, "");
        // CHECK_PARAM(effect, "");
        // CHECK_PARAM(type, 0);
        // CHECK_PARAM(weight, 0);
        // CHECK_PARAM(slot, 0);

#undef CHECK_PARAM
    }

    mLoaded = true;
}

void ItemDB::unload()
{
    logger->log("Unloading item database...");

    delete mUnknown;
    mUnknown = NULL;

    delete_all(mItemInfos);
    mItemInfos.clear();
    mLoaded = false;
}

const ItemInfo& ItemDB::get(int id)
{
    assert(mLoaded);

    ItemInfoIterator i = mItemInfos.find(id);

    if (i == mItemInfos.end())
    {
        logger->log("ItemDB: Error, unknown item ID# %d", id);
        return *mUnknown;
    }
    else
    {
        return *(i->second);
    }
}

void loadSpriteRef(ItemInfo *itemInfo, xmlNodePtr node)
{
    std::string gender = XML::getProperty(node, "gender", "unisex");
    std::string filename = (const char*) node->xmlChildrenNode->content;

    if (gender == "male" || gender == "unisex")
    {
        itemInfo->setSprite(filename, 0);
    }
    if (gender == "female" || gender == "unisex")
    {
        itemInfo->setSprite(filename, 1);
    }
}

void loadSoundRef(ItemInfo *itemInfo, xmlNodePtr node)
{
    std::string event = XML::getProperty(node, "event", "");
    std::string filename = (const char*) node->xmlChildrenNode->content;

    if (event == "hit")
    {
        itemInfo->addSound(EQUIP_EVENT_HIT, filename);
    }
    else if (event == "strike")
    {
        itemInfo->addSound(EQUIP_EVENT_STRIKE, filename);
    }
    else
    {
        logger->log("ItemDB: Ignoring unknown sound event '%s'",
                event.c_str());
    }
}
