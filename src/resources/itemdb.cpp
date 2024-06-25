/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "resources/hairdb.h"
#include "resources/iteminfo.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "configuration.h"

#include <libxml/tree.h>

#include <cassert>

void setStatsList(const std::list<ItemStat> &stats)
{
    extraStats = stats;
}

static ItemType itemTypeFromString(const std::string &name, int id = 0)
{
    if (name == "generic")          return ITEM_UNUSABLE;
    if (name == "usable")           return ITEM_USABLE;
    if (name == "equip-1hand")      return ITEM_EQUIPMENT_ONE_HAND_WEAPON;
    if (name == "equip-2hand")      return ITEM_EQUIPMENT_TWO_HANDS_WEAPON;
    if (name == "equip-torso")      return ITEM_EQUIPMENT_TORSO;
    if (name == "equip-arms")       return ITEM_EQUIPMENT_ARMS;
    if (name == "equip-head")       return ITEM_EQUIPMENT_HEAD;
    if (name == "equip-legs")       return ITEM_EQUIPMENT_LEGS;
    if (name == "equip-shield")     return ITEM_EQUIPMENT_SHIELD;
    if (name == "equip-ring")       return ITEM_EQUIPMENT_RING;
    if (name == "equip-charm")      return ITEM_EQUIPMENT_CHARM;
    if (name == "equip-necklace")   return ITEM_EQUIPMENT_NECKLACE;
    if (name == "equip-feet")       return ITEM_EQUIPMENT_FEET;
    if (name == "equip-ammo")       return ITEM_EQUIPMENT_AMMO;
    if (name == "racesprite")       return ITEM_SPRITE_RACE;
    if (name == "hairsprite")       return ITEM_SPRITE_HAIR;
    return ITEM_UNUSABLE;
}

void ItemDB::loadEmptyItemDefinition()
{
    mUnknown->mName = _("Unknown item");
    mUnknown->mDisplay = SpriteDisplay();
    std::string errFile = paths.getStringValue("spriteErrorFile");
    mUnknown->setSprite(errFile, Gender::MALE, 0);
    mUnknown->setSprite(errFile, Gender::FEMALE, 0);
    mUnknown->setSprite(errFile, Gender::HIDDEN, 0);
    mUnknown->setHitEffectId(paths.getIntValue("hitEffectId"));
    mUnknown->setCriticalHitEffectId(paths.getIntValue("criticalHitEffectId"));
}

/*
 * Common itemDB functions
 */

bool ItemDB::exists(int id) const
{
    assert(mLoaded);

    return mItemInfos.find(id) != mItemInfos.end();
}

const ItemInfo &ItemDB::get(int id) const
{
    assert(mLoaded);

    auto i = mItemInfos.find(id);
    if (i == mItemInfos.end())
    {
        logger->log("ItemDB: Warning, unknown item ID# %d", id);
        return *mUnknown;
    }

    return *(i->second);
}

