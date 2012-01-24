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

#ifndef BEING_H
#define BEING_H

#include "actorsprite.h"
#include "eventlistener.h"
#include "map.h"
#include "position.h"
#include "vector.h"

#include <guichan/color.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

#define FIRST_IGNORE_EMOTE 14
#define STATUS_EFFECTS 32

#define SPEECH_TIME 500
#define SPEECH_MAX_TIME 1000

class BeingInfo;
class FlashText;
class Guild;
class ItemInfo;
class Item;
class Particle;
class Party;
class Position;
class SpeechBubble;
class Text;

enum Gender
{
    GENDER_MALE = 0,
    GENDER_FEMALE = 1,
    GENDER_UNSPECIFIED = 2
};

class Being : public ActorSprite, public EventListener
{
    public:
        /**
         * Action the being is currently performing
         * WARNING: Has to be in sync with the same enum in the Being class
         * of the server!
         */
        enum Action
        {
            STAND,
            MOVE,
            ATTACK,
            SIT,
            DEAD,
            HURT
        };

        enum Speech
        {
            NO_SPEECH = 0,
            TEXT_OVERHEAD,
            NO_NAME_IN_BUBBLE,
            NAME_IN_BUBBLE
        };

        enum AttackType
        {
            HIT = 0x00,
            CRITICAL = 0x0a,
            MULTI = 0x08,
            REFLECT = 0x04,
            FLEE = 0x0b
        };

        /**
         * Directions, to be used as bitmask values
         */
        enum BeingDirection
        {
            DOWN = 1,
            LEFT = 2,
            UP = 4,
            RIGHT = 8
        };

        /**
         * Constructor.
         *
         * @param id      a unique being id
         * @param subtype partly determines the type of the being
         * @param map     the map the being is on
         */
        Being(int id, Type type, int subtype, Map *map);

        virtual ~Being();

        Type getType() const
        { return mType; }

        /**
         * Removes all path nodes from this being.
         */
        void clearPath();

        /**
         * Creates a path for the being from current position to ex and ey
         */
        void setDestination(int ex, int ey);

        /**
         * Returns the destination for this being.
         */
        const Vector &getDestination() const
        { return mDest; }

        /**
         * Returns the tile x coord
         */
        int getTileX() const
        { return mPos.x / mMap->getTileWidth(); }

        /**
         * Returns the tile y coord
         */
        int getTileY() const
        { return mPos.y / mMap->getTileHeight(); }

        /**
         * Puts a "speech balloon" above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setSpeech(const std::string &text, int time = 500);

        /**
         * Puts a damage bubble above this being.
         *
         * @param attacker the attacking being
         * @param damage the amount of damage recieved (0 means miss)
         * @param type the attack type
         * @param attackId the attack id (used for monsters)
         */
        void takeDamage(Being *attacker, int damage,
                        AttackType type, int attackId = 1);

        /**
         * Handles an attack of another being by this being.
         *
         * @param victim the victim being
         * @param damage the amount of damage dealt (0 means miss)
         * @param attackId the attack id
         */
        virtual void handleAttack(Being *victim, int damage, int attackId = 1);

        const ItemInfo *getEquippedWeapon() const
        { return mEquippedWeapon; }

        /**
         * Returns the name of the being.
         */
        const std::string &getName() const
        { return mName; }

        /**
         * Sets the name for the being.
         *
         * @param name The name that should appear.
         */
        void setName(const std::string &name);

        bool getShowName() const
        { return mShowName; }

        void setShowName(bool doShowName);

        /**
         * Sets the name of the party the being is in. Shown in BeingPopup.
         */
        void setPartyName(const std::string &name) { mPartyName = name; }

        const std::string &getPartyName() const { return mPartyName; }

        /**
         * Sets the name of the primary guild the being is in. Shown in
         * BeingPopup (eventually).
         */
        void setGuildName(const std::string &name);

        void setGuildPos(const std::string &pos);

        /**
         * Adds a guild to the being.
         */
        void addGuild(Guild *guild);

        /**
         * Removers a guild from the being.
         */
        void removeGuild(int id);

        /**
         * Returns a pointer to the specified guild that the being is in.
         */
        Guild *getGuild(const std::string &guildName) const;

        /**
         * Returns a pointer to the specified guild that the being is in.
         */
        Guild *getGuild(int id) const;

        /**
         * Returns all guilds the being is in.
         */
        const std::map<int, Guild*> &getGuilds() const
        { return mGuilds; }

        /**
         * Removes all guilds the being is in.
         */
        void clearGuilds();

        /**
         * Get number of guilds the being belongs to.
         */
        short getNumberOfGuilds() const
        { return mGuilds.size(); }

        bool isInParty() const
        { return mParty != NULL; }

        void setParty(Party *party);

        Party *getParty() const
        { return mParty; }

        /**
         * Sets visible equipments for this being.
         */
        void setSprite(unsigned int slot, int id,
                       const std::string &color = std::string(),
                       bool isWeapon = false);

        void setSpriteID(unsigned int slot, int id);

        void setSpriteColor(unsigned int slot, const std::string &color = "");

        /**
         * Get the number of hairstyles implemented
         */
        static int getNumOfHairstyles()
        { return mNumberOfHairstyles; }

        /**
         * Get the number of layers used to draw the being
         */
        int getNumberOfLayers() const;

        virtual bool drawnWhenBehind() const
        { return CompoundSprite::drawnWhenBehind(); }

        /**
         * Performs being logic.
         */
        virtual void logic();

        /**
         * Draws the speech text above the being.
         */
        void drawSpeech(int offsetX, int offsetY);

        uint16_t getSubType() const { return mSubType; }

