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

#include "configuration.h"
#include "keyboardconfig.h"

#include "gui/setup_keyboard.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

struct KeyData
{
    const char *configField;
    SDL_Keycode defaultValue;
    std::string caption;
};

// keyData must be in same order as enum keyAction.
static KeyData const keyData[KeyboardConfig::KEY_TOTAL] = {
    { "MoveUp",             SDLK_UP,            _("Move Up") },
    { "MoveDown",           SDLK_DOWN,          _("Move Down") },
    { "MoveLeft",           SDLK_LEFT,          _("Move Left") },
    { "MoveRight",          SDLK_RIGHT,         _("Move Right") },
    { "Attack",             SDLK_LCTRL,         _("Attack") },
    { "TargetAttack",       SDLK_x,             _("Target & Attack") },
    { "Smilie",             SDLK_LALT,          _("Smilie") },
    { "Talk",               SDLK_t,             _("Talk") },
    { "Target",             SDLK_LSHIFT,        _("Stop Attack") },
    { "TargetMonster",      SDLK_a,             _("Target Monster") },
    { "TargetNPC",          SDLK_n,             _("Target NPC") },
    { "TargetPlayer",       SDLK_q,             _("Target Player") },
    { "Pickup",             SDLK_z,             _("Pickup") },
    { "HideWindows",        SDLK_h,             _("Hide Windows") },
    { "BeingSit",           SDLK_s,             _("Sit") },
    { "Screenshot",         SDLK_p,             _("Screenshot") },
    { "Trade",              SDLK_r,             _("Enable/Disable Trading") },
    { "Shortcut1",          SDLK_1,             strprintf(_("Item Shortcut %d"), 1) },
    { "Shortcut2",          SDLK_2,             strprintf(_("Item Shortcut %d"), 2) },
    { "Shortcut3",          SDLK_3,             strprintf(_("Item Shortcut %d"), 3) },
    { "Shortcut4",          SDLK_4,             strprintf(_("Item Shortcut %d"), 4) },
    { "Shortcut5",          SDLK_5,             strprintf(_("Item Shortcut %d"), 5) },
    { "Shortcut6",          SDLK_6,             strprintf(_("Item Shortcut %d"), 6) },
    { "Shortcut7",          SDLK_7,             strprintf(_("Item Shortcut %d"), 7) },
    { "Shortcut8",          SDLK_8,             strprintf(_("Item Shortcut %d"), 8) },
    { "Shortcut9",          SDLK_9,             strprintf(_("Item Shortcut %d"), 9) },
    { "Shortcut10",         SDLK_0,             strprintf(_("Item Shortcut %d"), 10) },
    { "Shortcut11",         SDLK_MINUS,         strprintf(_("Item Shortcut %d"), 11) },
    { "Shortcut12",         SDLK_EQUALS,        strprintf(_("Item Shortcut %d"), 12) },
    { "WindowHelp",         SDLK_F1,            _("Help Window") },
    { "WindowStatus",       SDLK_F2,            _("Status Window") },
    { "WindowInventory",    SDLK_F3,            _("Inventory Window") },
    { "WindowEquipment",    SDLK_F4,            _("Equipment Window") },
    { "WindowSkill",        SDLK_F5,            _("Skill Window") },
    { "WindowQuests",       KeyboardConfig::KEY_NO_VALUE, _("Quest Window") },
    { "WindowMinimap",      SDLK_F6,            _("Minimap Window") },
    { "WindowChat",         SDLK_F7,            _("Chat Window") },
    { "WindowShortcut",     SDLK_F8,            _("Item Shortcut Window") },
    { "WindowSetup",        SDLK_F9,            _("Setup Window") },
    { "WindowDebug",        SDLK_F10,           _("Debug Window") },
    { "WindowSocial",       SDLK_F11,           _("Social Window") },
    { "WindowEmoteBar",     SDLK_F12,           _("Emote Shortcut Window") },
    { "WindowOutfit",       SDLK_o,             _("Outfits Window") },
    { "WearOutfit",         SDLK_RCTRL,         _("Wear Outfit") },
    { "CopyOutfit",         SDLK_RALT,          _("Copy Outfit") },
    { "EmoteShortcut1",     SDLK_1,             strprintf(_("Emote Shortcut %d"), 1) },
    { "EmoteShortcut2",     SDLK_2,             strprintf(_("Emote Shortcut %d"), 2) },
    { "EmoteShortcut3",     SDLK_3,             strprintf(_("Emote Shortcut %d"), 3) },
    { "EmoteShortcut4",     SDLK_4,             strprintf(_("Emote Shortcut %d"), 4) },
    { "EmoteShortcut5",     SDLK_5,             strprintf(_("Emote Shortcut %d"), 5) },
    { "EmoteShortcut6",     SDLK_6,             strprintf(_("Emote Shortcut %d"), 6) },
    { "EmoteShortcut7",     SDLK_7,             strprintf(_("Emote Shortcut %d"), 7) },
    { "EmoteShortcut8",     SDLK_8,             strprintf(_("Emote Shortcut %d"), 8) },
    { "EmoteShortcut9",     SDLK_9,             strprintf(_("Emote Shortcut %d"), 9) },
    { "EmoteShortcut10",    SDLK_0,             strprintf(_("Emote Shortcut %d"), 10) },
    { "EmoteShortcut11",    SDLK_MINUS,         strprintf(_("Emote Shortcut %d"), 11) },
    { "EmoteShortcut12",    SDLK_EQUALS,        strprintf(_("Emote Shortcut %d"), 12) },
    { "Chat",               SDLK_RETURN,        _("Toggle Chat") },
    { "ChatScrollUp",       SDLK_PAGEUP,        _("Scroll Chat Up") },
    { "ChatScrollDown",     SDLK_PAGEDOWN,      _("Scroll Chat Down") },
    { "ChatPrevTab",        SDLK_LEFTBRACKET,   _("Previous Chat Tab") },
    { "ChatNextTab",        SDLK_RIGHTBRACKET,  _("Next Chat Tab") },
    { "OK",                 SDLK_SPACE,         _("Select OK") },
    { "Quit",               SDLK_ESCAPE,        _("Quit") },
    { "IgnoreInput1",       SDLK_LGUI,          _("Ignore input 1") },
    { "IgnoreInput2",       SDLK_RGUI,          _("Ignore input 2") }
};

