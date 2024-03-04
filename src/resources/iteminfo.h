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
 * Specialized version for one or another protocol are defined below.
 */
class ItemInfo
{
    friend class ItemDB;
    friend void loadSpriteRef(ItemInfo *itemInfo, xmlNodePtr node);
    friend class TmwAthena::TaItemDB;
    friend class ManaServ::ManaServItemDB;

    public:
        ItemInfo():
            mAttackAction(SpriteAction::INVALID)
        {
        }

        int getId() const
        { return mId; }

        const std::string &getName() const
        { return mName; }

        std::string getParticleEffect() const
        { return mParticle; }

        const SpriteDisplay &getDisplay() const
        { return mDisplay; }

        const std::string &getDescription() const
        { return mDescription; }

        const std::vector<std::string> &getEffect() const
        { return mEffect; }

        int getWeight() const
        { return mWeight; }

        const std::string &getSprite(Gender gender, int race) const;

        // Handlers for seting and getting the string used for particles when attacking
        void setMissileParticleFile(const std::string &s)
        { mMissileParticleFile = s; }

        const std::string &getMissileParticleFile() const
        { return mMissileParticleFile; }

        void setHitEffectId(int s)
        { mHitEffectId = s; }

        int getHitEffectId() const
        { return mHitEffectId; }

        void setCriticalHitEffectId(int s)
        { mCriticalHitEffectId = s; }

        int getCriticalHitEffectId() const
        { return mCriticalHitEffectId; }

        const std::string &getAttackAction() const
        { return mAttackAction; }

        int getAttackRange() const
        { return mAttackRange; }

        const std::string &getSound(EquipmentSoundEvent event) const;

        bool getEquippable() const
        { return mEquippable; }

        bool getActivatable() const
        { return mActivatable; }

        ItemType getItemType() const
        { return mType; }

    private:
        void setSprite(const std::string &animationFile,
                       Gender gender, int race);

        void addSound(EquipmentSoundEvent event, const std::string &filename);

        SpriteDisplay mDisplay;           /**< Display info (like icon) */
        std::string mName;
        std::string mDescription;         /**< Short description. */
        std::vector<std::string> mEffect; /**< Description of effects. */
        ItemType mType = ITEM_UNUSABLE;   /**< Item type. */
        std::string mParticle;            /**< Particle effect used with this item */
        int mWeight = 0;                  /**< Weight in grams. */
        int mView = 0;                    /**< Item ID of how this item looks. */
        int mId = 0;                      /**< Item ID */

        bool mEquippable = false;         /**< Whether this item can be equipped. */
        bool mActivatable = false;        /**< Whether this item can be activated. */

        // Equipment related members.
        /** Attack type, in case of weapon.
         * See SpriteAction in spritedef.h for more info.
         * Attack action sub-types (bow, sword, ...) are defined in items.xml.
         */
        std::string mAttackAction;

        /** Attack range, will be equal to ATTACK_RANGE_NOT_SET if no weapon. */
        int mAttackRange = 0;

        /** Effects to be shown when weapon attacks - see also effects.xml */
        std::string mMissileParticleFile;
        int mHitEffectId = 0;
        int mCriticalHitEffectId = 0;

        /** Maps gender to sprite filenames. */
        std::map<int, std::string> mAnimationFiles;

        /** Stores the names of sounds to be played at certain event. */
        std::map< EquipmentSoundEvent, std::vector<std::string> > mSounds;
};

/*
 * TmwAthena specialization of the itemInfo for TmwAthena
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

/**
 * Defines a class for storing TmwAthena specific item infos.
 * Specialized version for one or another protocol are defined below.
 */
class TaItemInfo: public ItemInfo
{
    friend class TaItemDB;

    public:
        TaItemInfo()
        {}

        // Declare TmwAthena Specific item info here
};

} // namespace TmwAthena

namespace ManaServ {

/**
 * Defines a class for storing Manaserv Specific item infos.
 * Specialized version for one or another protocol are defined below.
 */
class ManaServItemInfo: public ItemInfo
{
    public:
        ManaServItemInfo()
        {}

        // Declare Manaserv Specific item info here
};


} // namespace ManaServ

#endif // ITEMINFO_H
