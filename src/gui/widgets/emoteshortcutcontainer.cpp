/*
 *  Extended support for activating emotes
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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
                image->setAlpha(isSlotDragged(i) ? 0.5f : 1.0f);
                g->drawImage(image, state.x + 2, state.y + 10);
            }
        }
    }
}

void EmoteShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (!gui->getActiveDrag() && mClickedIndex != -1)
        {
            const int emoteId = emoteShortcut->getEmote(mClickedIndex);
            if (emoteId < 0)
                return;
            gui->startDrag(Drag::fromEmoteShortcut(emoteId, this, mClickedIndex));
        }
    }
}

void EmoteShortcutContainer::mousePressed(gcn::MouseEvent &event)
{
    mClickedIndex = getIndexFromGrid(event.getX(), event.getY());
    if (mClickedIndex == -1)
        return;

    // Stores the selected emote if there is one.
    if (emoteShortcut->isEmoteSelected())
    {
        emoteShortcut->setEmote(mClickedIndex);
        emoteShortcut->setEmoteSelected(-1);
        mClickedIndex = -1;
    }
}

void EmoteShortcutContainer::mouseReleased(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        const int index = getIndexFromGrid(event.getX(), event.getY());

        if (emoteShortcut->isEmoteSelected())
            emoteShortcut->setEmoteSelected(-1);

        if (index != -1 && mClickedIndex == index)
            emoteShortcut->useEmote(index);

        mClickedIndex = -1;
    }
}

void EmoteShortcutContainer::removeSlot(int index)
{
    emoteShortcut->removeEmote(index);
}

bool EmoteShortcutContainer::handleDrop(const Drag &drag, int absX, int absY)
{
    if (drag.sourceType != Drag::SourceType::EmoteShortcut || drag.emoteId < 0)
        return false;

    const int index = getIndexFromAbsolute(absX, absY);
    if (index == -1)
        return false;

    const int replaced = emoteShortcut->getEmote(index);
    emoteShortcut->setEmotes(index, drag.emoteId);

    if (drag.source == this &&
        drag.sourceIndex >= 0 &&
        drag.sourceIndex != index)
    {
        emoteShortcut->setEmotes(drag.sourceIndex, replaced);
    }

    return true;
}
