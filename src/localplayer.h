/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include <memory>
#include <vector>

#include "player.h"

class Equipment;
class FloorItem;
class ImageSet;
class Inventory;
class Item;
class Map;

#ifdef TMWSERV_SUPPORT

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

#endif

/**
 * The local player character.
 */
class LocalPlayer : public Player
{
    public:
        enum Attribute
        {
#ifdef TMWSERV_SUPPORT
            STR = 0, AGI, DEX, VIT, INT, WIL, CHR
#else
            STR = 0, AGI, VIT, INT, DEX, LUK
#endif
        };

        /**
         * Constructor.
         */
#ifdef TMWSERV_SUPPORT
        LocalPlayer();
#else
        LocalPlayer(int id, int job, Map *map);
#endif

        /**
         * Destructor.
         */
        ~LocalPlayer();

        virtual void setName(const std::string &name);

        virtual void logic();

        virtual void setAction(Action action, int attackType = 0);

        /**
         * Adds a new step when walking before calling super. Also, when
         * specified it picks up an item at the end of a path.
         */
        virtual void nextStep();

        /**
         * Returns the player's inventory.
         */
        Inventory *getInventory() const { return mInventory; }

        /**
         * Returns the player's storage
         */
        Inventory *getStorage() const { return mStorage; }

#ifdef TMWSERV_SUPPORT
        /**
         * Check the player has permission to invite users to specific guild
         */
        bool checkInviteRights(const std::string &guildName);

        /**
         * Invite a player to join guild
         */
        void inviteToGuild(Being *being);

        void clearInventory();
        void setInvItem(int index, int id, int amount);
#endif

        /**
         * Invite a player to join their party
         */
        void inviteToParty(const std::string &name);

        /**
         * Invite a player to join their party
         */
        void inviteToParty(Player *player);

        /**
         * Equips an item.
         */
        void equipItem(Item *item);

        /**
         * Unequips an item.
         */
        void unequipItem(Item *item);

        void useItem(Item *item);

        void dropItem(Item *item, int quantity);

        void splitItem(Item *item, int quantity);

        void pickUp(FloorItem *item);

#ifdef EATHENA_SUPPORT
        /**
         * Sets the attack range.
         */
        void setAttackRange(int range) { mAttackRange = range; }
#endif

        /**
         * Gets the attack range.
         */
        int getAttackRange();

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

#ifdef TMWSERV_SUPPORT
        void attack();
        void useSpecial(int id);
#else
        void attack(Being *target = NULL, bool keep = false);
#endif

        /**
         * Triggers whether or not to show the name as a GM name.
         */
        virtual void setGM();

        void stopAttack();

        /**
         * Overridden to do nothing. The attacks of the local player are
         * displayed as soon as the player attacks, not when the server says
         * the player does.
         *
         * @param victim the victim being
         * @param damage the amount of damage dealt (0 means miss)
         * @param type the attack type
         */
        //virtual void handleAttack(Being *victim, int damage, AttackType type) {}
        virtual void handleAttack() {}

        /**
         * Returns the current target of the player. Returns 0 if no being is
         * currently targeted.
         */
        Being* getTarget() const;

        /**
         * Sets the target being of the player.
         */
        void setTarget(Being* target);

        /**
         * Sets a new destination for this being to walk to.
         */
#ifdef TMWSERV_SUPPORT
        void setDestination(int x, int y);
#else
        virtual void setDestination(Uint16 x, Uint16 y);
#endif

        /**
         * Sets a new direction to keep walking in.
         */
        void setWalkingDir(int dir);

        /**
         * Gets the walking direction
         */
        int getWalkingDir() const
        { return mWalkingDir; }

        /**
         * Sets going to being to attack
         */
        void setGotoTarget(Being *target);

        /**
         * Returns whether the target is in range to attack
         */
        bool withinAttackRange(Being *target);

#ifdef EATHENA_SUPPORT
        void raiseSkill(Uint16 skillId);
#else

        /**
         * Stops the player dead in his tracks
         */
        void stopWalking(bool sendToServer = true);

        /**
         * Uses a character point to raise an attribute
         */
        void raiseAttribute(size_t attr);

        /**
         * Uses a correction point to lower an attribute
         */
        void lowerAttribute(size_t attr);
#endif

        void toggleSit();
        void emote(Uint8 emotion);

        void revive();

        /**
         * Shows item pickup effect if the player is on a map.
         */
        void pickedUp(const std::string &item);

