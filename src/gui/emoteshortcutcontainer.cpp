/*
 *  Aethyra
 *  Copyright 2009 Aethyra Development Team
 *
 *  This file is part of Aethyra.
 *
 *  Aethyra is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  Aethyra is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Aethyra; if not, write to the Free Software Foundation, 
 *  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "emoteshortcutcontainer.h"

#include "../emoteshortcut.h"
#include "../graphics.h"
#include "../inventory.h"
#include "../item.h"
#include "../itemshortcut.h"
#include "../keyboardconfig.h"
#include "../localplayer.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

EmoteShortcutContainer::EmoteShortcutContainer():
    mEmoteClicked(false),
    mEmoteMoved(0)
{
    mGridWidth = 1,
    mGridHeight = 1,
    addMouseListener(this);
    addWidgetListener(this);

    ResourceManager *resman = ResourceManager::getInstance();

    mBackgroundImg = resman->getImage("graphics/gui/item_shortcut_bgr.png");
    mEmoteImg = resman->getImageSet("graphics/sprites/emotions.png", 30, 32);
    if (!mEmoteImg) logger->error(_("Unable to load emotions"));

    mMaxItems = emoteShortcut->getEmoteCount(); 

    mBoxHeight = mBackgroundImg->getHeight();
    mBoxWidth = mBackgroundImg->getWidth();
}

EmoteShortcutContainer::~EmoteShortcutContainer()
{
    mBackgroundImg->decRef();
    if (mEmoteImg)
    {
        mEmoteImg->decRef();
        mEmoteImg=NULL;
    }
}

void EmoteShortcutContainer::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    //graphics->setFont(getFont());

    for (int i = 0; i < mMaxItems; i++)
    {
        const int emoteX = (i % mGridWidth) * mBoxWidth;
        const int emoteY = (i / mGridWidth) * mBoxHeight;

        g->drawImage(mBackgroundImg, emoteX, emoteY);

        // Draw emote keyboard shortcut.
        const char *key = SDL_GetKeyName(
                         (SDLKey) keyboard.getKeyValue(keyboard.KEY_EMOTE_1 + i));
        graphics->setColor(0x000000);
        g->drawText(key, emoteX + 2, emoteY + 2, gcn::Graphics::LEFT);

        if (emoteShortcut->getEmote(i))
        {
            static_cast<Graphics*>(graphics)->drawImage(
                                   mEmoteImg->get(emoteShortcut->getEmote(i) - 1), emoteX + 2, emoteY + 10);
        }

    }
    if (mEmoteMoved)
    {
        // Draw the emote image being dragged by the cursor.
        Image* image = mEmoteImg->get(mEmoteMoved-1);
        if (image)
        {
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
        if (!mEmoteMoved && mEmoteClicked) 
        {
            const int index = getIndexFromGrid(event.getX(), event.getY());
            const int emoteId = emoteShortcut->getEmote(index);

            if (index == -1) 
            {
                return;
            }

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
    {
         return;
    }

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
        {
            emoteShortcut->setEmoteSelected(0);
        }

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
        {
            mEmoteClicked = false;
        }
    }
}