const ItemInfo &ItemDB::get(const std::string &name) const
{
    assert(mLoaded);

    auto i = mNamedItemInfos.find(normalize(name));
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

    const int race = XML::getProperty(node, "race", 0);
    if (gender == "male" || gender == "unisex")
        itemInfo->setSprite(filename, Gender::MALE, race);
    if (gender == "female" || gender == "unisex")
        itemInfo->setSprite(filename, Gender::FEMALE, race);
    if (gender == "hidden" || gender == "other" || gender == "unisex")
        itemInfo->setSprite(filename, Gender::HIDDEN, race);
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
    for (auto spriteNode : XML::Children(floorNode))
    {
        if (xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
        {
            SpriteReference currentSprite;
            currentSprite.sprite = (const char*)spriteNode->xmlChildrenNode->content;
            currentSprite.variant = XML::getProperty(spriteNode, "variant", 0);
            display->sprites.push_back(currentSprite);
        }
        else if (xmlStrEqual(spriteNode->name, BAD_CAST "particlefx"))
        {
            std::string particlefx = (const char*)spriteNode->xmlChildrenNode->content;
            display->particles.push_back(particlefx);
        }
    }
}

void ItemDB::unload()
{
    logger->log("Unloading item database...");

    delete mUnknown;
    mUnknown = nullptr;

    delete_all(mItemInfos);
    mItemInfos.clear();
    mNamedItemInfos.clear();
    mLoaded = false;
}

void ItemDB::loadCommonRef(ItemInfo *itemInfo, xmlNodePtr node, const std::string &filename)
{
        int id = XML::getProperty(node, "id", 0);

        if (!id)
        {
            logger->log("ItemDB: Invalid or missing item Id in %s!", filename.c_str());
            return;
        }
        else if (mItemInfos.find(id) != mItemInfos.end())
            logger->log("ItemDB: Redefinition of item Id %d in %s", id, filename.c_str());

        int view = XML::getProperty(node, "view", 0);

        std::string name = XML::getProperty(node, "name", "");
        std::string image = XML::getProperty(node, "image", "");
        std::string description = XML::getProperty(node, "description", "");
        std::string attackAction = XML::getProperty(node, "attack-action",
                                                    SpriteAction::INVALID);
        int attackRange = XML::getProperty(node, "attack-range", 0);
        std::string missileParticleFile = XML::getProperty(node,
                                                           "missile-particle",
                                                           "");
        int hitEffectId = XML::getProperty(node, "hit-effect-id",
                                           paths.getIntValue("hitEffectId"));
        int criticalEffectId = XML::getProperty(node, "critical-hit-effect-id",
                                      paths.getIntValue("criticalHitEffectId"));

        // Load Ta Item Type
        std::string typeStr = XML::getProperty(node, "type", "other");
        itemInfo->mType = itemTypeFromString(typeStr);

        int weight = XML::getProperty(node, "weight", 0);
        itemInfo->mWeight = weight > 0 ? weight : 0;

        SpriteDisplay display;
        display.image = image;

        itemInfo->mId = id;
        itemInfo->mName = name;
        itemInfo->mDescription = description;
        itemInfo->mView = view;
        itemInfo->mWeight = weight;
        itemInfo->mAttackAction = attackAction;
        itemInfo->mAttackRange = attackRange;
        itemInfo->setMissileParticleFile(missileParticleFile);
        itemInfo->setHitEffectId(hitEffectId);
        itemInfo->setCriticalHitEffectId(criticalEffectId);

        // Load <sprite>, <sound>, and <floor>
        for (auto itemChild : XML::Children(node))
        {
            if (xmlStrEqual(itemChild->name, BAD_CAST "sprite"))
            {
                itemInfo->mParticle = XML::getProperty(
                            itemChild, "particle-effect", std::string());

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

        }

        // If the item has got a floor image, we bind the good reference.
        itemInfo->mDisplay = display;
}

void ItemDB::addItem(ItemInfo *itemInfo)
{
    std::string itemName = itemInfo->mName;
    itemInfo->mName = itemName.empty() ? _("unnamed") : itemName;
    mItemInfos[itemInfo->mId] = itemInfo;
    if (!itemName.empty())
    {
        std::string temp = normalize(itemName);

        auto itr = mNamedItemInfos.find(temp);
        if (itr == mNamedItemInfos.end())
            mNamedItemInfos[temp] = itemInfo;
        else
            logger->log("ItemDB: Duplicate name (%s) for item id %d found.",
                        temp.c_str(), itemInfo->mId);
    }
}

template <class T>
static void checkParameter(int id, const T param, const T errorValue)
{
    if (param == errorValue)
    {
        std::stringstream errMsg;
        errMsg << "ItemDB: Missing " << param << " attribute for item id "
               << id << "!";
        logger->log("%s", errMsg.str().c_str());
    }
}

void ItemDB::checkItemInfo(ItemInfo* itemInfo)
{
    int id = itemInfo->mId;
    if (!itemInfo->getAttackAction().empty())
        if (itemInfo->mAttackRange == 0)
            logger->log("ItemDB: Missing attack range from weapon %i!", id);

    if (id >= 0)
    {
        checkParameter(id, itemInfo->mName, std::string());
        checkParameter(id, itemInfo->mDescription, std::string());
        checkParameter(id, itemInfo->mDisplay.image, std::string());
        checkParameter(id, itemInfo->mWeight, 0);
    }
}

namespace TmwAthena {

// Description fields used by TaItemDB *itemInfo->mEffect.

static char const *const fields[][2] =
{
    { "attack",    N_("Attack %+d")    },
    { "defense",   N_("Defense %+d")   },
    { "hp",        N_("HP %+d")        },
    { "mp",        N_("MP %+d")        }
};

void TaItemDB::init()
{
    if (mLoaded)
        unload();
}

void TaItemDB::readItemNode(xmlNodePtr node, const std::string &filename)
{
    auto *itemInfo = new TaItemInfo;

    loadCommonRef(itemInfo, node, filename);

    // Everything not unusable or usable is equippable by the Ta type system.
    itemInfo->mEquippable  = itemInfo->mType != ITEM_UNUSABLE
                             && itemInfo->mType != ITEM_USABLE;
    itemInfo->mActivatable = itemInfo->mType == ITEM_USABLE;

    // Load nano description
    std::vector<std::string> effect;
    for (auto field : fields)
    {
        int value = XML::getProperty(node, field[0], 0);
        if (!value)
            continue;
        effect.push_back(strprintf(gettext(field[1]), value));
    }
    for (auto &extraStat : extraStats)
    {
        int value = XML::getProperty(node, extraStat.mTag.c_str(), 0);
        if (!value)
            continue;
        effect.push_back(strprintf(extraStat.mFormat.c_str(), value));
    }
    std::string temp = XML::getProperty(node, "effect", "");
    if (!temp.empty())
        effect.push_back(temp);

    itemInfo->mEffect = effect;

    checkItemInfo(itemInfo);

    addItem(itemInfo);

    // Insert hairstyle id while letting the info as an item.
    if (itemInfo->mType == ITEM_SPRITE_HAIR)
        hairDB.addHairStyle(itemInfo->mId);
}

void TaItemDB::checkStatus()
{
    mUnknown = new TaItemInfo;
    loadEmptyItemDefinition();

    checkHairWeaponsRacesSpecialIds();

    mLoaded = true;
}

void TaItemDB::checkItemInfo(ItemInfo* itemInfo)
{
    ItemDB::checkItemInfo(itemInfo);

    // Check for unusable items?
    //checkParameter(id, itemInfo->mType, 0);
}

}; // namespace TmwAthena

namespace ManaServ {

static std::map<std::string, const char* > triggerTable;

static void initTriggerTable()
{
    if (triggerTable.empty())
    {
        // FIXME: This should ideally be softcoded via XML or similar.
        logger->log("Initializing ManaServ trigger table...");
        triggerTable["existence"]       = " when it is in the inventory";
        triggerTable["activation"]      = " upon activation";
        triggerTable["equip"]           = " upon successful equip";
        triggerTable["leave-inventory"] = " when it leaves the inventory";
        triggerTable["unequip"]         = " when it is unequipped";
        triggerTable["equip-change"]    = " when it changes the way it is equipped";
    }
}

void ManaServItemDB::init()
{
    if (mLoaded)
        unload();

    // Initialize the trigger table for effect descriptions
    initTriggerTable();
}

void ManaServItemDB::readItemNode(xmlNodePtr node, const std::string &filename)
{
    auto *itemInfo = new ManaServItemInfo;

    loadCommonRef(itemInfo, node, filename);

    // We default eqippable and activatable to false as their actual value will be set
    // within the <equip> and <effect> sub-nodes..
    itemInfo->mActivatable = false;
    itemInfo->mEquippable = false;

    // Load <equip>, and <effect> sub nodes.
    std::vector<std::string> effect;
    for (auto itemChild : XML::Children(node))
    {
        if (xmlStrEqual(itemChild->name, BAD_CAST "equip"))
        {
            // The fact that there is a way to equip is enough.
            // Discard any details, but mark the item as equippable.
            itemInfo->mEquippable = true;
        }
        else if (xmlStrEqual(itemChild->name, BAD_CAST "effect"))
        {
            std::string trigger = XML::getProperty(
                    itemChild, "trigger", "");
            if (trigger.empty())
            {
                logger->log("Found empty trigger effect label in %s, skipping.", filename.c_str());
                continue;
            }

            if (trigger == "activation")
                itemInfo->mActivatable = true;

            auto triggerLabel = triggerTable.find(trigger);
            if (triggerLabel == triggerTable.end())
            {
                logger->log("Warning: unknown trigger %s in item %d!",
                            trigger.c_str(), itemInfo->mId);
                continue;
            }

            for (auto effectChild : XML::Children(itemChild))
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
                        logger->log("Warning: incomplete modifier definition in %s, skipping.", filename.c_str());
                        continue;
                    }
                    auto it = extraStats.cbegin();
                    auto it_end = extraStats.cend();
                    while (it != it_end && !(*it == attribute))
                        ++it;
                    if (it == extraStats.end())
                    {
                        logger->log("Warning: unknown modifier tag %s in %s, skipping.", attribute.c_str(), filename.c_str());
                        continue;
                    }
                    effect.push_back(
                            strprintf(strprintf(
                                    duration ?
                                    strprintf("%%s%%s. This effect lasts %d ticks.", duration).c_str()
                                    : "%s%s.", it->mFormat.c_str(), triggerLabel->second).c_str(), value));
                }
                else if (xmlStrEqual(effectChild->name, BAD_CAST "modifier"))
                    effect.push_back(strprintf("Provides an autoattack%s.",
                                               triggerLabel->second));
                else if (xmlStrEqual(effectChild->name, BAD_CAST "consumes"))
                    effect.push_back(strprintf("This will be consumed%s.",
                                               triggerLabel->second));
                else if (xmlStrEqual(effectChild->name, BAD_CAST "label"))
                    effect.emplace_back(
                            (const char*)effectChild->xmlChildrenNode->content);
            }
        }

        // FIXME: Load hair styles through the races.xml file
        if (itemInfo->mType == ITEM_SPRITE_HAIR)
            hairDB.addHairStyle(itemInfo->mId);

        // Set Item Type based on subnodes info
        // TODO: Improve it once the itemTypes are loaded through xml
        itemInfo->mType = ITEM_UNUSABLE;
        if (itemInfo->mActivatable)
            itemInfo->mType = ITEM_USABLE;
        else if (itemInfo->mEquippable)
            itemInfo->mType = ITEM_EQUIPMENT_TORSO;
    } // end for (auto itemChild : XML::Children(node))

    itemInfo->mEffect = effect;

    checkItemInfo(itemInfo);

    addItem(itemInfo);
}

void ManaServItemDB::checkStatus()
{
    mUnknown = new ManaServItemInfo;
    loadEmptyItemDefinition();

    mLoaded = true;
}

void ManaServItemDB::checkItemInfo(ItemInfo* itemInfo)
{
    ItemDB::checkItemInfo(itemInfo);

    // Add specific Manaserv checks here
}

} // namespace ManaServ
