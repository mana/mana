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

#pragma once

#include "being.h"

#include "resources/userpalette.h"

#include "utils/time.h"

#include <guichan/actionlistener.hpp>

class ChatTab;
class FloorItem;
class ImageSet;
class Map;
class OkDialog;

class AwayListener : public gcn::ActionListener
{
    public:
        void action(const gcn::ActionEvent &event) override;
};

/**
 * Reasons an item can fail to be picked up.
 */
enum
{
    PICKUP_OKAY,
    PICKUP_BAD_ITEM,
    PICKUP_TOO_HEAVY,
    PICKUP_TOO_FAR,
    PICKUP_INV_FULL,
    PICKUP_STACK_FULL,
    PICKUP_DROP_STEAL
};

/**
 * The local player character.
 */
class LocalPlayer final : public Being
{
    public:
        LocalPlayer(int id = 65535, int subtype = 0);
        ~LocalPlayer() override;

        void logic() override;

        void setAction(Action action, int attackId = 1) override;

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
        void setAttackRange(int range);

        /**
         * Gets the attack range.
         */
        int getAttackRange() const
        { return mAttackRange; }

        void attack(Being *target = nullptr, bool keep = false);

        void setGMLevel(int level);

        void stopAttack();

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
        virtual void setDestination(const Vector &dest)
        { setDestination((int)dest.x, (int)dest.y); }
        virtual void setDestination(const Position &dest)
        { setDestination(dest.x, dest.y); }

        /**
         * Sets a new direction to keep walking in, when using the keyboard
         * or the joystick.
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
         * Cancel a possible target destination in progress,
         * but not the targeting.
         */
        void cancelGoToTarget()
        { mGoingToTarget = mKeepAttacking = false; }

        /**
         * Returns whether the target is in range (in pixels).
         */
        bool withinRange(Actor *target, int range) const;

        void toggleSit();
        void emote(int emoteId);

        /**
         * Shows item pickup notifications.
         */
        void pickedUp(const ItemInfo &itemInfo, int amount,
                      unsigned char fail);

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

        void changeAwayMode();

        bool getAwayMode() const
        { return mAwayMode; }

        void setAway(const std::string &message);

        void afkRespond(ChatTab *tab, const std::string &nick);

        void addMessageToQueue(const std::string &message,
                               int color = UserPalette::EXP_INFO);

        void event(Event::Channel channel, const Event &event) override;

    protected:
        /** Make the character starts to walk. */
        void startWalking(unsigned char dir);

        /**
         * Stops the player dead in his tracks
         */
        void stopWalking(bool sendToServer = true);

        /**
         * set the next path tile when walking and using the keyboard.
         */
        void nextTile(unsigned char dir);

        /**
         * Compute the next pathnode location when walking using keyboard.
         * used by nextTile().
         */
        Position getNextWalkPosition(unsigned char dir);

        void pathFinished() override { nextTile(mWalkingDir); }

        int mAttackRange = -1;

        Timer mLastTargetTimer; /**< Timer for last targeting action. */

        int mGMLevel = 0;

        Being *mTarget = nullptr;

        FloorItem *mPickUpTarget = nullptr;

        bool mGoingToTarget = false;
        bool mKeepAttacking = false;    /**< Whether or not to continue to attack */
        Timer mLastActionTimer;         /**< Timeout for the last action. */
        int mWalkingDir = 0;            /**< The direction the player is walking in. */
        bool mPathSetByMouse = false;   /**< Tells if the path was set using mouse */

        /** Queued messages */
        std::list<std::pair<std::string, int>> mMessages;
        Timer mMessageTimer;

        bool mShowIp = false;

        AwayListener *mAwayListener;
        OkDialog *mAwayDialog = nullptr;
        Timer mAfkTimer;
        bool mAwayMode = false;
};

extern LocalPlayer *local_player;
