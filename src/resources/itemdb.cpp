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

#include <algorithm>
#include <cassert>
#include <libxml/tree.h>

#include "itemdb.h"

#include "iteminfo.h"
#include "resourcemanager.h"

#include "../log.h"

#include "../utils/dtor.h"
#include "../utils/gettext.h"
#include "../utils/strprintf.h"
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

static char const *const fields[][2] =
{
    { "attack",    N_("Attack %+d")    },
    { "defense",   N_("Defense %+d")   },
    { "hp",        N_("HP %+d")        },
    { "mp",        N_("MP %+d")        }
};

ItemType itemTypeFromString (std::string name, int id = 0)
{
    if      (name=="generic")           return ITEM_UNUSABLE;
    else if (name=="usable")            return ITEM_USABLE;
    else if (name=="equip-1hand")       return ITEM_EQUIPMENT_ONE_HAND_WEAPON;
    else if (name=="equip-2hand")       return ITEM_EQUIPMENT_TWO_HANDS_WEAPON;
    else if (name=="equip-torso")       return ITEM_EQUIPMENT_TORSO;
    else if (name=="equip-arms")        return ITEM_EQUIPMENT_ARMS;
    else if (name=="equip-head")        return ITEM_EQUIPMENT_HEAD;
    else if (name=="equip-legs")        return ITEM_EQUIPMENT_LEGS;
    else if (name=="equip-shield")      return ITEM_EQUIPMENT_SHIELD;
    else if (name=="equip-ring")        return ITEM_EQUIPMENT_RING;
    else if (name=="equip-necklace")    return ITEM_EQUIPMENT_NECKLACE;
    else if (name=="equip-feet")        return ITEM_EQUIPMENT_FEET;
    else if (name=="equip-ammo")        return ITEM_EQUIPMENT_AMMO;
    else return ITEM_UNUSABLE;
}

WeaponType weaponTypeFromString (std::string name, int id = 0)
{
    if      (name=="knife")      return WPNTYPE_KNIFE;
    else if (name=="sword")      return WPNTYPE_SWORD;
    else if (name=="polearm")    return WPNTYPE_POLEARM;
    else if (name=="javelin")    return WPNTYPE_JAVELIN;
    else if (name=="staff")      return WPNTYPE_STAFF;
    else if (name=="whip")       return WPNTYPE_WHIP;
    else if (name=="boomerang")  return WPNTYPE_BOOMERANG;
    else if (name=="bow")        return WPNTYPE_BOW;
    else if (name=="sickle")     return WPNTYPE_SICKLE;
    else if (name=="crossbow")   return WPNTYPE_CROSSBOW;
    else if (name=="mace")       return WPNTYPE_MACE;
    else if (name=="axe")        return WPNTYPE_AXE;
    else if (name=="thrown")     return WPNTYPE_THROWN;
    else return WPNTYPE_NONE;
}

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

    ResourceManager *resman = ResourceManager::getInstance();
    int size;
    char *data = (char*) resman->loadFile("items.xml", size);

    if (!data) {
        logger->error("ItemDB: Could not find items.xml!");
    }

    xmlDocPtr doc = xmlParseMemory(data, size);
    free(data);

    if (!doc)
    {
        logger->error("ItemDB: Error while parsing item database (items.xml)!");
    }

    xmlNodePtr rootNode = xmlDocGetRootElement(doc);
    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "items"))
    {
        logger->error("ItemDB: items.xml is not a valid database file!");
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

        int type = itemTypeFromString(XML::getProperty(node, "type", ""));
        int weight = XML::getProperty(node, "weight", 0);
        int view = XML::getProperty(node, "view", 0);

        std::string name = XML::getProperty(node, "name", "");
        std::string image = XML::getProperty(node, "image", "");
        std::string description = XML::getProperty(node, "description", "");
        int weaponType = weaponTypeFromString(XML::getProperty(node, "weapon-type", ""));

        ItemInfo *itemInfo = new ItemInfo;
        itemInfo->setImageName(image);
        itemInfo->setName(name.empty() ? "Unnamed" : name);
        itemInfo->setDescription(description);
        itemInfo->setType(type);
        itemInfo->setView(view);
        itemInfo->setWeight(weight);
        itemInfo->setWeaponType(weaponType);

        std::string effect;
        for (int i = 0; i < int(sizeof(fields) / sizeof(fields[0])); ++i)
        {
            int value = XML::getProperty(node, fields[i][0], 0);
            if (!value) continue;
            if (!effect.empty()) effect += " / ";
            effect += strprintf(gettext(fields[i][1]), value);
        }
        itemInfo->setEffect(effect);


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

#define CHECK_PARAM(param, error_value) \
        if (param == error_value) \
            logger->log("ItemDB: Missing " #param " attribute for item %i!",id)

        CHECK_PARAM(name, "");
        CHECK_PARAM(image, "");
        // CHECK_PARAM(description, "");
        // CHECK_PARAM(effect, "");
        // CHECK_PARAM(type, 0);
        CHECK_PARAM(weight, 0);
        // CHECK_PARAM(slot, 0);

#undef CHECK_PARAM
    }

    xmlFreeDoc(doc);

    mLoaded = true;
}

void ItemDB::unload()
{
    logger->log("Unloading item database...");

    delete mUnknown;
    mUnknown = NULL;

    for_each(mItemInfos.begin(), mItemInfos.end(), make_dtor(mItemInfos));
    mItemInfos.clear();
    mLoaded = false;
}

const ItemInfo& ItemDB::get(int id)
{
    assert(mLoaded && id);

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
