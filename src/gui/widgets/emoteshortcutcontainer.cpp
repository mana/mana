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

#include "gui/widgets/emoteshortcutcontainer.h"

#include "emoteshortcut.h"
#include "graphics.h"
#include "keyboardconfig.h"

#include "gui/gui.h"

#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/theme.h"

static const int MAX_ITEMS = 12;

EmoteShortcutContainer::EmoteShortcutContainer()
{
    mMaxItems = std::min(EmoteDB::getEmoteCount(), MAX_ITEMS);
}

void EmoteShortcutContainer::draw(gcn::Graphics *graphics)
{
    auto *g = static_cast<Graphics*>(graphics);
    auto theme = gui->getTheme();

    graphics->setFont(getFont());

    for (int i = 0; i < mMaxItems; i++)
    {
        WidgetState state;
        state.x = (i % mGridWidth) * mBoxWidth;
        state.y = (i / mGridWidth) * mBoxHeight;
        theme->drawSkin(g, SkinType::ShortcutBox, state);

        // Draw emote keyboard shortcut.
        const char *key = SDL_GetKeyName(
                    keyboard.getKeyValue(KeyboardConfig::KEY_EMOTE_1 + i));
        graphics->setColor(Theme::getThemeColor(Theme::TEXT));
        g->drawText(key, state.x + 2, state.y + 2, gcn::Graphics::LEFT);

        int emoteId = emoteShortcut->getEmote(i);
        if (emoteId != -1)
        {
            if (auto image = EmoteDB::get(emoteId).image)
            {
                image->setAlpha(1.0f);
                g->drawImage(image, state.x + 2, state.y + 10);
            }
        }
    }

    if (mEmoteMoved != -1)
    {
        // Draw the emote image being dragged by the cursor.
        if (auto image = EmoteDB::get(mEmoteMoved).image)
        {
            image->setAlpha(1.0f);

            const int tPosX = mCursorPosX - (image->getWidth() / 2);
            const int tPosY = mCursorPosY - (image->getHeight() / 2);

            g->drawImage(image, tPosX, tPosY);
        }
    }
}

void EmoteShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (mEmoteMoved == -1 && mEmoteClicked)
        {
            const int index = getIndexFromGrid(event.getX(), event.getY());
            if (index == -1)
                return;

            const int emoteId = emoteShortcut->getEmote(index);
            if (emoteId != -1)
            {
                mEmoteMoved = emoteId;
                emoteShortcut->removeEmote(index);
            }
        }

        if (mEmoteMoved != -1)
        {
            mCursorPosX = event.getX();
            mCursorPosY = event.getY();
        }
    }
}

void EmoteShortcutContainer::mousePressed(gcn::MouseEvent &event)
{
    const int index = getIndexFromGrid(event.getX(), event.getY());
    if (index == -1)
        return;

    // Stores the selected emote if there is one.
    if (emoteShortcut->isEmoteSelected())
    {
        emoteShortcut->setEmote(index);
        emoteShortcut->setEmoteSelected(-1);
    }
    else if (emoteShortcut->getEmote(index) != -1)
    {
        mEmoteClicked = true;
    }
}

void EmoteShortcutContainer::mouseReleased(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        const int index = getIndexFromGrid(event.getX(), event.getY());

        if (emoteShortcut->isEmoteSelected())
            emoteShortcut->setEmoteSelected(-1);

        if (index == -1)
        {
            mEmoteMoved = -1;
            return;
        }

        if (mEmoteMoved != -1)
        {
            emoteShortcut->setEmotes(index, mEmoteMoved);
            mEmoteMoved = -1;
        }
        else if (mEmoteClicked)
        {
            emoteShortcut->useEmote(index);
        }

        mEmoteClicked = false;
    }
}
