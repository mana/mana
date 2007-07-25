/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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

#ifndef _TMW_KEYBOARDCONFIG_H
#define _TMW_KEYBOARDCONFIG_H

#include <string>

#include "gui/setup_keyboard.h"

#include <guichan/sdl/sdlinput.hpp>

/**
 * Each key represents a key function. Such as 'Move up', 'Attack' etc.
 */
struct KeyFunction
{
    KeyFunction() {}

    KeyFunction(std::string configField,
                int defaultValue,
                std::string caption):
        configField(configField),
        caption(caption),
        defaultValue(defaultValue)
    {}

    std::string configField;    /** Field index that is in the config file. */
    std::string caption;        /** The caption value for the key function. */
    int defaultValue;           /** The default key value used. */
    int value;                  /** The actual value that is used. */
};

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
        std::string const &getKeyCaption(int index) const
        { return mKey[index].caption; }

        /**
         * Get the key function index by providing the keys value.
         */
        int getKeyIndex(int keyValue) const;

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
        bool isKeyActive(int index);

        /**
         * Takes a snapshot of all the active keys.
         */
        void refreshActiveKeys();

        /**
         * All the key functions.
         * KEY_NO_VALUE is used in initialization, and should be unchanged.
         * KEY_MIN and KEY_TOTAL should always be first and last respectively,
         * the bare used in control loops.
         * The third element (after KEY_NO_VALUE and KEY_MIN),
         * should always equal KEY_MIN.
         * The key assignment view gets arranged according to the order of
         * these values.
         */
        enum KeyAction {
            KEY_NO_VALUE = -1,
            KEY_MOVE_UP,
            KEY_MOVE_DOWN,
            KEY_MOVE_LEFT,
            KEY_MOVE_RIGHT,
            KEY_ATTACK,
            KEY_TARGET,
            KEY_TARGET_CLOSEST,
            KEY_PICKUP,
            KEY_SIT,
            KEY_HIDE_WINDOWS,
            KEY_TOTAL
        };

    private:
        int mNewKeyIndex;              /**< Index of new key to be assigned */
        bool mEnabled;                 /**< Flag to respond to key input */

        Setup_Keyboard *mSetupKey;     /**< Reference to setup window */

        KeyFunction mKey[KEY_TOTAL];   /**< Pointer to all the key data */

        Uint8 *mActiveKeys;            /**< Stores a list of all the keys */
};

extern KeyboardConfig keyboard;

#endif