void KeyboardConfig::init()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        mKey[i].configField = keyData[i].configField;
        mKey[i].defaultValue = keyData[i].defaultValue;
        mKey[i].value = KEY_NO_VALUE;
    }
    mNewKeyIndex = KEY_NO_VALUE;
    mEnabled = true;

    retrieve();
}

void KeyboardConfig::retrieve()
{
    for (auto &key : mKey)
    {
        const auto keyIt = config.keys.find(key.configField);
        if (keyIt != config.keys.end())
        {
            key.value = SDL_GetKeyFromName(keyIt->second.c_str());

            if (key.value == SDLK_UNKNOWN)
                key.value = KEY_NO_VALUE;
        }
        else
        {
            key.value = key.defaultValue;
        }
    }
}

void KeyboardConfig::store()
{
    config.keys.clear();

    for (auto &key : mKey)
    {
        if (key.value == key.defaultValue)
            continue;

        config.keys[key.configField] = SDL_GetKeyName(key.value);
    }
}

void KeyboardConfig::makeDefault()
{
    for (auto &key : mKey)
        key.value = key.defaultValue;
}

bool KeyboardConfig::hasConflicts()
{
    /**
     * No need to parse the square matrix: only check one triangle
     * that's enough to detect conflicts
     */
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        if (mKey[i].value == KEY_NO_VALUE)
            continue;

        for (int j = i + 1; j < KEY_TOTAL; j++)
        {
            if (mKey[j].value == KEY_NO_VALUE)
                continue;

            // Allow collisions between shortcut and emote keys
            if ((i >= KEY_SHORTCUT_1 && i <= KEY_SHORTCUT_12) &&
                    (j >= KEY_EMOTE_1 && j <= KEY_EMOTE_12))
                continue;

            // Why?
            if (i == KEY_TOGGLE_CHAT && j == KEY_OK)
                continue;

            // Ignore keys can collide with anything.
            if (j == KEY_IGNORE_INPUT_1 || j == KEY_IGNORE_INPUT_2)
                continue;

            // If the one of the keys is not set, then no conflict can happen.
            if (mKey[i].value == -1 || mKey[j].value == -1)
                continue;

            // Finally test to see if a conflict DOES exist.
            if (mKey[i].value == mKey[j].value)
            {
                mBindError = strprintf(_("Conflict \"%s\" and \"%s\" keys. "
                    "Resolve them, or gameplay may result"
                    " in strange behaviour."),
                    keyData[i].caption.c_str(), keyData[j].caption.c_str());
                return true;
            }
        }
    }
    mBindError.clear();
    return false;
}

void KeyboardConfig::callbackNewKey()
{
    mSetupKey->newKeyCallback(mNewKeyIndex);
}

const std::string &KeyboardConfig::getKeyCaption(int index) const
{
    return keyData[index].caption;
}

int KeyboardConfig::getKeyIndex(SDL_Keycode keyValue) const
{
    for (int i = 0; i < KEY_TOTAL; i++)
        if (keyValue == mKey[i].value)
            return i;

    return KEY_NO_VALUE;
}

std::string_view KeyboardConfig::getKeyName(std::string_view configName) const
{
    for (auto key : mKey)
    {
        if (configName == key.configField)
        {
            if (key.value == KEY_NO_VALUE)
                return {};
            return SDL_GetKeyName(key.value);
        }
    }

    return {};
}

int KeyboardConfig::getKeyEmoteOffset(SDL_Keycode keyValue) const
{
    for (int i = KEY_EMOTE_1; i <= KEY_EMOTE_12; i++)
        if (keyValue == mKey[i].value)
            return i - KEY_EMOTE_1;

    return -1;
}

bool KeyboardConfig::isKeyActive(int index) const
{
    if (!mActiveKeys)
        return false;
    int scanCode = SDL_GetScancodeFromKey(mKey[index].value);
    return mActiveKeys[scanCode];
}

void KeyboardConfig::refreshActiveKeys()
{
    mActiveKeys = SDL_GetKeyboardState(nullptr);
}
