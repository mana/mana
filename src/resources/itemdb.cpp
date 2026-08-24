/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "configuration.h"
#include "log.h"

#include "resources/hairdb.h"
#include "resources/iteminfo.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "net/tmwa/protocol.h"

#include <algorithm>
#include <cassert>
#include <string_view>

/**
 * Item modifiers understood by ManaServItemDB, set up from the attributes
 * defined by the server (see Attributes::informItemDB).
 */
static std::list<ItemStat> extraStats;

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

static uint8_t spriteFromString(std::string_view name)
{
    if (name.empty())
        return SPRITE_ALL;
    if (name == "race" || name == "type")
        return TmwAthena::SPRITE_BASE;
    if (name == "shoes" || name == "boot" || name == "boots")
        return TmwAthena::SPRITE_SHOE;
    if (name == "bottomclothes" || name == "bottom" || name == "pants")
        return TmwAthena::SPRITE_BOTTOMCLOTHES;
    if (name == "topclothes" || name == "top" || name == "torso" || name == "body")
        return TmwAthena::SPRITE_TOPCLOTHES;
    if (name == "misc1")
        return TmwAthena::SPRITE_MISC1;
    if (name == "misc2" || name == "scarf" || name == "scarfs")
        return TmwAthena::SPRITE_MISC2;
    if (name == "hair")
        return TmwAthena::SPRITE_HAIR;
    if (name == "hat" || name == "hats")
        return TmwAthena::SPRITE_HAT;
    if (name == "wings")
        return TmwAthena::SPRITE_CAPE;
    if (name == "glove" || name == "gloves")
        return TmwAthena::SPRITE_GLOVES;
    if (name == "weapon" || name == "weapons")
        return TmwAthena::SPRITE_WEAPON;
    if (name == "shield" || name == "shields")
        return TmwAthena::SPRITE_SHIELD;
    if (name == "amulet" || name == "amulets")
        return 12;
    if (name == "ring" || name == "rings")
        return 13;

    return SPRITE_UNKNOWN;
}

static uint8_t directionFromString(std::string_view name)
{
    if (name.empty())
        return DIRECTION_ALL;
    if (name == "down" || name == "downall")
        return DIRECTION_DOWN;
    if (name == "left")
        return DIRECTION_LEFT;
    if (name == "up" || name == "upall")
        return DIRECTION_UP;
    if (name == "right")
        return DIRECTION_RIGHT;
    if (name == "upleft")
        return DIRECTION_UPLEFT;
    if (name == "upright")
        return DIRECTION_UPRIGHT;
    if (name == "downleft")
        return DIRECTION_DOWNLEFT;
    if (name == "downright")
        return DIRECTION_DOWNRIGHT;

    // hack for died action.
    if (name == "died")
        return DIRECTION_DEAD;

    return DIRECTION_UNKNOWN;
}

