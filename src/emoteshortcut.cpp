/*
 *  Extended support for activating emotes
 *  Copyright (C) 2009  Aethyra Development Team
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

#include "emoteshortcut.h"

#include "configuration.h"
#include "localplayer.h"

#include "utils/stringutils.h"

EmoteShortcut::EmoteShortcut *emoteShortcut;

EmoteShortcut::EmoteShortcut():
    mEmoteSelected(0)
{
    for (int i = 0; i < SHORTCUT_EMOTES; i++)
    {
        mEmotes[i] = i + 1;
    }
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
        int emoteId = (int) config.getValue("emoteshortcut" + toString(i), i + 1);

        if (emoteId)
        {
            mEmotes[i] = emoteId;
        }
    }
}

void EmoteShortcut::save()
{
    for (int i = 0; i < SHORTCUT_EMOTES; i++)
    {
        const int emoteId = mEmotes[i] ? mEmotes[i] : 0;
        config.setValue("emoteshortcut" + toString(i), emoteId);
    }
}

void EmoteShortcut::useEmote(int index)
{
    if ((index > 0) && (index <= SHORTCUT_EMOTES))
    {
       if (mEmotes[index - 1] > 0)
       {
          player_node->emote(mEmotes[index - 1]);
       }
    }
}
