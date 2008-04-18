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

#include "player.h"

#include <memory>

class FloorItem;
class Inventory;
class Item;

/**
 * Attributes used during combat. Available to all the beings.
 */
enum
{
BASE_ATTR_BEGIN = 0,
    BASE_ATTR_PHY_ATK_MIN = BASE_ATTR_BEGIN,
    BASE_ATTR_PHY_ATK_DELTA,
                       /**< Physical attack power. */
    BASE_ATTR_MAG_ATK, /**< Magical attack power. */
    BASE_ATTR_PHY_RES, /**< Resistance to physical damage. */
    BASE_ATTR_MAG_RES, /**< Resistance to magical damage. */
    BASE_ATTR_EVADE,   /**< Ability to avoid hits. */
    BASE_ATTR_HIT,     /**< Ability to hit stuff. */
    BASE_ATTR_HP,      /**< Hit Points (Base value: maximum, Modded value: current) */
    BASE_ATTR_HP_REGEN,/**< number of HP regenerated every 10 game ticks */
    BASE_ATTR_END,
    BASE_ATTR_NB = BASE_ATTR_END - BASE_ATTR_BEGIN,

    BASE_ELEM_BEGIN = BASE_ATTR_END,
    BASE_ELEM_NEUTRAL = BASE_ELEM_BEGIN,
    BASE_ELEM_FIRE,
    BASE_ELEM_WATER,
    BASE_ELEM_EARTH,
    BASE_ELEM_AIR,
    BASE_ELEM_SACRED,
    BASE_ELEM_DEATH,
    BASE_ELEM_END,
    BASE_ELEM_NB = BASE_ELEM_END - BASE_ELEM_BEGIN,

    NB_BEING_ATTRIBUTES = BASE_ELEM_END
};

/**
 * Attributes of characters. Used to derive being attributes.
 */
enum
{
    CHAR_ATTR_BEGIN = NB_BEING_ATTRIBUTES,
    CHAR_ATTR_STRENGTH = CHAR_ATTR_BEGIN,
    CHAR_ATTR_AGILITY,
    CHAR_ATTR_DEXTERITY,
    CHAR_ATTR_VITALITY,
    CHAR_ATTR_INTELLIGENCE,
    CHAR_ATTR_WILLPOWER,
    CHAR_ATTR_END,
    CHAR_ATTR_NB = CHAR_ATTR_END - CHAR_ATTR_BEGIN,

    CHAR_SKILL_BEGIN = CHAR_ATTR_END,

    CHAR_SKILL_WEAPON_BEGIN = CHAR_SKILL_BEGIN,
    CHAR_SKILL_WEAPON_NONE = CHAR_SKILL_WEAPON_BEGIN,
    CHAR_SKILL_WEAPON_KNIFE,
    CHAR_SKILL_WEAPON_SWORD,
    CHAR_SKILL_WEAPON_POLEARM,
    CHAR_SKILL_WEAPON_STAFF,
    CHAR_SKILL_WEAPON_WHIP,
    CHAR_SKILL_WEAPON_BOW,
    CHAR_SKILL_WEAPON_SHOOTING,
    CHAR_SKILL_WEAPON_MACE,
    CHAR_SKILL_WEAPON_AXE,
    CHAR_SKILL_WEAPON_THROWN,
    CHAR_SKILL_WEAPON_END,
    CHAR_SKILL_WEAPON_NB = CHAR_SKILL_WEAPON_END - CHAR_SKILL_WEAPON_BEGIN,

    CHAR_SKILL_MAGIC_BEGIN = CHAR_SKILL_WEAPON_END,
    CHAR_SKILL_MAGIC_IAMJUSTAPLACEHOLDER = CHAR_SKILL_MAGIC_BEGIN,
    // add magic skills here
    CHAR_SKILL_MAGIC_END,
    CHAR_SKILL_MAGIC_NB = CHAR_SKILL_MAGIC_END - CHAR_SKILL_MAGIC_BEGIN,

    CHAR_SKILL_CRAFT_BEGIN = CHAR_SKILL_MAGIC_END,
    CHAR_SKILL_CRAFT_IAMJUSTAPLACEHOLDER = CHAR_SKILL_CRAFT_BEGIN,
    // add crafting skills here
    CHAR_SKILL_CRAFT_END,
    CHAR_SKILL_CRAFT_NB = CHAR_SKILL_CRAFT_END - CHAR_SKILL_CRAFT_BEGIN,

    CHAR_SKILL_END = CHAR_SKILL_CRAFT_END,
    CHAR_SKILL_NB = CHAR_SKILL_END - CHAR_SKILL_BEGIN,

    NB_CHARACTER_ATTRIBUTES = CHAR_SKILL_END
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

        /**
         * Check the player has permission to invite users to specific guild
         */
        bool checkInviteRights(const std::string &guildName);

        /**
         * Invite a player to join guild
         */
        void invite(Being* being);

        void clearInventory();
        void setInvItem(int index, int id, int amount);

        /**
         * Move the Inventory item from the old slot to the new slot.
         */
        void moveInvItem(Item *item, int newIndex);

        /**
         * Searches for the specified item by it's identification.
         *
         * @param itemId The id of the item to be searched.
         * @return Item found on success, NULL on failure.
         */
        Item* searchForItem(int itemId);

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

        /**
         * Uses a character point to raise an attribute
         */
        void raiseAttribute(size_t attr);

        /**
         * Uses a correction point to lower an attribute
         */
        void lowerAttribute(size_t attr);

        void toggleSit();
        void emote(Uint8 emotion);

        void revive();

        int getHP() const
        { return mHP; }

        Uint32 mCharId;

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

        void setLevelProgress(int percent)
        { mLevelProgress = percent; }

        int getLevelProgress() const
        { return mLevelProgress; }

        int getMoney() const
        { return mMoney; }

        void setMoney(int value)
        { mMoney = value; }

        int getTotalWeight() const
        { return mTotalWeight; }

        int getMaxWeight() const
        { return mMaxWeight; }

        int getAttributeBase(int num) const
        { return mAttributeBase[num]; }

        void setAttributeBase(int num, int value)
        { mAttributeBase[num] = value; }

        int getAttributeEffective(int num) const
        { return mAttributeEffective[num]; }

        void setAttributeEffective(int num, int value)
        { mAttributeEffective[num] = value; }

        int getCharacterPoints() const
        { return mCharacterPoints; }

        void setCharacterPoints(int n)
        { mCharacterPoints = n; }

        int getCorrectionPoints() const
        { return mCorrectionPoints; }

        void setCorrectionPoints(int n)
        { mCorrectionPoints = n; }

        void setExperience(int skill, int current, int next);

        static const std::string& getSkillName(int skill);

        std::pair<int, int> getExperience(int skill);

        float mLastAttackTime; /**< Used to synchronize the charge dialog */

        Inventory *mInventory;
        const std::auto_ptr<Equipment> mEquipment;

    protected:
        void walk(unsigned char dir);

        // Character status:
        std::vector<int> mAttributeBase;
        std::vector<int> mAttributeEffective;
        std::vector<int> mExpCurrent;
        std::vector<int> mExpNext;
        int mCharacterPoints;
        int mCorrectionPoints;
        int mLevel;
        int mLevelProgress;
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
        int mLocalWalkTime;  /**< Timestamp used to control keyboard walk
                                 messages flooding */

        std::list<std::string> mExpMessages; /**< Queued exp messages*/
        int mExpMessageTime;

};

extern LocalPlayer *player_node;

#endif
