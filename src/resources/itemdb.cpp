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


#include <cassert>

void setStatsList(std::list<ItemStat> stats)
{
    extraStats = std::move(stats);
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
    mUnknown->name = _("Unknown item");
    mUnknown->display = SpriteDisplay();
    std::string errFile = paths.getStringValue("spriteErrorFile");
    mUnknown->setSprite(errFile, Gender::MALE, 0);
    mUnknown->setSprite(errFile, Gender::FEMALE, 0);
    mUnknown->setSprite(errFile, Gender::HIDDEN, 0);
    mUnknown->hitEffectId = paths.getIntValue("hitEffectId");
    mUnknown->criticalHitEffectId = paths.getIntValue("criticalHitEffectId");
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

void ItemDB::loadSpriteRef(ItemInfo &itemInfo, XML::Node node)
{
    std::string gender = node.getProperty("gender", "unisex");
    std::string filename { node.textContent() };

    const int race = node.getProperty("race", 0);
    if (gender == "male" || gender == "unisex")
        itemInfo.setSprite(filename, Gender::MALE, race);
    if (gender == "female" || gender == "unisex")
        itemInfo.setSprite(filename, Gender::FEMALE, race);
    if (gender == "hidden" || gender == "other" || gender == "unisex")
        itemInfo.setSprite(filename, Gender::HIDDEN, race);
}

void ItemDB::loadSoundRef(ItemInfo &itemInfo, XML::Node node)
{
    std::string event = node.getProperty("event", std::string());
    std::string filename { node.textContent() };

    if (event == "hit")
    {
        itemInfo.addSound(EquipmentSoundEvent::HIT, filename);
    }
    else if (event == "strike" || event == "miss")
    {
        itemInfo.addSound(EquipmentSoundEvent::STRIKE, filename);
    }
    else
    {
        logger->log("ItemDB: Ignoring unknown sound event '%s'",
                event.c_str());
    }
}

void ItemDB::loadFloorSprite(SpriteDisplay &display, XML::Node floorNode)
{
    for (auto spriteNode : floorNode.children())
    {
        if (spriteNode.name() == "sprite")
        {
            SpriteReference &currentSprite = display.sprites.emplace_back();
            currentSprite.sprite = spriteNode.textContent();
            currentSprite.variant = spriteNode.getProperty("variant", 0);
        }
        else if (spriteNode.name() == "particlefx")
        {
            display.particles.emplace_back(spriteNode.textContent());
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

void ItemDB::loadCommonRef(ItemInfo &itemInfo, XML::Node node, const std::string &filename)
{
    itemInfo.id = node.getProperty("id", 0);

    if (!itemInfo.id)
    {
        logger->log("ItemDB: Invalid or missing item Id in %s!", filename.c_str());
        return;
    }
    else if (mItemInfos.find(itemInfo.id) != mItemInfos.end())
    {
        logger->log("ItemDB: Redefinition of item Id %d in %s", itemInfo.id, filename.c_str());
    }

    itemInfo.mView = node.getProperty("view", 0);
    itemInfo.name = node.getProperty("name", std::string());
    itemInfo.display.image = node.getProperty("image", std::string());
    itemInfo.description = node.getProperty("description", std::string());
    itemInfo.attackAction = node.getProperty("attack-action", SpriteAction::INVALID);
    itemInfo.attackRange = node.getProperty("attack-range", 0);
    itemInfo.missileParticleFile = node.getProperty("missile-particle", std::string());
    itemInfo.hitEffectId = node.getProperty("hit-effect-id",
                                            paths.getIntValue("hitEffectId"));
    itemInfo.criticalHitEffectId = node.getProperty("critical-hit-effect-id",
                                                    paths.getIntValue("criticalHitEffectId"));

    // Load Ta Item Type
    std::string typeStr = node.getProperty("type", "other");
    itemInfo.type = itemTypeFromString(typeStr);
    itemInfo.weight = node.getProperty("weight", 0);

    for (auto itemChild : node.children())
    {
        if (itemChild.name() == "sprite")
        {
            loadSpriteRef(itemInfo, itemChild);
        }
        else if (itemChild.name() == "particlefx")
        {
            itemInfo.display.particles.emplace_back(itemChild.textContent());
        }
        else if (itemChild.name() == "sound")
        {
            loadSoundRef(itemInfo, itemChild);
        }
        else if (itemChild.name() == "floor")
        {
            loadFloorSprite(itemInfo.display, itemChild);
        }
    }
}

void ItemDB::addItem(ItemInfo *itemInfo)
{
    std::string itemName = itemInfo->name;
    itemInfo->name = itemName.empty() ? _("unnamed") : itemName;
    mItemInfos[itemInfo->id] = itemInfo;
    if (!itemName.empty())
    {
        std::string temp = normalize(itemName);

        auto itr = mNamedItemInfos.find(temp);
        if (itr == mNamedItemInfos.end())
            mNamedItemInfos[temp] = itemInfo;
        else
            logger->log("ItemDB: Duplicate name (%s) for item id %d found.",
                        temp.c_str(), itemInfo->id);
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

void ItemDB::checkItemInfo(ItemInfo &itemInfo)
{
    int id = itemInfo.id;
    if (!itemInfo.attackAction.empty())
        if (itemInfo.attackRange == 0)
            logger->log("ItemDB: Missing attack range from weapon %i!", id);

    if (id >= 0)
    {
        checkParameter(id, itemInfo.name, std::string());
        checkParameter(id, itemInfo.description, std::string());
        checkParameter(id, itemInfo.display.image, std::string());
        checkParameter(id, itemInfo.weight, 0);
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

void TaItemDB::readItemNode(XML::Node node, const std::string &filename)
{
    auto *itemInfo = new ItemInfo;

    loadCommonRef(*itemInfo, node, filename);

    // Everything not unusable or usable is equippable by the Ta type system.
    itemInfo->equippable  = itemInfo->type != ITEM_UNUSABLE
                             && itemInfo->type != ITEM_USABLE;
    itemInfo->activatable = itemInfo->type == ITEM_USABLE;

    // Load nano description
    std::vector<std::string> effect;
    for (auto field : fields)
    {
        int value = node.getProperty(field[0], 0);
        if (!value)
            continue;
        effect.push_back(strprintf(gettext(field[1]), value));
    }
    for (auto &extraStat : extraStats)
    {
        int value = node.getProperty(extraStat.mTag.c_str(), 0);
        if (!value)
            continue;
        effect.push_back(strprintf(extraStat.mFormat.c_str(), value));
    }
    std::string temp = node.getProperty("effect", std::string());
    if (!temp.empty())
        effect.push_back(temp);

    itemInfo->effect = effect;

    checkItemInfo(*itemInfo);

    addItem(itemInfo);

    // Insert hairstyle id while letting the info as an item.
    if (itemInfo->type == ITEM_SPRITE_HAIR)
        hairDB.addHairStyle(itemInfo->id);
}

void TaItemDB::checkStatus()
{
    mUnknown = new ItemInfo;
    loadEmptyItemDefinition();

    checkHairWeaponsRacesSpecialIds();

    mLoaded = true;
}

void TaItemDB::checkItemInfo(ItemInfo &itemInfo)
{
    ItemDB::checkItemInfo(itemInfo);

    // Check for unusable items?
    //checkParameter(id, itemInfo->mType, 0);
}

}; // namespace TmwAthena

namespace ManaServ {

void ManaServItemDB::init()
{
    if (mLoaded)
        unload();
}

void ManaServItemDB::readItemNode(XML::Node node, const std::string &filename)
{
    // Trigger table for effect descriptions
    // FIXME: This should ideally be softcoded via XML or similar.
    static const std::map<std::string, const char* > triggerTable = {
        { "existence",       " when it is in the inventory" },
        { "activation",      " upon activation" },
        { "equip",           " upon successful equip" },
        { "leave-inventory", " when it leaves the inventory" },
        { "unequip",         " when it is unequipped" },
        { "equip-change",    " when it changes the way it is equipped" },
    };

    auto *itemInfo = new ItemInfo;

    loadCommonRef(*itemInfo, node, filename);

    // We default eqippable and activatable to false as their actual value will be set
    // within the <equip> and <effect> sub-nodes..
    itemInfo->activatable = false;
    itemInfo->equippable = false;

    // Load <equip>, and <effect> sub nodes.
    std::vector<std::string> effect;
    for (auto itemChild : node.children())
    {
        if (itemChild.name() == "equip")
        {
            // The fact that there is a way to equip is enough.
            // Discard any details, but mark the item as equippable.
            itemInfo->equippable = true;
        }
        else if (itemChild.name() == "effect")
        {
            std::string trigger = itemChild.getProperty("trigger", std::string());
            if (trigger.empty())
            {
                logger->log("Found empty trigger effect label in %s, skipping.", filename.c_str());
                continue;
            }

            if (trigger == "activation")
                itemInfo->activatable = true;

            auto triggerLabel = triggerTable.find(trigger);
            if (triggerLabel == triggerTable.end())
            {
                logger->log("Warning: unknown trigger %s in item %d!",
                            trigger.c_str(), itemInfo->id);
                continue;
            }

            for (auto effectChild : itemChild.children())
            {
                if (effectChild.name() == "modifier")
                {
                    std::string attribute = effectChild.getProperty("attribute", std::string());
                    double value = effectChild.getFloatProperty("value", 0.0);
                    int duration = effectChild.getProperty("duration", 0);
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
                else if (effectChild.name() == "modifier")
                    effect.push_back(strprintf("Provides an autoattack%s.",
                                               triggerLabel->second));
                else if (effectChild.name() == "consumes")
                    effect.push_back(strprintf("This will be consumed%s.",
                                               triggerLabel->second));
                else if (effectChild.name() == "label")
                    effect.emplace_back(effectChild.textContent());
            }
        }

        // FIXME: Load hair styles through the races.xml file
        if (itemInfo->type == ITEM_SPRITE_HAIR)
            hairDB.addHairStyle(itemInfo->id);

        // Set Item Type based on subnodes info
        // TODO: Improve it once the itemTypes are loaded through xml
        itemInfo->type = ITEM_UNUSABLE;
        if (itemInfo->activatable)
            itemInfo->type = ITEM_USABLE;
        else if (itemInfo->equippable)
            itemInfo->type = ITEM_EQUIPMENT_TORSO;
    } // end for (auto itemChild : node.children())

    itemInfo->effect = effect;

    checkItemInfo(*itemInfo);

    addItem(itemInfo);
}

void ManaServItemDB::checkStatus()
{
    mUnknown = new ItemInfo;
    loadEmptyItemDefinition();

    mLoaded = true;
}

void ManaServItemDB::checkItemInfo(ItemInfo &itemInfo)
{
    ItemDB::checkItemInfo(itemInfo);

    // Add specific Manaserv checks here
}

} // namespace ManaServ
