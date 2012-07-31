/*
 *  Custom keyboard shortcuts configuration
 *  Copyright (C) 2007  Joshua Langley <joshlangley@optusnet.com.au>
 *  Copyright (C) 2008-2009  The Mana Development Team
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

#ifndef KEYBOARDCONFIG_H
#define KEYBOARDCONFIG_H

#include <cstdint>
#include <string>

/**
 * Each key represents a key function. Such as 'Move up', 'Attack' etc.
 */
struct KeyFunction
{
    const char* configField;    /** Field index that is in the config file. */
    int defaultValue;           /** The default key value used. */
    std::string caption;        /** The caption value for the key function. */
    int value;                  /** The actual value that is used. */
};

class Setup_Keyboard;

class KeyboardConfig
{
    public:
        /**
         * Initializes the keyboard config explicitly.
         */
        void init();

        /**
         * Retrieve the key values from config file.
         */
        void retrieve();

        /**
         * Store the key values to config file.
         */
        void store();

        /**
         * Make the keys their default values.
         */
        void makeDefault();

        /**
         * Determines if any key assignments are the same as each other.
         */
        bool hasConflicts();

        /**
         * Calls a function back so the key re-assignment(s) can be seen.
         */
        void callbackNewKey();

        /**
         * Obtain the value stored in memory.
         */
        int getKeyValue(int index) const
        { return mKey[index].value; }

        /**
         * Get the index of the new key to be assigned.
         */
        int getNewKeyIndex() const
        { return mNewKeyIndex; }

        /**
         * Get the enable flag, which will stop the user from doing actions.
         */
        bool isEnabled() const
        { return mEnabled; }

        /**
         * Get the key caption, providing more meaning to the user.
         */
        const std::string &getKeyCaption(int index) const
        { return mKey[index].caption; }

        /**
         * Get the key function index by providing the keys value.
         */
        int getKeyIndex(int keyValue) const;

        /**
         * Get the key function index for an emote by providing the offset value.
         */
        int getKeyEmoteOffset(int keyValue) const;

        /**
         * Set the enable flag, which will stop the user from doing actions.
         */
        void setEnabled(bool flag)
        { mEnabled = flag; }

        /**
         * Set the index of the new key to be assigned.
         */
        void setNewKeyIndex(int value)
        { mNewKeyIndex = value; }

        /**
         * Set the value of the new key.
         */
        void setNewKey(int value)
        { mKey[mNewKeyIndex].value = value; }

        /**
         * Set a reference to the key setup window.
         */
        void setSetupKeyboard(Setup_Keyboard *setupKey)
        { mSetupKey = setupKey; }

        /**
         * Checks if the key is active, by providing the key function index.
         */
        bool isKeyActive(int index) const;

        /**
         * Takes a snapshot of all the active keys.
         */
        void refreshActiveKeys();

        std::string getBindError() const
        { return mBindError; }

        /**
         * All the key functions.
         * KEY_NO_VALUE is used in initialization, and should be unchanged.
         * KEY_TOTAL should always be last (used as a conditional in loops).
         * The key assignment view gets arranged according to the order of
         * these values.
         */
        enum KeyAction
        {
            KEY_NO_VALUE = -1,
            KEY_MOVE_UP,
            KEY_MOVE_DOWN,
            KEY_MOVE_LEFT,
            KEY_MOVE_RIGHT,
            KEY_ATTACK,
            KEY_TARGET_ATTACK,
            KEY_EMOTE,
            KEY_TALK,
            KEY_TARGET,
            KEY_TARGET_CLOSEST,
            KEY_TARGET_NPC,
            KEY_TARGET_PLAYER,
            KEY_PICKUP,
            KEY_HIDE_WINDOWS,
            KEY_SIT,
            KEY_SCREENSHOT,
            KEY_TRADE,
            KEY_SHORTCUT_1,
            KEY_SHORTCUT_2,
            KEY_SHORTCUT_3,
            KEY_SHORTCUT_4,
            KEY_SHORTCUT_5,
            KEY_SHORTCUT_6,
            KEY_SHORTCUT_7,
            KEY_SHORTCUT_8,
            KEY_SHORTCUT_9,
            KEY_SHORTCUT_10,
            KEY_SHORTCUT_11,
            KEY_SHORTCUT_12,
            KEY_WINDOW_HELP,
            KEY_WINDOW_STATUS,
            KEY_WINDOW_INVENTORY,
            KEY_WINDOW_EQUIPMENT,
            KEY_WINDOW_SKILL,
            KEY_WINDOW_MINIMAP,
            KEY_WINDOW_CHAT,
            KEY_WINDOW_SHORTCUT,
            KEY_WINDOW_SETUP,
            KEY_WINDOW_DEBUG,
            KEY_WINDOW_SOCIAL,
            KEY_WINDOW_OUTFIT,
            KEY_WEAR_OUTFIT,
            KEY_COPY_OUTFIT,
            KEY_EMOTE_1,
            KEY_EMOTE_2,
            KEY_EMOTE_3,
            KEY_EMOTE_4,
            KEY_EMOTE_5,
            KEY_EMOTE_6,
            KEY_EMOTE_7,
            KEY_EMOTE_8,
            KEY_EMOTE_9,
            KEY_EMOTE_10,
            KEY_EMOTE_11,
            KEY_EMOTE_12,
            KEY_TOGGLE_CHAT,
            KEY_SCROLL_CHAT_UP,
            KEY_SCROLL_CHAT_DOWN,
            KEY_PREV_CHAT_TAB,
            KEY_NEXT_CHAT_TAB,
            KEY_OK,
            KEY_QUIT,
            KEY_IGNORE_INPUT_1,
            KEY_IGNORE_INPUT_2,
            KEY_TOTAL
        };

    private:
        int mNewKeyIndex;              /**< Index of new key to be assigned */
        bool mEnabled;                 /**< Flag to respond to key input */

        Setup_Keyboard *mSetupKey;     /**< Reference to setup window */

        KeyFunction mKey[KEY_TOTAL];   /**< Pointer to all the key data */

        uint8_t *mActiveKeys;          /**< Stores a list of all the keys */

        std::string mBindError;
};

extern KeyboardConfig keyboard;

#endif
