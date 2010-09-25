/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "resources/itemdb.h"

#include "log.h"

#include "net/net.h"

#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"
#include "configuration.h"

#include <libxml/tree.h>

#include <cassert>

// Forward declarations
static char const *const fields[][2] =
{
    { "attack",    N_("Attack %+d")    },
    { "defense",   N_("Defense %+d")   },
    { "hp",        N_("HP %+d")        },
    { "mp",        N_("MP %+d")        }
};

static std::list<ItemDB::Stat> extraStats;

void ItemDB::setStatsList(const std::list<Stat> &stats)
{
    extraStats = stats;
}

static ItemType itemTypeFromString(const std::string &name, int id = 0)
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
    else if (name=="equip-charm")       return ITEM_EQUIPMENT_CHARM;
    else if (name=="equip-necklace")    return ITEM_EQUIPMENT_NECKLACE;
    else if (name=="equip-feet")        return ITEM_EQUIPMENT_FEET;
    else if (name=="equip-ammo")        return ITEM_EQUIPMENT_AMMO;
    else if (name=="racesprite")        return ITEM_SPRITE_RACE;
    else if (name=="hairsprite")        return ITEM_SPRITE_HAIR;
    else return ITEM_UNUSABLE;
}

static std::string normalized(const std::string &name)
{
    std::string normalized = name;
    return toLower(trim(normalized));;
}

