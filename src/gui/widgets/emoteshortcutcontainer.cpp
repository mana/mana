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

#include "configuration.h"
#include "emoteshortcut.h"
#include "graphics.h"
#include "imagesprite.h"
#include "item.h"
#include "keyboardconfig.h"

#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/theme.h"

static const int MAX_ITEMS = 12;

EmoteShortcutContainer::EmoteShortcutContainer()
{
    addMouseListener(this);
    addWidgetListener(this);

    mBackgroundImg = Theme::getImageFromTheme("item_shortcut_bgr.png");

    mBackgroundImg->setAlpha(config.getFloatValue("guialpha"));

    mMaxItems = std::min(EmoteDB::getEmoteCount(), MAX_ITEMS);

    mBoxHeight = mBackgroundImg->getHeight();
    mBoxWidth = mBackgroundImg->getWidth();
}

EmoteShortcutContainer::~EmoteShortcutContainer()
{
    mBackgroundImg->decRef();
}

void EmoteShortcutContainer::draw(gcn::Graphics *graphics)
{
    if (config.getFloatValue("guialpha") != mAlpha)
    {
        mAlpha = config.getFloatValue("guialpha");
        mBackgroundImg->setAlpha(mAlpha);
    }

    auto *g = static_cast<Graphics*>(graphics);

    graphics->setFont(getFont());

    for (int i = 0; i < mMaxItems; i++)
    {
        const int emoteX = (i % mGridWidth) * mBoxWidth;
        const int emoteY = (i / mGridWidth) * mBoxHeight;

        g->drawImage(mBackgroundImg, emoteX, emoteY);

        // Draw emote keyboard shortcut.
        const char *key = SDL_GetKeyName(
                    keyboard.getKeyValue(KeyboardConfig::KEY_EMOTE_1 + i));
        graphics->setColor(Theme::getThemeColor(Theme::TEXT));
        g->drawText(key, emoteX + 2, emoteY + 2, gcn::Graphics::LEFT);

        int emoteId = emoteShortcut->getEmote(i);
        if (emoteId != -1)
        {
            EmoteDB::get(emoteId).sprite->draw(g, emoteX + 2, emoteY + 10);
        }
    }

    if (mEmoteMoved != -1)
    {
        // Draw the emote image being dragged by the cursor.
        const ImageSprite *sprite = EmoteDB::get(mEmoteMoved).sprite.get();

        const int tPosX = mCursorPosX - (sprite->getWidth() / 2);
        const int tPosY = mCursorPosY - (sprite->getHeight() / 2);

        sprite->draw(g, tPosX, tPosY);
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

