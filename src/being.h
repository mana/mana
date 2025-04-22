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

#include "actorsprite.h"
#include "eventlistener.h"
#include "gui/gui.h"
#include "map.h"
#include "position.h"
#include "vector.h"

#include "utils/time.h"

#include <guichan/color.hpp>
#include <guichan/deathlistener.hpp>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#define STATUS_EFFECTS 32

#define SPEECH_TIME 500
#define SPEECH_MAX_TIME 10000

class BeingInfo;
class FlashText;
class Guild;
class ItemInfo;
class Particle;
class Party;
struct Position;
class SpeechBubble;
class Text;

enum class Gender
{
    Male = 0,
    Female = 1,
    Unspecified = 2,
    Hidden = 3
};

class Being : public ActorSprite, public EventListener, public gcn::DeathListener
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
         * @param type    type of being
         * @param subtype refers to a specific npc, monster, species, etc.
         * @param map     the map the being is on
         */
        Being(int id, Type type, int subtype, Map *map);

        ~Being() override;

        Type getType() const final
        { return mType; }

        void setType(Type type, int subtype);

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
        int getTileX() const override
        { return mPos.x / mMap->getTileWidth(); }

        /**
         * Returns the tile y coord
         */
        int getTileY() const override
        { return mPos.y / mMap->getTileHeight(); }

        /**
         * Puts a "speech balloon" above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setSpeech(const std::string &text, int time = 5000);

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
        void handleAttack(Being *victim, int damage, int attackId = 1);

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
        { return mParty != nullptr; }

        void setParty(Party *party);

        Party *getParty() const
        { return mParty; }

        /**
         * Sets visible equipments for this being.
         */
        void setSprite(unsigned slot, int id,
                       const std::string &color = std::string(),
                       bool isWeapon = false);

        void setSpriteID(unsigned slot, int id);

        void setSpriteColor(unsigned slot,
                            const std::string &color = std::string());

        unsigned getSpriteCount() const
        { return mSpriteStates.size(); }

        bool drawnWhenBehind() const override;

        /**
         * Performs being logic.
         */
        void logic() override;

        /**
         * Draws the speech text above the being.
         */
        void drawSpeech(int offsetX, int offsetY);

        uint16_t getSubType() const { return mSubType; }

        const BeingInfo &getInfo() const
        { return *mInfo; }

        bool isTargetSelection() const;

        TargetCursorSize getTargetCursorSize() const override;

        Cursor getHoverCursor() const;

        /**
         * Gets the way the object is blocked by other objects.
         */
        unsigned char getWalkMask() const;

        /**
         * Gets the way the monster blocks pathfinding for other objects
         */
        Map::BlockType getBlockType() const override;

        /**
         * Sets the move speed.
         * in ticks per tile for eAthena,
         * in tiles per second for Manaserv (0.1 precision).
         */
        void setMoveSpeed(const Vector &speed);

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
         * Sets the current action by name.
         */
        void setAction(const std::string &action);

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

        void setPosition(const Vector &pos) final;

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
        int getCollisionRadius() const;

        /**
         * Shoots a missile particle from this being, to target being
         */
        void fireMissile(Being *target, const std::string &particle);

        void setStatusEffect(int id, bool active);

        /**
         * Returns the path this being is following. An empty path is returned
         * when this being isn't following any path currently.
         */
        const Path &getPath() const { return mPath; }

        void flashName(int time);

        int getDamageTaken() const
        { return mDamageTaken; }

        void updateName();

        /**
         * Sets the gender of this being.
         */
        void setGender(Gender gender);

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
        void setIp(int ip);

        /**
         * Returns the player's IP or an IP hash.
         * Value is 0 if not set by the server.
         */
        int getIp() const { return mIp; }

        bool canTalk();

        void talkTo();

        // EventListener
        void event(Event::Channel channel, const Event &event) override;

        // gcn::DeathListener
        void death(const gcn::Event &event) override;

        void setMap(Map *map) final;

        /**
         * Make the being look at a given pixel position.
         */
        void lookAt(const Vector &destVec);
        void lookAt(const Position &destPos)
        { lookAt(Vector(destPos.x, destPos.y)); }

    protected:
        struct SpriteState {
            int id = 0;
            int visibleId = 0;
            std::string color;
            std::vector<ParticleHandle> particles;
        };

        /**
         * Sets the new path for this being.
         */
        void setPath(const Path &path);

        /**
         * Updates name's location.
         */
        void updateNamePosition();

        void addSpriteParticles(SpriteState &spriteState, const SpriteDisplay &display);
        void restoreAllSpriteParticles();

        void updateColors();
        void updatePlayerSprites();

        /**
         * Gets the advised Y chat text position.
         */
        int getSpeechTextYPosition() const;

        /**
         * Called when the being has reached the end of its path.
         */
        virtual void pathFinished() {}

        /**
         * Notify self that a status effect has flipped.
         * The new flag is passed.
         */
        virtual void updateStatusEffect(int index, bool newStatus);

        const BeingInfo *mInfo;

        Timer mActionTimer;     /**< Time spent in current action. TODO: Remove use of it */

        /** Time until the last speech sentence disappears */
        Timer mSpeechTimer;

        int mAttackSpeed = 350; /**< Attack speed */

        Action mAction = STAND;     /**< Action the being is performing */
        int mSubType = 0xFFFF;      /**< Subtype (graphical view, basically) */

        uint8_t mDirection = DOWN;                  /**< Facing direction */
        uint8_t mSpriteDirection = DIRECTION_DOWN;  /**< Facing direction */
        std::string mName;              /**< Name of character */
        std::string mPartyName;

        /**
         * Holds a text object when the being displays its name, 0 otherwise
         */
        FlashText *mDispName = nullptr;
        const gcn::Color *mNameColor;
        bool mShowName = false;

        /** Engine-related infos about weapon. */
        const ItemInfo *mEquippedWeapon = nullptr;

        Path mPath;
        std::string mSpeech;
        Text *mText = nullptr;

        Vector mDest;  /**< destination coordinates. */

        std::vector<SpriteState> mSpriteStates;
        bool mRestoreParticlesOnLogic = false;

        Gender mGender = Gender::Unspecified;

        // Character guild information
        std::map<int, Guild*> mGuilds;
        Party *mParty = nullptr;

        bool mIsGM = false;

    private:
        void updateMovement();

        Type mType = UNKNOWN;

        std::set<int> mStatusEffects;   /**< set of active status effects */
        std::map<int, ParticleHandle> mStatusParticleEffects;

        /** Speech Bubble components */
        SpeechBubble *mSpeechBubble;

        /**
         * Walk speed for x and y movement values.
         * In ticks per tile for eAthena,
         * In tiles per second for Manaserv.
         */
        Vector mMoveSpeed;

        /**
         * Being speed in pixel per second. Used internally for the being logic.
         */
        Vector mSpeedPixelsPerSecond;

        int mDamageTaken = 0;
        int mIp = 0;
};
