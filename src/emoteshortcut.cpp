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

#include "emoteshortcut.h"

#include "configuration.h"
#include "localplayer.h"

#include "utils/stringutils.h"

EmoteShortcut *emoteShortcut;

EmoteShortcut::EmoteShortcut()
{
    load();
}

EmoteShortcut::~EmoteShortcut()
{
    save();
}

void EmoteShortcut::load()
{
    for (int i = 0; i < SHORTCUT_EMOTES; i++)
    {
        mEmotes[i] = (int) config.getValue("emoteshortcut" + toString(i), i + 1);
    }
}

void EmoteShortcut::save()
{
    for (int i = 0; i < SHORTCUT_EMOTES; i++)
    {
        config.setValue("emoteshortcut" + toString(i), mEmotes[i]);
    }
}

void EmoteShortcut::useEmote(int index)
{
    if (index >= 0 && index < SHORTCUT_EMOTES)
    {
        if (mEmotes[index] > 0)
        {
            local_player->emote(mEmotes[index] - 1);
        }
    }
}