void ItemDB::load()
{
    if (mLoaded)
        unload();

    logger->log("Initializing item database...");

    mUnknown = new ItemInfo;
    mUnknown->mName = _("Unknown item");
    mUnknown->mDisplay = SpriteDisplay();
    std::string errFile = paths.getStringValue("spriteErrorFile");
    mUnknown->setSprite(errFile, GENDER_MALE);
    mUnknown->setSprite(errFile, GENDER_FEMALE);

    XML::Document doc("items.xml");
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "items"))
        logger->error("ItemDB: Error while loading items.xml!");

    for_each_xml_child_node(node, rootNode)
    {
        if (!xmlStrEqual(node->name, BAD_CAST "item"))
            continue;

        int id = XML::getProperty(node, "id", 0);

        if (!id)
        {
            logger->log("ItemDB: Invalid or missing item ID in items.xml!");
            continue;
        }
        else if (mItemInfos.find(id) != mItemInfos.end())
            logger->log("ItemDB: Redefinition of item ID %d", id);

        std::string typeStr = XML::getProperty(node, "type", "other");
        int weight = XML::getProperty(node, "weight", 0);
        int view = XML::getProperty(node, "view", 0);

        std::string name = XML::getProperty(node, "name", "");
        std::string image = XML::getProperty(node, "image", "");
        std::string description = XML::getProperty(node, "description", "");
        std::string attackAction = XML::getProperty(node, "attack-action", "");
        int attackRange = XML::getProperty(node, "attack-range", 0);
        std::string missileParticle = XML::getProperty(node, "missile-particle", "");

        SpriteDisplay display;
        display.image = image;

        ItemInfo *itemInfo = new ItemInfo;
        itemInfo->mId = id;
        itemInfo->mName = name.empty() ? _("unnamed") : name;
        itemInfo->mDescription = description;
        itemInfo->mType = itemTypeFromString(typeStr);
        itemInfo->mActivatable = itemInfo->mType == ITEM_USABLE;
        // Everything not unusable or usable is equippable by the old type system.
        itemInfo->mEquippable  = itemInfo->mType != ITEM_UNUSABLE
                                 && itemInfo->mType != ITEM_USABLE;
        itemInfo->mView = view;
        itemInfo->mWeight = weight;
        itemInfo->setAttackAction(attackAction);
        itemInfo->mAttackRange = attackRange;
        itemInfo->setMissileParticle(missileParticle);

        std::vector<std::string> effect;
        for (int i = 0; i < int(sizeof(fields) / sizeof(fields[0])); ++i)
        {
            int value = XML::getProperty(node, fields[i][0], 0);
            if (!value) continue;
            effect.push_back(strprintf(gettext(fields[i][1]), value));
        }
        for (std::list<Stat>::iterator it = extraStats.begin();
                it != extraStats.end(); it++)
        {
            int value = XML::getProperty(node, it->tag.c_str(), 0);
            if (!value) continue;
            effect.push_back(strprintf(it->format.c_str(), value));
        }
        std::string temp = XML::getProperty(node, "effect", "");
        if (!temp.empty())
            effect.push_back(temp);

        for_each_xml_child_node(itemChild, node)
        {
            if (xmlStrEqual(itemChild->name, BAD_CAST "sprite"))
            {
                std::string attackParticle = XML::getProperty(
                    itemChild, "particle-effect", "");
                itemInfo->mParticle = attackParticle;

                loadSpriteRef(itemInfo, itemChild);
            }
            else if (xmlStrEqual(itemChild->name, BAD_CAST "sound"))
            {
                loadSoundRef(itemInfo, itemChild);
            }
            else if (xmlStrEqual(itemChild->name, BAD_CAST "floor"))
            {
                loadFloorSprite(&display, itemChild);
            }
            /*
             * Begin new item definition code. Previous code is left in to
             *     maintain backwards compatibility.
             * Exit here if tmwAthena.
             */
            else if (Net::getNetworkType() == ServerInfo::TMWATHENA);
            else if (xmlStrEqual(itemChild->name, BAD_CAST "equip"))
            {
                // The fact that there is a way to equip is enough.
                // Discard any details, but mark the item as equippable.
                itemInfo->mEquippable = true;
            }
            else if (xmlStrEqual(itemChild->name, BAD_CAST "effect"))
            {
                std::string trigger = XML::getProperty(
                        itemChild, "trigger", "");
                if (trigger.empty()) {
                    logger->log("Found empty trigger effect label, skipping.");
                    continue;
                }

                if (trigger == "activation")
                    itemInfo->mActivatable = true;

                static std::map<std::string, const char* > triggerTable;
                if (triggerTable.empty())
                {
                    // FIXME: This should ideally be softcoded via XML or similar.
                    triggerTable["existence"]       = " when it is in the inventory";
                    triggerTable["activation"]      = " upon activation";
                    triggerTable["equip"]           = " upon successful equip";
                    triggerTable["leave-inventory"] = " when it leaves the inventory";
                    triggerTable["unequip"]         = " when it is unequipped";
                    triggerTable["equip-change"]    = " when it changes the way it is equipped";
                }
                std::map<std::string, const char* >::const_iterator triggerLabel =
                        triggerTable.find(trigger);
                if (triggerLabel == triggerTable.end())
                {
                    logger->log("Warning: unknown trigger %s in item %d!", trigger.c_str(), id);
                    continue;
                }

                for_each_xml_child_node(effectChild, itemChild)
                {
                    if (xmlStrEqual(effectChild->name, BAD_CAST "modifier"))
                    {
                        std::string attribute = XML::getProperty(
                                effectChild, "attribute", "");
                        double value = XML::getFloatProperty(
                                effectChild, "value", 0.0);
                        int duration = XML::getProperty(
                                effectChild, "duration", 0);
                        if (attribute.empty() || !value)
                        {
                            logger->log("Warning: incomplete modifier definition, skipping.");
                            continue;
                        }
                        std::list<Stat>::const_iterator
                                it = extraStats.begin(),
                                it_end = extraStats.end();
                        while (it != it_end && !(*it == attribute))
                            ++it;
                        if (it == extraStats.end())
                        {
                            logger->log("Warning: unknown modifier tag %s, skipping.", attribute.c_str());
                            continue;
                        }
                        effect.push_back(
                                strprintf(strprintf(
                                        duration ?
                                        strprintf("%%s%%s. This effect lasts %d ticks.", duration).c_str()
                                        : "%s%s.", it->format.c_str(), triggerLabel->second).c_str(), value));
                    }
                    else if (xmlStrEqual(effectChild->name, BAD_CAST "modifier"))
                        effect.push_back(strprintf("Provides an autoattack%s.",
                                                   triggerLabel->second));
                    else if (xmlStrEqual(effectChild->name, BAD_CAST "consumes"))
                        effect.push_back(strprintf("This will be consumed%s.",
                                                   triggerLabel->second));
                    else if (xmlStrEqual(effectChild->name, BAD_CAST "label"))
                        effect.push_back(
                                (const char*)effectChild->xmlChildrenNode->content);
                }
            }
        }

        itemInfo->mEffect = effect;

        itemInfo->mDisplay = display;

        mItemInfos[id] = itemInfo;
        if (!name.empty())
        {
            std::string temp = normalized(name);

            NamedItemInfos::const_iterator itr = mNamedItemInfos.find(temp);
            if (itr == mNamedItemInfos.end())
            {
                mNamedItemInfos[temp] = itemInfo;
            }
            else
            {
                logger->log("ItemDB: Duplicate name of item found item %d", id);
            }
        }

        if (!attackAction.empty())
            if (attackRange == 0)
                logger->log("ItemDB: Missing attack range from weapon %i!", id);

#define CHECK_PARAM(param, error_value) \
        if (param == error_value) \
            logger->log("ItemDB: Missing " #param " attribute for item %i!",id)

        if (id >= 0)
        {
            CHECK_PARAM(name, "");
            CHECK_PARAM(description, "");
            CHECK_PARAM(image, "");
        }
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
    mNamedItemInfos.clear();
    mLoaded = false;
}

bool ItemDB::exists(int id)
{
    assert(mLoaded);

    ItemInfos::const_iterator i = mItemInfos.find(id);

    return i != mItemInfos.end();
}

const ItemInfo &ItemDB::get(int id)
{
    assert(mLoaded);

    ItemInfos::const_iterator i = mItemInfos.find(id);

    if (i == mItemInfos.end())
    {
        logger->log("ItemDB: Warning, unknown item ID# %d", id);
        return *mUnknown;
    }

    return *(i->second);
}

const ItemInfo &ItemDB::get(const std::string &name)
{
    assert(mLoaded);

    NamedItemInfos::const_iterator i = mNamedItemInfos.find(normalized(name));

    if (i == mNamedItemInfos.end())
    {
        if (!name.empty())
        {
            logger->log("ItemDB: Warning, unknown item name \"%s\"",
                        name.c_str());
        }
        return *mUnknown;
    }

    return *(i->second);
}

void ItemDB::loadSpriteRef(ItemInfo *itemInfo, xmlNodePtr node)
{
    std::string gender = XML::getProperty(node, "gender", "unisex");
    std::string filename = (const char*) node->xmlChildrenNode->content;

    if (gender == "male" || gender == "unisex")
    {
        itemInfo->setSprite(filename, GENDER_MALE);
    }
    if (gender == "female" || gender == "unisex")
    {
        itemInfo->setSprite(filename, GENDER_FEMALE);
    }
}

void ItemDB::loadSoundRef(ItemInfo *itemInfo, xmlNodePtr node)
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

void ItemDB::loadFloorSprite(SpriteDisplay *display, xmlNodePtr floorNode)
{
    for_each_xml_child_node(spriteNode, floorNode)
    {
        if (xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
        {
            SpriteReference *currentSprite = new SpriteReference;
            currentSprite->sprite = (const char*)spriteNode->xmlChildrenNode->content;
            currentSprite->variant = XML::getProperty(spriteNode, "variant", 0);
            display->sprites.push_back(currentSprite);
        }
        else if (xmlStrEqual(spriteNode->name, BAD_CAST "particlefx"))
        {
            std::string particlefx = (const char*)spriteNode->xmlChildrenNode->content;
            display->particles.push_back(particlefx);
        }
    }
}
