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

#ifndef _TMW_PLAYER_H
#define _TMW_PLAYER_H

#include "being.h"

class Graphics;
class Map;
class Guild;

enum Gender {
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
        Player(int id, int job, Map *map);

        virtual ~Player();

        virtual Type
        getType() const;

        virtual void
        drawName(Graphics *graphics, int offsetX, int offsetY);

        /**
         * Sets the gender for this player.
         */
        void setGender(Gender);

        /**
         * Gets the hair color for this player.
         */
        int getHairColor() const
        { return mHairColor; }

        /**
         * Gets the hair style for this player.
         */
        int getHairStyle() const
        { return mHairStyle; }

        /**
         * Sets the hair style and color for this player.
         *
         * NOTE: This method was necessary for convenience in the 0.0 client.
         * It should be removed here since the server can provide the hair ID
         * and coloring the same way it does for other equipment pieces. Then
         * Being::setSprite can be used instead.
         */
        void setHairStyle(int style, int color);

        /**
         * Sets visible equipments for this player.
         */
        virtual void
        setSprite(int slot, int id, const std::string &color = "");

        /**
         * Adds a guild to the player.
         */
        Guild* addGuild(short guildId, short rights);

        /**
         * Removers a guild from the player.
         */
        void removeGuild(int id);

        /**
         * Returns a pointer to the specified guild.
         */
        Guild* getGuild(const std::string &guildName);

        /**
         * Returns a pointer to the guild with matching id.
         */
        Guild* getGuild(int id);

        /**
         * Get number of guilds the player belongs to.
         */
        short getNumberOfGuilds();

        /**
         * Set the player in party
         */
        void setInParty(bool value);

        /**
         * Returns whether player is in the party
         */
        bool getInParty() const { return mInParty; }

        /**
         * Gets the way the character is blocked by other objects.
         */
        virtual unsigned char getWalkMask() const
        { return 0x82; } // blocked by walls and monsters (bin 1000 0010)

    protected:
        /**
         * Gets the way the monster blocks pathfinding for other objects.
         */
        virtual Map::BlockType getBlockType() const
        { return Map::BLOCKTYPE_CHARACTER; }

        // Character guild information
        std::map<int, Guild*> mGuilds;

    private:
        Gender mGender;
        Uint8 mHairStyle;
        Uint8 mHairColor;
        bool mInParty;
};

#endif
