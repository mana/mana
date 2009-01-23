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

#ifndef _TMW_LOCALPLAYER_H
#define _TMW_LOCALPLAYER_H

#include "player.h"
#include "simpleanimation.h"

// TODO move into some sane place...
#define MAX_SLOT 2

#define INVENTORY_SIZE 102
#define STORAGE_SIZE 301

class FloorItem;
class ImageSet;
class Inventory;
class Item;
class Network;

/**
 * The local player character.
 */
class LocalPlayer : public Player
{
    public:
        enum Attribute {
            STR = 0, AGI, VIT, INT, DEX, LUK
        };

        /**
         * Constructor.
         */
        LocalPlayer(Uint32 id, Uint16 job, Map *map);

        /**
         * Destructor.
         */
        ~LocalPlayer();

        virtual void setName(const std::string &name);
        void setNetwork(Network *network) { mNetwork = network; }
        Network *getNetwork() {return mNetwork; }
        virtual void logic();

        /**
         * Adds a new step when walking before calling super. Also, when
         * specified it picks up an item at the end of a path.
         */
        virtual void nextStep();

        /**
         * Returns the player's inventory.
         */
        Inventory* getInventory() const { return mInventory; }

        /**
         * Returns the player's storage
         */
        Inventory* getStorage() const { return mStorage; }

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

        void attack(Being *target = NULL, bool keep = false);

        /**
         * Triggers whether or not to show the name as a GM name.
         * NOTE: This doesn't mean that just anyone can use this.
         * If the server doesn't acknowlege you, you won't be shown
         * as a GM on other people's clients.
         */
        virtual void setGM();

        void stopAttack();

        Being* getTarget() const;

        /**
         * Overridden to do nothing. The attacks of the local player are
         * displayed as soon as the player attacks, not when the server says
         * the player does.
         *
         * @param victim The attacked being.
         * @param damage The amount of damage dealt (0 means miss).
         */
        virtual void handleAttack(Being *victim, int damage) {}

        /**
         * Sets the target being of the player.
         */
        void setTarget(Being* target);

        /**
         * Sets a new destination for this being to walk to.
         */
        virtual void setDestination(Uint16 x, Uint16 y);

        /**
         * Sets a new direction to keep walking in.
         */
        void setWalkingDir(int dir);

        /**
         * Sets going to being to attack
         */
        void setGotoTarget(Being *target);

        /**
         * Returns whether the target is in range to attack
         */
        bool withinAttackRange(Being *target);

        void raiseAttribute(Attribute attr);
        void raiseSkill(Uint16 skillId);

        void toggleSit();
        void emote(Uint8 emotion);

        void revive();

        /**
         * Accessors for mInStorage
         */
        bool getInStorage() { return mInStorage; }
        void setInStorage(bool inStorage) { mInStorage = inStorage; }

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
        Uint16 mLevel;
        Uint32 mJobLevel;
        Uint32 mXpForNextLevel, mJobXpForNextLevel;
        Uint16 mHp, mMaxHp, mMp, mMaxMp;
        Uint32 mGp;

        Uint16 mAttackRange;

        Uint32 mTotalWeight, mMaxWeight;

        Uint8 mAttr[6];
        Uint8 mAttrUp[6];

        Sint16 ATK, MATK, DEF, MDEF, HIT, FLEE;
        Sint16 ATK_BONUS, MATK_BONUS, DEF_BONUS, MDEF_BONUS, FLEE_BONUS;

        Uint16 mStatPoint, mSkillPoint;
        Uint16 mStatsPointsToAttribute;

        bool mUpdateName;     /** Whether or not the name settings have changed */

        bool mMapInitialized; /** Whether or not the map is available yet */

        float mLastAttackTime; /**< Used to synchronize the charge dialog */

        void drawTargetCursor(Graphics *graphics, int offsetX, int offsetY);

        /** Animated in range target cursor. */
        SimpleAnimation *mTargetCursorInRange[NUM_TC];

        /** Animated out of range target cursor. */
        SimpleAnimation *mTargetCursorOutRange[NUM_TC];

    protected:
        void walk(unsigned char dir);

        int mXp;            /**< Experience points. */

        Network *mNetwork;
        Being *mTarget;
        FloorItem *mPickUpTarget;

        bool mTrading;
        bool mInStorage;      /**< Whether storage is currently accessible */
        bool mGoingToTarget;
        bool mKeepAttacking;  /** Whether or not to continue to attack */
        int mTargetTime;      /** How long the being has been targeted **/
        int mLastAction;      /**< Time stamp of the last action, -1 if none. */
        int mLastTarget;      /** Time stamp of last targeting action, -1 if none. */
        int mWalkingDir;      /**< The direction the player is walking in. */
        int mDestX;           /**< X coordinate of destination. */
        int mDestY;           /**< Y coordinate of destination. */

        Inventory *mInventory;
        Inventory *mStorage;

        /**
         * Helper function for loading target cursors
         */
        void loadTargetCursor(std::string filename, int width, int height,
                              bool outRange, Being::TargetCursorSize size);

        /** Images of in range target cursor. */
        ImageSet *mInRangeImages[NUM_TC];

        /** Images of out of range target cursor. */
        ImageSet *mOutRangeImages[NUM_TC];

        // Load the target cursors into memory
        void initTargetCursor();
};

extern LocalPlayer *player_node;

#endif
