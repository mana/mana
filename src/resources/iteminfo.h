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

#ifndef ITEMINFO_H
#define ITEMINFO_H

#include "being.h"

#include "resources/spritedef.h"

#include <map>
#include <string>
#include <vector>

enum EquipmentSoundEvent
{
    EQUIP_EVENT_STRIKE,
    EQUIP_EVENT_HIT
};

/**
 * Enumeration of available Item types.
 * TODO: Dynamise this using an xml.
 */
enum ItemType
{
    ITEM_UNUSABLE = 0,
    ITEM_USABLE,
    ITEM_EQUIPMENT_ONE_HAND_WEAPON,
    ITEM_EQUIPMENT_TWO_HANDS_WEAPON,
    ITEM_EQUIPMENT_TORSO,
    ITEM_EQUIPMENT_ARMS, // 5
    ITEM_EQUIPMENT_HEAD,
    ITEM_EQUIPMENT_LEGS,
    ITEM_EQUIPMENT_SHIELD,
    ITEM_EQUIPMENT_RING,
    ITEM_EQUIPMENT_NECKLACE, // 10
    ITEM_EQUIPMENT_FEET,
    ITEM_EQUIPMENT_AMMO,
    ITEM_EQUIPMENT_CHARM,
    ITEM_SPRITE_RACE,
    ITEM_SPRITE_HAIR // 15
};

namespace TmwAthena {
class TaItemDB;
}

namespace ManaServ {
class ManaServItemDB;
}

/**
 * Defines a class for storing generic item infos.
 */
class ItemInfo
{
    friend class ItemDB;
    friend class TmwAthena::TaItemDB;
    friend class ManaServ::ManaServItemDB;

public:
    ItemInfo() = default;

    // disable copying
    ItemInfo(const ItemInfo &) = delete;
    ItemInfo &operator=(const ItemInfo &) = delete;

    int id = 0;                         /**< Item ID */
    std::string name;
    std::string particle;               /**< Particle effect used with this item */
    SpriteDisplay display;              /**< Display info (like icon) */
    std::string description;            /**< Short description. */
    std::vector<std::string> effect;    /**< Description of effects. */
    int weight = 0;                     /**< Weight in grams. */

    /** Effects to be shown when weapon attacks - see also effects.xml */
    std::string missileParticleFile;
    int hitEffectId = 0;
    int criticalHitEffectId = 0;

    /** Attack type, in case of weapon.
     * See SpriteAction in spritedef.h for more info.
     * Attack action sub-types (bow, sword, ...) are defined in items.xml.
     */
    std::string attackAction = SpriteAction::INVALID;

    /** Attack range, will be equal to ATTACK_RANGE_NOT_SET if no weapon. */
    int attackRange = 0;

    bool equippable = false;            /**< Whether this item can be equipped. */
    bool activatable = false;           /**< Whether this item can be activated. */

    ItemType type = ITEM_UNUSABLE;      /**< Item type. */

    const std::string &getSprite(Gender gender, int race) const;
    const std::string &getSound(EquipmentSoundEvent event) const;

private:
    void setSprite(const std::string &animationFile, Gender gender, int race);
    void addSound(EquipmentSoundEvent event, const std::string &filename);

    int mView = 0;                       /**< Item ID of how this item looks. */

    /** Maps gender to sprite filenames. */
    std::map<int, std::string> mAnimationFiles;

    /** Stores the names of sounds to be played at certain event. */
    std::map<EquipmentSoundEvent, std::vector<std::string>> mSounds;
};

/*
 * TmwAthena specialization of the ItemInfo for TmwAthena
 */
namespace TmwAthena {

enum EquipmentSlot
{
    // Equipment rules:
    // 1 Brest equipment
    EQUIP_TORSO_SLOT = 0,
    // 1 arms equipment
    EQUIP_ARMS_SLOT = 1,
    // 1 head equipment
    EQUIP_HEAD_SLOT = 2,
    // 1 legs equipment
    EQUIP_LEGS_SLOT = 3,
    // 1 feet equipment
    EQUIP_FEET_SLOT = 4,
    // 2 rings
    EQUIP_RING1_SLOT = 5,
    EQUIP_RING2_SLOT = 6,
    // 1 necklace
    EQUIP_NECKLACE_SLOT = 7,
    // Fight:
    //   2 one-handed weapons
    //   or 1 two-handed weapon
    //   or 1 one-handed weapon + 1 shield.
    EQUIP_FIGHT1_SLOT = 8,
    EQUIP_FIGHT2_SLOT = 9,
    // Projectile:
    //   this item does not amount to one, it only indicates the chosen projectile.
    EQUIP_PROJECTILE_SLOT = 10,
    EQUIP_VECTOR_END = 11
};

} // namespace TmwAthena

#endif // ITEMINFO_H
