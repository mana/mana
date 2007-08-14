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

#ifndef _TMW_LOCALPLAYER_H
#define _TMW_LOCALPLAYER_H

#include <memory.h>

#include "player.h"

// TODO move into some sane place...
#define MAX_SLOT 2

class FloorItem;
class Inventory;
class Item;

/**
 * Stats every being needs
 */
enum BeingStats
{
    BASE_ATTR_STRENGTH = 0,    // Basic attributes
    BASE_ATTR_AGILITY,
    BASE_ATTR_DEXTERITY,
    BASE_ATTR_VITALITY,
    BASE_ATTR_INTELLIGENCE,
    BASE_ATTR_WILLPOWER,
    BASE_ATTR_CHARISMA,
    NB_BASE_ATTRIBUTES,

    ATTR_EFF_STRENGTH = NB_BASE_ATTRIBUTES,    // modified basic attributes
    ATTR_EFF_AGILITY,
    ATTR_EFF_DEXTERITY,
    ATTR_EFF_VITALITY,
    ATTR_EFF_INTELLIGENCE,
    ATTR_EFF_WILLPOWER,
    ATTR_EFF_CHARISMA,
    NB_EFFECTIVE_ATTRIBUTES,

    DERIVED_ATTR_HP_MAXIMUM = NB_EFFECTIVE_ATTRIBUTES,    // Computed stats
    DERIVED_ATTR_PHYSICAL_ATTACK_MINIMUM,
    DERIVED_ATTR_PHYSICAL_ATTACK_FLUCTUATION,
    DERIVED_ATTR_PHYSICAL_DEFENCE,
    // add new computed statistics when they are needed
    NB_ATTRIBUTES_BEING
};

/**
 * Player character specific stats
 */
enum CharacterStats
{
    CHAR_SKILL_WEAPON_UNARMED = NB_ATTRIBUTES_BEING,
    CHAR_SKILL_WEAPON_SWORD,
    CHAR_SKILL_WEAPON_AXE,
    CHAR_SKILL_WEAPON_POLEARM,
    CHAR_SKILL_WEAPON_JAVELIN,
    CHAR_SKILL_WEAPON_WHIP,
    CHAR_SKILL_WEAPON_DAGGER,
    CHAR_SKILL_WEAPON_STAFF,
    CHAR_SKILL_WEAPON_BOW,
    CHAR_SKILL_WEAPON_CROSSBOW,
    CHAR_SKILL_WEAPON_THROWN,
    NB_CHAR_WEAPONSKILLS,

    CHAR_SKILL_MAGIC_IAMJUSTAPLACEHOLDER = NB_CHAR_WEAPONSKILLS,
    NB_CHAR_MAGICSKILLS,

    CHAR_SKILL_CRAFT_IAMJUSTAPLACEHOLDER = NB_CHAR_MAGICSKILLS,
    NB_CHAR_CRAFTSKILLS,

    CHAR_SKILL_IAMJUSTAPLACEHOLDER = NB_CHAR_CRAFTSKILLS,
    NB_CHAR_OTHERSKILLS,

    NB_ATTRIBUTES_CHAR = NB_CHAR_OTHERSKILLS
};


/**
 * The local player character.
 */
class LocalPlayer : public Player
{
    public:
        enum Attribute {
            STR = 0, AGI, DEX, VIT, INT, WIL, CHR
        };

        /**
         * Constructor.
         */
        LocalPlayer();

        /**
         * Destructor.
         */
        ~LocalPlayer();

        virtual void logic();

        /**
         * Adds a new step when walking before calling super. Also, when
         * specified it picks up an item at the end of a path.
         */
        virtual void nextStep();

        /**
         * Draws the name text below the being.
         */
        virtual void
        drawName(Graphics *, int, int) {};

        virtual Type getType() const;

        void clearInventory();
        Item* getInvItem(int index);

        /**
         * Move the Inventory item from the old slot to the new slot.
         */
        void moveInvItem(Item *item, int newIndex);

        /**
         * Equips an item.
         */
        void equipItem(Item *item);

        /**
         * Unequips an item.
         */
        void unequipItem(int slot);

        void useItem(int slot);

        void dropItem(Item *item, int quantity);

        void splitItem(Item *item, int quantity);

        void pickUp(FloorItem *item);

        /**
         * Sets the attack range.
         */
        void setAttackRange(int range) { mAttackRange = range; }

        /**
         * Gets the attack range.
         */
        int getAttackRange() const { return mAttackRange; }

        /**
         * Sents a trade request to the given being.
         */
        void trade(Being *being) const;

        /**
         * Accept or decline a trade offer
         */
        void tradeReply(bool accept);

        /**
         * Returns true when the player is ready to accept a trade offer.
         * Returns false otherwise.
         */
        bool tradeRequestOk() const;

        /**
         * Sets the trading state of the player, i.e. whether or not he is
         * currently involved into some trade.
         */
        void setTrading(bool trading) { mTrading = trading; }

        void attack();

        Being* getTarget() const;

        /**
         * Overridden to do nothing. The attacks of the local player are
         * displayed as soon as the player attacks, not when the server says
         * the player does.
         */
        virtual void handleAttack() {}

        /**
         * Sets the target being of the player.
         */
        void setTarget(Being* target) { mTarget = target; }

        /**
         * Sets a new destination for this being to walk to.
         */
        void setDestination(Uint16 x, Uint16 y);

        /**
         * Sets a new direction to keep walking in.
         */
        void setWalkingDir(int dir);

        void raiseAttribute(size_t attr);

        void toggleSit();
        void emote(Uint8 emotion);

        void revive();

        int getHP() const
        { return mHP; }

        int getMaxHP() const
        { return mMaxHP; }

        Uint16 mAttackRange;

        void setHP(int value)
        { mHP = value; }

        void setMaxHP(int value)
        { mMaxHP = value; }

        int getLevel() const
        { return mLevel; }

        void setLevel(int value)
        { mLevel = value; }

        int getMoney() const
        { return mMoney; }

        void setMoney(int value)
        { mMoney = value; }

        int getTotalWeight() const
        { return mTotalWeight; }

        int getMaxWeight() const
        { return mMaxWeight; }

        int getAttributeBase(size_t num) const
        { return mAttributeBase.at(num); }

        void setAttributeBase(size_t num, int value)
        { mAttributeBase.at(num) = value; }

        int getAttributeEffective(size_t num) const
        { return mAttributeEffective.at(num); }

        void setAttributeEffective(size_t num, int value)
        { mAttributeEffective.at(num) = value; }

        int getAttributeIncreasePoints() const
        { return mAttributeIncreasePoints; }

        float mLastAttackTime; /**< Used to synchronize the charge dialog */

        std::auto_ptr<Inventory> mInventory;

    protected:
        void walk(unsigned char dir);

        // Character status:
        std::vector<int> mAttributeBase;
        std::vector<int> mAttributeEffective;
        int mAttributeIncreasePoints;
        int mLevel;
        int mMoney;
        int mTotalWeight;
        int mMaxWeight;
        int mHP;
        int mMaxHP;

        Being *mTarget;
        FloorItem *mPickUpTarget;

        bool mTrading;
        int mLastAction;    /**< Time stamp of the last action, -1 if none. */
        int mWalkingDir;    /**< The direction the player is walking in. */
        int mDestX;         /**< X coordinate of destination. */
        int mDestY;         /**< Y coordinate of destination. */
};

extern LocalPlayer *player_node;

#endif