        /**
         * Accessors for mInStorage
         */
        bool getInStorage() { return mInStorage; }
        void setInStorage(bool inStorage);

#ifdef EATHENA_SUPPORT
        /**
         * Sets the amount of XP. Shows XP gaining effect if the player is on
         * a map.
         */
        void setXp(int xp);

        /**
         * Returns the amount of experience points.
         */
        int getXp() const { return mXp; }

        Uint32 mCharId;     /**< Used only during character selection. */

        Uint32 mJobXp;
        Uint32 mJobLevel;
        Uint32 mXpForNextLevel, mJobXpForNextLevel;
        Uint16 mMp, mMaxMp;

        Uint16 mAttackRange;

        Uint8 mAttr[6];
        Uint8 mAttrUp[6];

        int ATK, MATK, DEF, MDEF, HIT, FLEE;
        int ATK_BONUS, MATK_BONUS, DEF_BONUS, MDEF_BONUS, FLEE_BONUS;

        Uint16 mStatPoint, mSkillPoint;
        Uint16 mStatsPointsToAttribute;
#endif

        int getHp() const
        { return mHp; }

        int getMaxHp() const
        { return mMaxHp; }

        void setHp(int value)
        { mHp = value; }

        void setMaxHp(int value)
        { mMaxHp = value; }

        int getLevel() const
        { return mLevel; }

        void setLevel(int value)
        { mLevel = value; }

#ifdef TMWSERV_SUPPORT
        void setLevelProgress(int percent)
        { mLevelProgress = percent; }

        int getLevelProgress() const
        { return mLevelProgress; }
#endif

        int getMoney() const
        { return mMoney; }

        void setMoney(int value)
        { mMoney = value; }

        int getTotalWeight() const
        { return mTotalWeight; }

        void setTotalWeight(int value)
        { mTotalWeight = value; }

        int getMaxWeight() const
        { return mMaxWeight; }

        void setMaxWeight(int value)
        { mMaxWeight = value; }

#ifdef TMWSERV_SUPPORT
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

        struct SkillInfo {
            std::string name;
            std::string icon;
        };

        static const SkillInfo& getSkillInfo(int skill);

        std::pair<int, int> getExperience(int skill);
#endif

        bool mUpdateName;     /** Whether or not the name settings have changed */

        bool mMapInitialized; /** Whether or not the map is available yet */

        const std::auto_ptr<Equipment> mEquipment;

    protected:
        virtual void handleStatusEffect(StatusEffect *effect, int effectId);

        void walk(unsigned char dir);

        bool mInStorage;      /**< Whether storage is currently accessible */
#ifdef EATHENA_SUPPORT
        int mXp;            /**< Experience points. */
        int mTargetTime;      /** How long the being has been targeted **/
#endif
        int mLastTarget;      /** Time stamp of last targeting action, -1 if none. */

#ifdef TMWSERV_SUPPORT
        // Character status:
        std::vector<int> mAttributeBase;
        std::vector<int> mAttributeEffective;
        std::vector<int> mExpCurrent;
        std::vector<int> mExpNext;
        int mCharacterPoints;
        int mCorrectionPoints;
        int mLevelProgress;
#endif
        int mLevel;
        int mMoney;
        int mTotalWeight;
        int mMaxWeight;
        int mHp;
        int mMaxHp;

        Being *mTarget;
        FloorItem *mPickUpTarget;

        bool mTrading;
        bool mGoingToTarget;
        bool mKeepAttacking;  /** Whether or not to continue to attack */
        int mLastAction;      /**< Time stamp of the last action, -1 if none. */
        int mWalkingDir;      /**< The direction the player is walking in. */
        int mDestX;           /**< X coordinate of destination. */
        int mDestY;           /**< Y coordinate of destination. */

        std::vector<int> mStatusEffectIcons;

        Inventory *mInventory;

#ifdef TMWSERV_SUPPORT
        int mLocalWalkTime;   /**< Timestamp used to control keyboard walk
                                  messages flooding */
#endif

        Inventory *mStorage;

        // Load the target cursors into memory
        void initTargetCursor();

        /**
         * Helper function for loading target cursors
         */
        void loadTargetCursor(std::string filename, int width, int height,
                              bool outRange, Being::TargetCursorSize size);

        /** Images of the target cursor. */
        ImageSet *mTargetCursorImages[2][NUM_TC];

        /** Animated target cursors. */
        SimpleAnimation *mTargetCursor[2][NUM_TC];

#ifdef TMWSERV_SUPPORT
        std::list<std::string> mExpMessages; /**< Queued exp messages*/
        int mExpMessageTime;
#endif
};

extern LocalPlayer *player_node;

#endif
