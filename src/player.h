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

#ifndef PLAYER_H
#define PLAYER_H

#include "being.h"

class FlashText;
class Graphics;
class Map;
#ifdef TMWSERV_SUPPORT
class Guild;
#endif

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

        ~Player();

        /**
         * Set up mName to be the character's name
         */
        virtual void setName(const std::string &name);

#ifdef EATHENA_SUPPORT
        virtual void logic();
#endif

        virtual Type getType() const;

        virtual void setGender(Gender gender);

        /**
         * Whether or not this player is a GM.
         */
        bool isGM() const { return mIsGM; }

        /**
         * Triggers whether or not to show the name as a GM name.
         */
        virtual void setGM(bool gm);

        /**
         * Sets the hair style and color for this player.
         *
         * Only for convenience in 0.0 client. When porting
         * this to the trunk remove this function and
         * call setSprite directly instead. The server should
         * provide the hair ID and coloring in the same way
         * it does for other equipment pieces.
         *
         */
        void setHairStyle(int style, int color);

        /**
         * Sets visible equipments for this player.
         */
        virtual void setSprite(int slot, int id, const std::string &color = "");

        /**
         * Flash the player's name
         */
        void flash(int time);

#ifdef TMWSERV_SUPPORT
        /**
         * Adds a guild to the player.
         */
        Guild *addGuild(short guildId, short rights);

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
         * Get number of guilds the player belongs to.
         */
        short getNumberOfGuilds();

#endif

        /**
         * Set whether the player in the LocalPlayer's party. Players that are
         * in the same party as the local player get their name displayed in
         * a different color.
         */
        void setInParty(bool inParty);

        bool isInParty() const { return mInParty; }

        /**
         * Gets the way the character is blocked by other objects.
         */
        virtual unsigned char getWalkMask() const
        { return Map::BLOCKMASK_WALL | Map::BLOCKMASK_MONSTER; }

        /**
         * Called when a option (set with config.addListener()) is changed
         */
        void optionChanged(const std::string &value);

    protected:
        /**
         * Gets the way the monster blocks pathfinding for other objects.
         */
        virtual Map::BlockType getBlockType() const
        { return Map::BLOCKTYPE_CHARACTER; }

        virtual void updateCoords();

#ifdef TMWSERV_SUPPORT
        // Character guild information
        std::map<int, Guild*> mGuilds;
#endif

        bool mShowName;
        FlashText *mName;

        bool mIsGM;

    private:
        bool mInParty;
};

#endif
