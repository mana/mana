/*
 *  Extended support for activating emotes
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "gui/emotepopup.h"

#include "animatedsprite.h"
#include "configuration.h"
#include "emoteshortcut.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"

#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"

#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

const int EmotePopup::gridWidth = 34;  // emote icon width + 4
const int EmotePopup::gridHeight = 36; // emote icon height + 4

static const int MAX_COLUMNS = 6;

EmotePopup::EmotePopup():
    mSelectedEmoteIndex(-1),
    mHoveredEmoteIndex(-1),
    mRowCount(1),
    mColumnCount(1)
{
    // Setup emote sprites
    for (int i = 0; i <= EmoteDB::getLast(); ++i)
    {
        mEmotes.push_back(EmoteDB::getAnimation(i));
    }

    ResourceManager *resman = ResourceManager::getInstance();
    mSelectionImage = resman->getImage("graphics/gui/selection.png");
    if (!mSelectionImage)
        logger->error("Unable to load selection.png");

    mSelectionImage->setAlpha(config.getValue("guialpha", 0.8));

    addMouseListener(this);
    recalculateSize();
    setVisible(true);
}

EmotePopup::~EmotePopup()
{
    mSelectionImage->decRef();
}

void EmotePopup::draw(gcn::Graphics *graphics)
{
    Popup::draw(graphics);

    const int emoteCount = mEmotes.size();
    const int emotesLeft = mEmotes.size() % mColumnCount;

    for (int i = 0; i < emoteCount ; i++)
    {
        int row = i / mColumnCount;
        int column = i % mColumnCount;

        int emoteX = 4 + column * gridWidth;
        int emoteY = 4 + row * gridHeight;

        // Center the last row when there are less emotes than columns
        if (emotesLeft > 0 && row == mRowCount - 1)
            emoteX += (mColumnCount - emotesLeft) * gridWidth / 2;

        // Draw selection image below hovered item
        if (i == mHoveredEmoteIndex)
        {
            static_cast<Graphics*>(graphics)->drawImage(
                    mSelectionImage, emoteX, emoteY + 4);
        }

        // Draw emote icon
        mEmotes[i]->draw(static_cast<Graphics*>(graphics), emoteX, emoteY);
    }
}

void EmotePopup::mousePressed(gcn::MouseEvent &event)
{
    if (event.getButton() != gcn::MouseEvent::LEFT)
        return;

    const int index = getIndexAt(event.getX(), event.getY());
    if (index != -1) {
        setSelectedEmoteIndex(index);
        emoteShortcut->setEmoteSelected(index + 1);
    }
}

void EmotePopup::mouseMoved(gcn::MouseEvent &event)
{
    mHoveredEmoteIndex = getIndexAt(event.getX(), event.getY());
}

int EmotePopup::getSelectedEmote() const
{
    return 1 + mSelectedEmoteIndex;
}

void EmotePopup::setSelectedEmoteIndex(int index)
{
    if (index == mSelectedEmoteIndex)
        return;

    mSelectedEmoteIndex = index;
    distributeValueChangedEvent();
}

int EmotePopup::getIndexAt(int x, int y) const
{
    const int emotesLeft = mEmotes.size() % mColumnCount;
    const int row = y / gridHeight;
    int column;

    // Take into account that the last row is centered
    if (emotesLeft > 0 && row == mRowCount - 1)
    {
        int emotesMissing = mColumnCount - emotesLeft;
        column = std::min((x - emotesMissing * gridWidth / 2) / gridWidth,
                          emotesLeft - 1);
    }
    else
    {
        column = std::min(x / gridWidth, mColumnCount - 1);
    }

    int index = column + (row * mColumnCount);

    if ((unsigned) index < mEmotes.size())
        return index;

    return -1;
}

void EmotePopup::recalculateSize()
{
    const unsigned emoteCount = mEmotes.size();

    mRowCount = emoteCount / MAX_COLUMNS;
    if (emoteCount % MAX_COLUMNS > 0)
        ++mRowCount;

    mColumnCount = emoteCount / mRowCount;
    if (emoteCount % mRowCount > 0)
        ++mColumnCount;

    setContentSize(mColumnCount * gridWidth, mRowCount * gridHeight);
}

void EmotePopup::distributeValueChangedEvent()
{
    gcn::SelectionEvent event(this);
    Listeners::const_iterator i_end = mListeners.end();
    Listeners::const_iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
    {
        (*i)->valueChanged(event);
    }
}
