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
#include "inventory.h"
#include "imagesprite.h"
#include "item.h"
#include "itemshortcut.h"
#include "keyboardconfig.h"
#include "localplayer.h"
#include "log.h"

#include "gui/palette.h"

#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/theme.h"

#include "utils/dtor.h"

static const int MAX_ITEMS = 12;

EmoteShortcutContainer::EmoteShortcutContainer():
    ShortcutContainer(),
    mEmoteClicked(false),
    mEmoteMoved(0)
{
    addMouseListener(this);
    addWidgetListener(this);

    mBackgroundImg = Theme::getImageFromTheme("item_shortcut_bgr.png");

    mBackgroundImg->setAlpha(config.getFloatValue("guialpha"));

    // Setup emote sprites
    for (int i = 0; i <= EmoteDB::getLast(); i++)
    {
        mEmoteImg.push_back(EmoteDB::get(i)->sprite);
    }

    mMaxItems = std::min(EmoteDB::getLast(), MAX_ITEMS);

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

    Graphics *g = static_cast<Graphics*>(graphics);

    graphics->setFont(getFont());

    for (int i = 0; i < mMaxItems; i++)
    {
        const int emoteX = (i % mGridWidth) * mBoxWidth;
        const int emoteY = (i / mGridWidth) * mBoxHeight;

        g->drawImage(mBackgroundImg, emoteX, emoteY);

        // Draw emote keyboard shortcut.
        const char *key = SDL_GetKeyName(
                         (SDL_Scancode) keyboard.getKeyValue(keyboard.KEY_EMOTE_1 + i));
        graphics->setColor(Theme::getThemeColor(Theme::TEXT));
        g->drawText(key, emoteX + 2, emoteY + 2, gcn::Graphics::LEFT);

        int emoteId = emoteShortcut->getEmote(i);
        if (emoteId > 0 && emoteId <= EmoteDB::getLast() + 1)
        {
            mEmoteImg[emoteId - 1]->draw(g, emoteX + 2, emoteY + 10);
        }
    }

    if (mEmoteMoved)
    {
        // Draw the emote image being dragged by the cursor.
        const ImageSprite* sprite = mEmoteImg[mEmoteMoved - 1];
        if (sprite)
        {
            const int tPosX = mCursorPosX - (sprite->getWidth() / 2);
            const int tPosY = mCursorPosY - (sprite->getHeight() / 2);

            sprite->draw(g, tPosX, tPosY);
        }
    }
}

void EmoteShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (!mEmoteMoved && mEmoteClicked)
        {
            const int index = getIndexFromGrid(event.getX(), event.getY());

            if (index == -1)
                return;

            const int emoteId = emoteShortcut->getEmote(index);

            if (emoteId)
            {
                mEmoteMoved = emoteId;
                emoteShortcut->removeEmote(index);
            }
        }
        if (mEmoteMoved)
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
        emoteShortcut->setEmoteSelected(0);
    }
    else if (emoteShortcut->getEmote(index))
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
            emoteShortcut->setEmoteSelected(0);

        if (index == -1)
        {
            mEmoteMoved = 0;
            return;
        }

        if (mEmoteMoved)
        {
            emoteShortcut->setEmotes(index, mEmoteMoved);
            mEmoteMoved = 0;
        }
        else if (emoteShortcut->getEmote(index) && mEmoteClicked)
        {
            emoteShortcut->useEmote(index + 1);
        }

        if (mEmoteClicked)
            mEmoteClicked = false;
    }
}

