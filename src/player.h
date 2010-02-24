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

#ifndef PLAYER_H
#define PLAYER_H

#include "being.h"

class Graphics;
class Guild;
class Map;
class Party;

enum Gender
{
    GENDER_MALE = 0,
    GENDER_FEMALE = 1,
    GENDER_UNSPECIFIED = 2
};

/**
 * A player being. Players have their name drawn beneath them. This class also
 * implements player-specific loading of base sprite, hair sprite and equipment
 * sprites.
 */
class Player : public Being
{
    public:
        /**
         * Constructor.
         */
        Player(int id, int job, Map *map, bool isNPC = false);

        ~Player();

        virtual void logic();

        virtual Type getType() const { return PLAYER; }

        /**
         * Sets the gender of this being.
         */
        virtual void setGender(Gender gender);

        Gender getGender() const { return mGender; }

        /**
         * Whether or not this player is a GM.
         */
        bool isGM() const { return mIsGM; }

        /**
         * Triggers whether or not to show the name as a GM name.
         */
        virtual void setGM(bool gm);

        /**
         * Sets visible equipments for this player.
         */
        virtual void setSprite(int slot, int id,
                               const std::string &color = "",
                               bool isWeapon = false);

        virtual void setSpriteID(unsigned int slot, int id);

        virtual void setSpriteColor(unsigned int slot,
                                    const std::string &color = "");

        /**
         * Adds a guild to the player.
         */
        void addGuild(Guild *guild);

        /**
         * Removers a guild from the player.
         */
        void removeGuild(int id);

        /**
         * Returns a pointer to the specified guild.
         */
        Guild *getGuild(const std::string &guildName) const;

        /**
         * Returns a pointer to the guild with matching id.
         */
        Guild *getGuild(int id) const;

        /**
         * Returns all guilds the player is in.
         */
        const std::map<int, Guild*> &getGuilds() const;

        /**
         * Removes all guilds the player is in.
         */
        void clearGuilds();

        /**
         * Get number of guilds the player belongs to.
         */
        short getNumberOfGuilds() const { return mGuilds.size(); }

        bool isInParty() const { return mParty != NULL; }

        void setParty(Party *party);

        Party *getParty() const { return mParty; }

        /**
         * Gets the way the character is blocked by other objects.
         */
        virtual unsigned char getWalkMask() const
        { return Map::BLOCKMASK_WALL | Map::BLOCKMASK_MONSTER; }

        /**
         * Called when a option (set with config.addListener()) is changed
         */
        virtual void optionChanged(const std::string &value);

    protected:
        /**
         * Gets the way the monster blocks pathfinding for other objects.
         */
        virtual Map::BlockType getBlockType() const
        { return Map::BLOCKTYPE_CHARACTER; }

        virtual void updateColors();

        Gender mGender;
        std::vector<int> mSpriteIDs;
        std::vector<std::string> mSpriteColors;

        // Character guild information
        std::map<int, Guild*> mGuilds;
        Party *mParty;

        bool mIsGM;
};

#endif