void ItemDB::loadEmptyItemDefinition()
{
    mUnknown->name = _("Unknown item");
    mUnknown->display = SpriteDisplay();
    mUnknown->setSprite(paths.spriteErrorFile, Gender::Male, 0);
    mUnknown->setSprite(paths.spriteErrorFile, Gender::Female, 0);
    mUnknown->setSprite(paths.spriteErrorFile, Gender::Neutral, 0);
    mUnknown->hitEffectId = paths.hitEffectId;
    mUnknown->criticalHitEffectId = paths.criticalHitEffectId;
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
        Log::info("ItemDB: Warning, unknown item ID# %d", id);
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
            Log::info("ItemDB: Warning, unknown item name \"%s\"",
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
        itemInfo.setSprite(filename, Gender::Male, race);
    if (gender == "female" || gender == "unisex")
        itemInfo.setSprite(filename, Gender::Female, race);
    if (gender == "hidden" || gender == "other" || gender == "unisex")
        itemInfo.setSprite(filename, Gender::Neutral, race);
}

void ItemDB::loadSoundRef(ItemInfo &itemInfo, XML::Node node)
{
    std::string event = node.getProperty("event", std::string());
    std::string filename { node.textContent() };

    if (event == "hit")
    {
        itemInfo.addSound(EquipmentSoundEvent::Hit, filename);
    }
    else if (event == "strike" || event == "miss")
    {
        itemInfo.addSound(EquipmentSoundEvent::Strike, filename);
    }
    else
    {
        Log::info("ItemDB: Ignoring unknown sound event '%s'",
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

void ItemDB::loadReplacement(ItemInfo &info, XML::Node replaceNode)
{
    std::string_view spriteString;
    std::string_view directionString;

    replaceNode.attribute("sprite", spriteString);
    replaceNode.attribute("direction", directionString);

    const uint8_t sprite = spriteFromString(spriteString);
    const uint8_t direction = directionFromString(directionString);

    if (sprite == SPRITE_UNKNOWN)
    {
        Log::info("ItemDB: Invalid sprite name '%s' in replace tag",
                  spriteString.data());
        return;
    }

    if (direction == DIRECTION_UNKNOWN)
    {
        Log::info("ItemDB: Invalid direction name '%s' in replace tag",
                  directionString.data());
        return;
    }

    Replacement &replace = info.replacements.emplace_back();
    replace.sprite = sprite;
    replace.direction = direction;

    for (auto child : replaceNode.children())
    {
        if (child.name() == "item")
        {
            Replacement::Item &item = replace.items.emplace_back();
            child.attribute("from", item.from);
            child.attribute("to", item.to);
        }
    }
}

void ItemDB::unload()
{
    Log::info("Unloading item database...");

    delete mUnknown;
    mUnknown = nullptr;

    delete_all(mItemInfos);
    mItemInfos.clear();
    mNamedItemInfos.clear();
    mLoaded = false;
}

void ItemDB::loadCommonRef(ItemInfo &itemInfo, XML::Node node, const std::string &filename)
{
    node.attribute("id", itemInfo.id);

    if (!itemInfo.id)
    {
        Log::info("ItemDB: Invalid or missing item Id in %s!", filename.c_str());
        return;
    }

    if (mItemInfos.find(itemInfo.id) != mItemInfos.end())
    {
        Log::info("ItemDB: Redefinition of item Id %d in %s", itemInfo.id, filename.c_str());
    }

    node.attribute("view", itemInfo.mView);
    node.attribute("name", itemInfo.name);
    node.attribute("image", itemInfo.display.image);
    node.attribute("description", itemInfo.description);
    if (!node.attribute("use-text", itemInfo.useText))
        node.attribute("useButton", itemInfo.useText);  // supported for compatibility
    node.attribute("attack-action", itemInfo.attackAction);
    node.attribute("attack-range", itemInfo.attackRange);
    node.attribute("missile-particle", itemInfo.missileParticleFile);
    itemInfo.hitEffectId = node.getProperty("hit-effect-id",
                                            paths.hitEffectId);
    itemInfo.criticalHitEffectId = node.getProperty("critical-hit-effect-id",
                                                    paths.criticalHitEffectId);

    // Load Ta Item Type
    std::string typeStr = node.getProperty("type", "other");
    itemInfo.type = itemTypeFromString(typeStr);
    node.attribute("weight", itemInfo.weight);

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
        else if (itemChild.name() == "replace")
        {
            loadReplacement(itemInfo, itemChild);
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
            Log::info("ItemDB: Duplicate name (%s) for item id %d found.",
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
        Log::info("%s", errMsg.str().c_str());
    }
}

void ItemDB::checkItemInfo(ItemInfo &itemInfo)
{
    int id = itemInfo.id;
    if (!itemInfo.attackAction.empty())
        if (itemInfo.attackRange == 0)
            Log::info("ItemDB: Missing attack range from weapon %i!", id);

    if (id >= 0)
    {
        checkParameter(id, itemInfo.name, std::string());
        checkParameter(id, itemInfo.description, std::string());
        checkParameter(id, itemInfo.display.image, std::string());
        checkParameter(id, itemInfo.weight, 0);
    }
}

namespace TmwAthena {

/**
 * The item attributes displayed below the item description, in the order in
 * which they are shown.
 *
 * Values are substituted as text rather than as a number, since the item
 * database uses suffixes like "%" and "+" on some of them (for example
 * atkspeed="25%" or req-int="5+").
 *
 * When `sign` is set, a "+" is prepended to values that aren't negative.
 *
 * A field is left out when its value is `uninformative`, which is 0 for the
 * bonuses and requirements, and 1 for the attack range since that is the
 * range of every melee weapon.
 */
static const struct ItemField
{
    const char *tag;
    const char *format;
    bool sign;
    const char *uninformative = "0";
} fields[] =
{
    { "attack",          N_("Attack %s"),            true  },
    { "mattack",         N_("M.Attack %s"),          true  },
    { "criticalattack",  N_("Critical chance %s"),   true  },
    { "hit",             N_("Accuracy %s"),          true  },
    { "flee",            N_("Evade %s"),             true  },
    { "atkspeed",        N_("Attack speed %s"),      true  },
    { "attack-range",    N_("Attack range %s"),      false, "1" },
    { "defense",         N_("Defense %s"),           true  },
    { "mdefense",        N_("M.Defense %s"),         true  },
    { "criticaldefense", N_("Critical defense %s"),  true  },
    { "hp",              N_("HP %s"),                true  },
    { "maxhp",           N_("Max HP %s"),            true  },
    { "mp",              N_("MP %s"),                true  },
    { "maxmp",           N_("Max MP %s"),            true  },
    { "str",             N_("Strength %s"),          true  },
    { "agi",             N_("Agility %s"),           true  },
    { "vit",             N_("Vitality %s"),          true  },
    { "int",             N_("Intelligence %s"),      true  },
    { "dex",             N_("Dexterity %s"),         true  },
    { "luk",             N_("Luck %s"),              true  },
    { "level",           N_("Req. level %s"),        false },
    { "req-str",         N_("Req. strength %s"),     false },
    { "req-agi",         N_("Req. agility %s"),      false },
    { "req-vit",         N_("Req. vitality %s"),     false },
    { "req-int",         N_("Req. intelligence %s"), false },
    { "req-dex",         N_("Req. dexterity %s"),    false },
    { "req-luk",         N_("Req. luck %s"),         false },
};

/**
 * Returns the description of the given item field, or an empty string when
 * the item does not have this field or when its value says nothing.
 */
static std::string describeItemField(const ItemField &field, XML::Node node)
{
    std::string value = node.getProperty(field.tag, std::string());
    if (value.empty() || value == field.uninformative)
        return std::string();

    if (field.sign && value[0] != '-' && value[0] != '+')
        value.insert(0, 1, '+');

    return strprintf(gettext(field.format), value.c_str());
}

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
    for (const auto &field : fields)
    {
        std::string description = describeItemField(field, node);
        if (!description.empty())
            effect.push_back(std::move(description));
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
                Log::info("Found empty trigger effect label in %s, skipping.", filename.c_str());
                continue;
            }

            if (trigger == "activation")
                itemInfo->activatable = true;

            auto triggerLabel = triggerTable.find(trigger);
            if (triggerLabel == triggerTable.end())
            {
                Log::warn("Unknown trigger %s in item %d!",
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
                        Log::warn("Incomplete modifier definition in %s, skipping.", filename.c_str());
                        continue;
                    }
                    auto it = std::find(extraStats.cbegin(), extraStats.cend(), attribute);
                    if (it == extraStats.end())
                    {
                        Log::warn("Unknown modifier tag %s in %s, skipping.", attribute.c_str(), filename.c_str());
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
