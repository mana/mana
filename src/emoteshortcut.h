/*
 *  Extended support for activating emotes
 *  Copyright (C) 2009  Aethyra Development Team
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

#define SHORTCUT_EMOTES 12

/**
 * The class which keeps track of the emote shortcuts.
 */
class EmoteShortcut
{
    public:
        EmoteShortcut();

        ~EmoteShortcut();

        /**
         * Load the configuration information.
         */
        void load();

        /**
         * Returns the shortcut Emote ID specified by the index.
         *
         * @param index Index of the shortcut Emote.
         */
        int getEmote(int index) const
        { return mEmotes[index] - 1; }

        /**
         * Returns the amount of shortcut Emotes.
         */
        int getEmoteCount() const
        { return SHORTCUT_EMOTES; }

        /**
         * Returns the emote ID that is currently selected.
         */
        int getEmoteSelected() const
        { return mEmoteSelected; }

        /**
         * Adds the selected emote ID to the emotes specified by the index.
         *
         * @param index Index of the emotes.
         */
        void setEmote(int index)
        { setEmotes(index, mEmoteSelected); }

        /**
         * Adds a emoticon to the emotes store specified by the index.
         *
         * @param index Index of the emote.
         * @param emoteId ID of the emote.
         */
        void setEmotes(int index, int emoteId)
        { mEmotes[index] = emoteId + 1; }

        /**
         * Set the Emote that is selected.
         *
         * @param emoteId The ID of the emote that is to be assigned.
         */
        void setEmoteSelected(int emoteId)
        { mEmoteSelected = emoteId; }

        /**
         * A flag to check if the Emote is selected.
         */
        bool isEmoteSelected() const
        { return mEmoteSelected != -1; }

        /**
         * Remove a Emote from the shortcut.
         */
        void removeEmote(int index)
        { mEmotes[index] = 0; }

        /**
         * Try to use the Emote specified by the index.
         *
         * @param index Index of the emote shortcut.
         */
        void useEmote(int index);

    private:
        /**
         * Save the configuration information.
         */
        void save();

        int mEmotes[SHORTCUT_EMOTES];  /**< The emote stored. */
        int mEmoteSelected = -1;       /**< The emote held by cursor. */

};

extern EmoteShortcut *emoteShortcut;
