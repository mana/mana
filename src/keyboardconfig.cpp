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
 */

#include "keyboardconfig.h"
#include "configuration.h"
#include "log.h"

#include <guichan/sdl/sdlinput.hpp>

#include "gui/setup_keyboard.h"

void KeyboardConfig::init()
{
    mKey[KEY_MOVE_UP] = KeyFunction("keyMoveUp", SDLK_UP, "Move Up");
    mKey[KEY_MOVE_DOWN] = KeyFunction("keyMoveDown", SDLK_DOWN, "Move Down");
    mKey[KEY_MOVE_LEFT] = KeyFunction("keyMoveLeft", SDLK_LEFT, "Move Left");
    mKey[KEY_MOVE_RIGHT] =
        KeyFunction("keyMoveRight", SDLK_RIGHT, "Move Right");

    mKey[KEY_ATTACK] = KeyFunction("keyAttack", SDLK_LCTRL, "Attack");
    mKey[KEY_TARGET] = KeyFunction("keyTarget", SDLK_LSHIFT, "Target");
    mKey[KEY_TARGET_CLOSEST] =
        KeyFunction("keyTargetClosest", SDLK_a, "Target Closest");
    mKey[KEY_PICKUP] = KeyFunction("keyPickup", SDLK_z, "Pickup");
    mKey[KEY_HIDE_WINDOWS] =
        KeyFunction("keyHideWindows", SDLK_h, "Hide Windows");
    mKey[KEY_SIT] = KeyFunction("keyBeingSit", SDLK_g, "Sit");

    for (int i = 0; i < KEY_TOTAL; i++)
    {
        mKey[i].value = KEY_NO_VALUE;
    }
    mNewKeyIndex = KEY_NO_VALUE;
    mEnabled = true;

    retrieve();
}

void KeyboardConfig::destroy()
{
    store();

    delete [] mActiveKeys;
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
    for (i = 0; i < KEY_TOTAL; i++)
    {
        for (j = 0; j < KEY_TOTAL; j++)
        {
            if (i != j && mKey[i].value == mKey[j].value)
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

int KeyboardConfig::getKeyIndex(const int keyValue) const
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

bool KeyboardConfig::isKeyActive(int index)
{
    return mActiveKeys[ mKey[index].value];
}

void KeyboardConfig::refreshActiveKeys()
{
    mActiveKeys = SDL_GetKeyState(NULL);
}
