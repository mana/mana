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

#include "configuration.h"
#include "keyboardconfig.h"
#include "log.h"

#include "gui/sdlinput.h"
#include "gui/setup_keyboard.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

struct KeyData
{
    const char *configField;
    int defaultValue;
    std::string caption;
};

// keyData must be in same order as enum keyAction.
static KeyData const keyData[KeyboardConfig::KEY_TOTAL] = {
    {"keyMoveUp", SDLK_UP, _("Move Up")},
    {"keyMoveDown", SDLK_DOWN, _("Move Down")},
    {"keyMoveLeft", SDLK_LEFT, _("Move Left")},
    {"keyMoveRight", SDLK_RIGHT, _("Move Right")},
    {"keyAttack", SDLK_LCTRL, _("Attack")},
    {"keySmilie", SDLK_LALT, _("Smilie")},
    {"keyTalk", SDLK_t, _("Talk")},
    {"keyTarget", SDLK_LSHIFT, _("Stop Attack")},
    {"keyTargetClosest", SDLK_a, _("Target Closest")},
    {"keyTargetNPC", SDLK_n, _("Target NPC")},
    {"keyTargetPlayer", SDLK_q, _("Target Player")},
    {"keyPickup", SDLK_z, _("Pickup")},
    {"keyHideWindows", SDLK_h, _("Hide Windows")},
    {"keyBeingSit", SDLK_s, _("Sit")},
    {"keyScreenshot", SDLK_p, _("Screenshot")},
    {"keyTrade", SDLK_r, _("Enable/Disable Trading")},
    {"keyPathfind", SDLK_f, _("Find Path to Mouse")},
    {"keyShortcut1", SDLK_1, strprintf(_("Item Shortcut %d"), 1)},
    {"keyShortcut2", SDLK_2, strprintf(_("Item Shortcut %d"), 2)},
    {"keyShortcut3", SDLK_3, strprintf(_("Item Shortcut %d"), 3)},
    {"keyShortcut4", SDLK_4, strprintf(_("Item Shortcut %d"), 4)},
    {"keyShortcut5", SDLK_5, strprintf(_("Item Shortcut %d"), 5)},
    {"keyShortcut6", SDLK_6, strprintf(_("Item Shortcut %d"), 6)},
    {"keyShortcut7", SDLK_7, strprintf(_("Item Shortcut %d"), 7)},
    {"keyShortcut8", SDLK_8, strprintf(_("Item Shortcut %d"), 8)},
    {"keyShortcut9", SDLK_9, strprintf(_("Item Shortcut %d"), 9)},
    {"keyShortcut10", SDLK_0, strprintf(_("Item Shortcut %d"), 10)},
    {"keyShortcut11", SDLK_MINUS, strprintf(_("Item Shortcut %d"), 11)},
    {"keyShortcut12", SDLK_EQUALS, strprintf(_("Item Shortcut %d"), 12)},
    {"keyWindowHelp", SDLK_F1, _("Help Window")},
    {"keyWindowStatus", SDLK_F2, _("Status Window")},
    {"keyWindowInventory", SDLK_F3, _("Inventory Window")},
    {"keyWindowEquipment", SDLK_F4, _("Equipment WIndow")},
    {"keyWindowSkill", SDLK_F5, _("Skill Window")},
    {"keyWindowMinimap", SDLK_F6, _("Minimap Window")},
    {"keyWindowChat", SDLK_F7, _("Chat Window")},
    {"keyWindowShortcut", SDLK_F8, _("Item Shortcut Window")},
    {"keyWindowSetup", SDLK_F9, _("Setup Window")},
    {"keyWindowDebug", SDLK_F10, _("Debug Window")},
    {"keyWindowParty", SDLK_F11, _("Party Window")},
    {"keyWindowEmoteBar", SDLK_F12, _("Emote Shortcut Window")},
    {"keyEmoteShortcut1", SDLK_1, strprintf(_("Emote Shortcut %d"), 1)},
    {"keyEmoteShortcut2", SDLK_2, strprintf(_("Emote Shortcut %d"), 2)},
    {"keyEmoteShortcut3", SDLK_3, strprintf(_("Emote Shortcut %d"), 3)},
    {"keyEmoteShortcut4", SDLK_4, strprintf(_("Emote Shortcut %d"), 4)},
    {"keyEmoteShortcut5", SDLK_5, strprintf(_("Emote Shortcut %d"), 5)},
    {"keyEmoteShortcut6", SDLK_6, strprintf(_("Emote Shortcut %d"), 6)},
    {"keyEmoteShortcut7", SDLK_7, strprintf(_("Emote Shortcut %d"), 7)},
    {"keyEmoteShortcut8", SDLK_8, strprintf(_("Emote Shortcut %d"), 8)},
    {"keyEmoteShortcut9", SDLK_9, strprintf(_("Emote Shortcut %d"), 9)},
    {"keyEmoteShortcut10", SDLK_0, strprintf(_("Emote Shortcut %d"), 10)},
    {"keyEmoteShortcut11", SDLK_MINUS, strprintf(_("Emote Shortcut %d"), 11)},
    {"keyEmoteShortcut12", SDLK_EQUALS, strprintf(_("Emote Shortcut %d"), 12)},
    {"keyChat", SDLK_RETURN, _("Toggle Chat")},
    {"keyChatScrollUp", SDLK_PAGEUP, _("Scroll Chat Up")},
    {"keyChatScrollDown", SDLK_PAGEDOWN, _("Scroll Chat Down")},
    {"keyChatPrevTab", SDLK_LEFTBRACKET, _("Previous Chat Tab")},
    {"keyChatNextTab", SDLK_RIGHTBRACKET, _("Next Chat Tab")},
    {"keyOK", SDLK_RETURN, _("Select OK")},
    {"keyQuit", SDLK_ESCAPE, _("Quit")},
    {"keyIgnoreInput1", SDLK_LSUPER, _("Ignore input 1")},
    {"keyIgnoreInput2", SDLK_RSUPER, _("Ignore input 2")}
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
            // Allow for item shortcut and emote keys to overlap
            // as well as emote and ignore keys, but no other keys
            if (!((((i >= KEY_SHORTCUT_1) && (i <= KEY_SHORTCUT_12)) &&
                   ((j >= KEY_EMOTE_1) && (j <= KEY_EMOTE_12))) ||
                   ((i == KEY_TOGGLE_CHAT) && (j == KEY_OK)) ||
                   ((i == KEY_EMOTE) &&
                    (j == KEY_IGNORE_INPUT_1 || j == KEY_IGNORE_INPUT_2))) &&
                   (mKey[i].value == mKey[j].value)
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
        if (keyValue == mKey[i].value)
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
        if (keyValue == mKey[i].value)
        {
            return 1 + i - KEY_EMOTE_1;
        }
    }
    return 0;
}

bool KeyboardConfig::isKeyActive(int index) const
{
    if (!mActiveKeys)
        return false;
    return mActiveKeys[mKey[index].value];
}

void KeyboardConfig::refreshActiveKeys()
{
    mActiveKeys = SDL_GetKeyState(NULL);
}