         /**
          * Set Being's subtype (mostly for view for monsters and NPCs)
          */
        void setSubtype(uint16_t subtype);

        const BeingInfo *getInfo() const
        { return mInfo; }

        TargetCursorSize getTargetCursorSize() const;

        /**
         * Gets the way the object is blocked by other objects.
         */
        unsigned char getWalkMask() const;

        /**
         * Gets the way the monster blocks pathfinding for other objects
         */
        Map::BlockType getBlockType() const;

        /**
         * Sets the move speed.
         * in ticks per tile for eAthena,
         * in tiles per second for Manaserv (0.1 precision).
         */
        virtual void setMoveSpeed(const Vector &speed);

        /**
         * Gets the original Move speed.
         * in ticks per tile for eAthena,
         * in tiles per second for Manaserv (0.1 precision).
         */
        Vector getMoveSpeed() const { return mMoveSpeed; }

        /**
         * Sets the attack speed.
         * @note The attack speed is to be set in milliseconds
         * before next attack.
         */
        void setAttackSpeed(int speed) { mAttackSpeed = speed; }

        /**
         * Gets the attack speed.
         * @note The attack speed is in milliseconds before next attack.
         */
        int getAttackSpeed() const { return mAttackSpeed; }

        /**
         * Sets the current action.
         */
        virtual void setAction(Action action, int attackId = 1);

        /**
         * Get the being's action currently performed.
         */
        Action getCurrentAction() const { return mAction; }

        /**
         * Returns whether this being is still alive.
         */
        bool isAlive() const { return mAction != DEAD; }

        /**
         * Returns the current direction.
         */
        uint8_t getDirection() const { return mDirection; }

        /**
         * Sets the current direction.
         */
        void setDirection(uint8_t direction);

        /**
         * Returns the direction the being is facing.
         */
        SpriteDirection getSpriteDirection() const
        { return SpriteDirection(mSpriteDirection); }

        void setPosition(const Vector &pos);

        /**
         * Overloaded method provided for convenience.
         *
         * @see setPosition(const Vector &pos)
         */
        void setPosition(float x, float y, float z = 0.0f)
        {
            setPosition(Vector(x, y, z));
        }

        /**
         * Returns the being's pixel radius used to detect collisions.
         */
        virtual int getCollisionRadius() const;

        /**
         * Shoots a missile particle from this being, to target being
         */
        void fireMissile(Being *target, const std::string &particle);

        /**
         * Returns the path this being is following. An empty path is returned
         * when this being isn't following any path currently.
         */
        const Path &getPath() const { return mPath; }

        static void load();

        void flashName(int time);

        int getDamageTaken() const
        { return mDamageTaken; }

        void updateName();

        /**
         * Sets the gender of this being.
         */
        virtual void setGender(Gender gender);

        Gender getGender() const
        { return mGender; }

        /**
         * Whether or not this player is a GM.
         */
        bool isGM() const
        { return mIsGM; }

        /**
         * Triggers whether or not to show the name as a GM name.
         */
        void setGM(bool gm);

        /**
         * Sets the IP or an IP hash.
         * The TMW-Athena server sends this information only to GMs.
         */
        void setIp(int ip) { mIp = ip; }

        /**
         * Returns the player's IP or an IP hash.
         * Value is 0 if not set by the server.
         */
        int getIp() const { return mIp; }

        bool canTalk();

        void talkTo();

        void event(Event::Channel channel, const Event &event);

        void setMap(Map *map);

        /**
         * Make the being look at a given pixel position.
         */
        void lookAt(const Vector &destPos);

    protected:
        /**
         * Sets the new path for this being.
         */
        void setPath(const Path &path);

        /**
         * Updates name's location.
         */
        void updateCoords();

        void showName();

        void updateColors();

        /**
         * Gets the advised Y chat text position.
         */
        int getSpeechTextYPosition() const;

        BeingInfo *mInfo;

        int mActionTime;      /**< Time spent in current action. TODO: Remove use of it */

        /** Time until the last speech sentence disappears */
        int mSpeechTime;

        int mAttackSpeed;     /**< Attack speed */

        Action mAction;       /**< Action the being is performing */
        uint16_t mSubType;      /**< Subtype (graphical view, basically) */

        uint8_t mDirection;             /**< Facing direction */
        uint8_t mSpriteDirection;       /**< Facing direction */
        std::string mName;              /**< Name of character */
        std::string mPartyName;

        /**
         * Holds a text object when the being displays it's name, 0 otherwise
         */
        FlashText *mDispName;
        const gcn::Color *mNameColor;
        bool mShowName;

        /** Engine-related infos about weapon. */
        const ItemInfo *mEquippedWeapon;

        static int mNumberOfHairstyles;          /** Number of hair styles in use */

        Path mPath;
        std::string mSpeech;
        Text *mText;
        const gcn::Color *mTextColor;

        Vector mDest;  /**< destination coordinates. */

        std::vector<int> mSpriteIDs;
        std::vector<std::string> mSpriteColors;
        Gender mGender;

        // Character guild information
        std::map<int, Guild*> mGuilds;
        Party *mParty;

        bool mIsGM;

    private:

        const Type mType;

        /** Speech Bubble components */
        SpeechBubble *mSpeechBubble;

        /**
         * Walk speed for x and y movement values.
         * In ticks per tile for eAthena,
         * In pixels per second for Manaserv.
         */
        Vector mMoveSpeed;

        /**
         * Being speed in pixel per ticks. Used internally for the being logic.
         * @see MILLISECONDS_IN_A_TICK
         */
        Vector mSpeedPixelsPerTick;

        int mDamageTaken;

        int mIp;
};

#endif
