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

#include "configuration.h"
#include "smileyshortcut.h"
#include "localplayer.h"

#include "utils/tostring.h"

SmileyShortcut::SmileyShortcut *smileyShortcut;

SmileyShortcut::SmileyShortcut():
    mSmileySelected(0)
{
    for (int i = 0; i < SHORTCUT_SMILEYS; i++)
    {
        mSmileys[i] = i+1;
    }
    load();
}

SmileyShortcut::~SmileyShortcut()
{
    save();
}

void SmileyShortcut::load()
{
    for (int i = 0; i < SHORTCUT_SMILEYS; i++)
    {
        int smileyId = (int) config.getValue("Smileyshortcut" + toString(i), i+1);

        if (smileyId)
        {
            mSmileys[i] = smileyId;
        }
    }
}

void SmileyShortcut::save()
{
    for (int i = 0; i < SHORTCUT_SMILEYS; i++)
    {
        const int smileyId = mSmileys[i] ? mSmileys[i] : 0;
        config.setValue("Smileyshortcut" + toString(i), smileyId);
    }
}

void SmileyShortcut::useSmiley(int index)
{
    if ((index>0)&&(index<=SHORTCUT_SMILEYS))
    {
       if (mSmileys[index-1]>0)
       {
          player_node->emote(mSmileys[index-1]);
       }
    }
}
