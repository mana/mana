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

#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "being.h"

#include "resources/userpalette.h"

#include <guichan/actionlistener.hpp>

#include <memory>
#include <vector>

class ChatTab;
class FloorItem;
class ImageSet;
class Item;
class Map;
class OkDialog;

/**
 * The local player character.
 */
class LocalPlayer : public Being
{
    public:
        /**
         * Constructor.
         */
        LocalPlayer(int id= 65535, int subtype = 0);

        /**
         * Destructor.
         */
        ~LocalPlayer();

        virtual void logic();

        virtual void setAction(Action action, int attackType = 0);

        /**
         * Compute the next pathnode location when walking using keyboard.
         * used by nextTile().
         */
        Position getNextWalkPosition(unsigned char dir);

        /**
         * Adds a new tile to the path when walking.
         * @note Eathena
         * Also, when specified, it picks up an item at the end of a path
         * or attack target.
         */
        virtual void nextTile()
        { nextTile(0); }

        virtual void nextTile(unsigned char dir);

        /**
         * Check the player has permission to invite users to specific guild
         */
        bool checkInviteRights(const std::string &guildName);

        /**
         * Invite a player to join guild
         */
        void inviteToGuild(Being *being);

        void pickUp(FloorItem *item);

        /**
         * Sets the attack range.
         */
        void setAttackRange(int range) { mAttackRange = range; }

        /**
         * Gets the attack range.
         */
        int getAttackRange();
        void attack(Being *target = NULL, bool keep = false);

        void setGMLevel(int level);

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
        Being *getTarget() const;

        /**
         * Sets the target being of the player.
         */
        void setTarget(Being *target);

        /**
         * Sets a new destination for this being to walk to.
         */
        virtual void setDestination(int x, int y);

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

        /**
         * Stops the player dead in his tracks
         */
        void stopWalking(bool sendToServer = true);

        void toggleSit();
        void emote(Uint8 emotion);

        /**
         * Shows item pickup notifications.
         */
        void pickedUp(const ItemInfo &itemInfo, int amount);

        void setShowIp(bool show)
        { mShowIp = show; }

        bool getShowIp() const
        { return mShowIp; }

        /** Tells that the path has been set by mouse. */
        void pathSetByMouse()
        { mPathSetByMouse = true; }

        /** Tells if the path has been set by mouse. */
        bool isPathSetByMouse() const
        { return mPathSetByMouse; }

        void addMessageToQueue(const std::string &message,
                               int color = UserPalette::EXP_INFO);

        void event(Channels channel, const Mana::Event &event);

        /**
         * Tells the engine wether to check
         * if the Player Name is to be displayed.
         */
        void setCheckNameSetting(bool checked) { mUpdateName = checked; }

        /**
         * Gets if the engine has to check
         * if the Player Name is to be displayed.
         */
        bool getCheckNameSetting() const { return mUpdateName; }

    protected:
        /** Whether or not the name settings have changed */
        bool mUpdateName;

        void startWalking(unsigned char dir);

        int mAttackRange;

        int mTargetTime;      /** How long the being has been targeted **/
        int mLastTarget;      /** Time stamp of last targeting action, -1 if none. */

        int mGMLevel;

        Being *mTarget;

        FloorItem *mPickUpTarget;

        bool mGoingToTarget;
        bool mKeepAttacking;  /** Whether or not to continue to attack */
        int mLastAction;      /**< Time stamp of the last action, -1 if none. */
        int mWalkingDir;      /**< The direction the player is walking in. */
        bool mPathSetByMouse; /**< Tells if the path was set using mouse */

        int mLocalWalkTime;   /**< Timestamp used to control keyboard walk
                                  messages flooding */

        typedef std::pair<std::string, int> MessagePair;
        /** Queued messages*/
        std::list<MessagePair> mMessages;
        int mMessageTime;

        bool mShowIp;
};

extern LocalPlayer *player_node;

#endif
