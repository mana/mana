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
 */

#include <guichan/sdl/sdlinput.hpp>

#include "configuration.h"
#include "keyboardconfig.h"
#include "log.h"

#include "gui/setup_keyboard.h"

struct KeyData
{
    const char *configField;
    int defaultValue;
    const char *caption;
};

// keyData must be in same order as enum keyAction.
static KeyData const keyData[KeyboardConfig::KEY_TOTAL] = {
    {"keyMoveUp", SDLK_UP, "Move Up"},
    {"keyMoveDown", SDLK_DOWN, "Move Down"},
    {"keyMoveLeft", SDLK_LEFT, "Move Left"},
    {"keyMoveRight", SDLK_RIGHT, "Move Right"},
    {"keyAttack", SDLK_LCTRL, "Attack"},
    {"keySmilie", SDLK_LALT, "Smilie"},
    {"keyTalk", SDLK_t, "Talk"},
    {"keyTarget", SDLK_LSHIFT, "Stop Attack"},
    {"keyTargetClosest", SDLK_a, "Target Closest"},
    {"keyTargetNPC", SDLK_n, "Target NPC"},
    {"keyTargetPlayer", SDLK_q, "Target Player"},
    {"keyPickup", SDLK_z, "Pickup"},
    {"keyHideWindows", SDLK_h, "Hide Windows"},
    {"keyBeingSit", SDLK_s, "Sit"},
    {"keyShortcut0", SDLK_0, "Item Shortcut 0"},
    {"keyShortcut1", SDLK_1, "Item Shortcut 1"},
    {"keyShortcut2", SDLK_2, "Item Shortcut 2"},
    {"keyShortcut3", SDLK_3, "Item Shortcut 3"},
    {"keyShortcut4", SDLK_4, "Item Shortcut 4"},
    {"keyShortcut5", SDLK_5, "Item Shortcut 5"},
    {"keyShortcut6", SDLK_6, "Item Shortcut 6"},
    {"keyShortcut7", SDLK_7, "Item Shortcut 7"},
    {"keyShortcut8", SDLK_8, "Item Shortcut 8"},
    {"keyShortcut9", SDLK_9, "Item Shortcut 9"},
    {"keyWindowStatus", SDLK_F2, "Status Window"},
    {"keyWindowInventory", SDLK_F3, "Inventory Window"},
    {"keyWindowEquipment", SDLK_F4, "Equipment WIndow"},
    {"keyWindowSkill", SDLK_F5, "Skill Window"},
    {"keyWindowMinimap", SDLK_F6, "Minimap Window"},
    {"keyWindowChat", SDLK_F7, "Chat Window"},
    {"keyWindowShortcut", SDLK_F8, "Item Shortcut Window"},
    {"keyWindowSetup", SDLK_F9, "Setup Window"},
    {"keyWindowDebug", SDLK_F10, "Debug Window"},
    {"keyWindowSmileyList", SDLK_F11, "Smiley List Window"},
    {"keyWindowSmileyBar", SDLK_F12, "Smiley Shortcut Window"},
    {"keySmileyShortcut1", SDLK_KP1, "Smiley Shortcut 1"},
    {"keySmileyShortcut2", SDLK_KP2, "Smiley Shortcut 2"},
    {"keySmileyShortcut3", SDLK_KP3, "Smiley Shortcut 3"},
    {"keySmileyShortcut4", SDLK_KP4, "Smiley Shortcut 4"},
    {"keySmileyShortcut5", SDLK_KP5, "Smiley Shortcut 5"},
    {"keySmileyShortcut6", SDLK_KP6, "Smiley Shortcut 6"},
    {"keySmileyShortcut7", SDLK_KP7, "Smiley Shortcut 7"},
    {"keySmileyShortcut8", SDLK_KP8, "Smiley Shortcut 8"},
    {"keySmileyShortcut9", SDLK_KP9, "Smiley Shortcut 9"},
    {"keySmileyShortcut10", SDLK_KP0, "Smiley Shortcut 10"},
    {"keySmileyShortcut11", SDLK_KP_DIVIDE, "Smiley Shortcut 11"},
    {"keySmileyShortcut12", SDLK_KP_MULTIPLY, "Smiley Shortcut 12"}
};

void KeyboardConfig::init()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        mKey[i].configField = keyData[i].configField;
        mKey[i].defaultValue = keyData[i].defaultValue;
        mKey[i].caption = keyData[i].caption;
        mKey[i].value = KEY_NO_VALUE;
    }
    mNewKeyIndex = KEY_NO_VALUE;
    mEnabled = true;

    retrieve();
}

void KeyboardConfig::retrieve()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        mKey[i].value = (int) config.getValue(
            mKey[i].configField, mKey[i].defaultValue);
    }
}

void KeyboardConfig::store()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        config.setValue(mKey[i].configField, mKey[i].value);
    }
}

void KeyboardConfig::makeDefault()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        mKey[i].value = mKey[i].defaultValue;
    }
}

bool KeyboardConfig::hasConflicts()
{
    int i, j;
/**
 * No need to parse the square matrix: only check one triangle
 * that's enough to detect conflicts
 */
    for (i = 0; i < KEY_TOTAL; i++)
    {
        for (j = i,j++; j < KEY_TOTAL; j++)
        {
/**
 * KEY_SMILEY_* are separated from other keys, duplicate in different
 *  area is allowed, but not in same area (of course)
 *  (i.e.: not two identical key for smiley, not two identical for other;
 *  but same key for a smiley and a not-smiley is ok)
 *  
 */
            if (!((i<KEY_SMILEY_1)&&(j>=KEY_SMILEY_1))
                 && mKey[i].value == mKey[j].value
	       )
            {
                return true;
            }
        }
    }
    return false;
}

void KeyboardConfig::callbackNewKey()
{
    mSetupKey->newKeyCallback(mNewKeyIndex);
}

int KeyboardConfig::getKeyIndex(int keyValue) const
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        if(keyValue == mKey[i].value)
        {
            return i;
        }
    }
    return KEY_NO_VALUE;
}

int KeyboardConfig::getKeySmilieOffset(int keyValue) const
{
    for (int i = KEY_SMILEY_1; i <= KEY_SMILEY_12; i++)
    {
        if(keyValue == mKey[i].value)
        {
            return 1+i-KEY_SMILEY_1;
        }
    }
    return 0;
}

bool KeyboardConfig::isKeyActive(int index)
{
    return mActiveKeys[ mKey[index].value];
}

void KeyboardConfig::refreshActiveKeys()
{
    mActiveKeys = SDL_GetKeyState(NULL);
}
