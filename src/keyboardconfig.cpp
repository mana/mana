/*
 *  Custom keyboard shortcuts configuration
 *  Copyright (C) 2007  Joshua Langley <joshlangley@optusnet.com.au>
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
    {"keyShortcut1", SDLK_1, "Item Shortcut 1"},
    {"keyShortcut2", SDLK_2, "Item Shortcut 2"},
    {"keyShortcut3", SDLK_3, "Item Shortcut 3"},
    {"keyShortcut4", SDLK_4, "Item Shortcut 4"},
    {"keyShortcut5", SDLK_5, "Item Shortcut 5"},
    {"keyShortcut6", SDLK_6, "Item Shortcut 6"},
    {"keyShortcut7", SDLK_7, "Item Shortcut 7"},
    {"keyShortcut8", SDLK_8, "Item Shortcut 8"},
    {"keyShortcut9", SDLK_9, "Item Shortcut 9"},
    {"keyShortcut10", SDLK_0, "Item Shortcut 10"},
    {"keyShortcut11", SDLK_MINUS, "Item Shortcut 11"},
    {"keyShortcut12", SDLK_EQUALS, "Item Shortcut 12"},
    {"keyWindowStatus", SDLK_F2, "Status Window"},
    {"keyWindowInventory", SDLK_F3, "Inventory Window"},
    {"keyWindowEquipment", SDLK_F4, "Equipment WIndow"},
    {"keyWindowSkill", SDLK_F5, "Skill Window"},
    {"keyWindowMinimap", SDLK_F6, "Minimap Window"},
    {"keyWindowChat", SDLK_F7, "Chat Window"},
    {"keyWindowShortcut", SDLK_F8, "Item Shortcut Window"},
    {"keyWindowSetup", SDLK_F9, "Setup Window"},
    {"keyWindowDebug", SDLK_F10, "Debug Window"},
    {"keyWindowEmote", SDLK_F11, "Emote Window"},
    {"keyWindowEmoteBar", SDLK_F12, "Emote Shortcut Window"},
    {"keyEmoteShortcut1", SDLK_1, "Emote Shortcut 1"},
    {"keyEmoteShortcut2", SDLK_2, "Emote Shortcut 2"},
    {"keyEmoteShortcut3", SDLK_3, "Emote Shortcut 3"},
    {"keyEmoteShortcut4", SDLK_4, "Emote Shortcut 4"},
    {"keyEmoteShortcut5", SDLK_5, "Emote Shortcut 5"},
    {"keyEmoteShortcut6", SDLK_6, "Emote Shortcut 6"},
    {"keyEmoteShortcut7", SDLK_7, "Emote Shortcut 7"},
    {"keyEmoteShortcut8", SDLK_8, "Emote Shortcut 8"},
    {"keyEmoteShortcut9", SDLK_9, "Emote Shortcut 9"},
    {"keyEmoteShortcut10", SDLK_0, "Emote Shortcut 10"},
    {"keyEmoteShortcut11", SDLK_MINUS, "Emote Shortcut 11"},
    {"keyEmoteShortcut12", SDLK_EQUALS, "Emote Shortcut 12"}
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
        for (j = i, j++; j < KEY_TOTAL; j++)
        {
            // Allow for item shortcut and emote keys to overlap, but no other keys
            if (!(((i >= KEY_SHORTCUT_1) && (i <= KEY_SHORTCUT_12)) && 
                  ((j >= KEY_EMOTE_1) && (j <= KEY_EMOTE_12)))
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


int KeyboardConfig::getKeyEmoteOffset(int keyValue) const
{
    for (int i = KEY_EMOTE_1; i <= KEY_EMOTE_12; i++)
    {
        if(keyValue == mKey[i].value)
        {
            return 1 + i - KEY_EMOTE_1;
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
