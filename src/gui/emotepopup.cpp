/*
 *  Extended support for activating emotes
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "gui/emotepopup.h"

#include "imagesprite.h"
#include "configuration.h"
#include "emoteshortcut.h"
#include "graphics.h"
#include "log.h"

#include "resources/emotedb.h"
#include "resources/image.h"
#include "resources/theme.h"

#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

const int EmotePopup::gridWidth = 34;  // emote icon width + 4
const int EmotePopup::gridHeight = 36; // emote icon height + 4

static const int MAX_COLUMNS = 6;

EmotePopup::EmotePopup()
{
    mSelectionImage = Theme::getImageFromTheme("selection.png");
    if (!mSelectionImage)
        logger->error("Unable to load selection.png");

    mSelectionImage->setAlpha(config.getFloatValue("guialpha"));

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

    const int emoteCount = EmoteDB::getEmoteCount();

    for (int i = 0; i < emoteCount ; i++)
    {
        int row = i / mColumnCount;
        int column = i % mColumnCount;

        int emoteX = 4 + column * gridWidth;
        int emoteY = 4 + row * gridHeight;

        // Center the last row when there are less emotes than columns
        if (row == mRowCount - 1)
        {
            const int emotesLeft = emoteCount % mColumnCount;
            emoteX += (mColumnCount - emotesLeft) * gridWidth / 2;
        }

        // Draw selection image below hovered item
        if (i == mHoveredEmoteIndex)
        {
            static_cast<Graphics*>(graphics)->drawImage(
                    mSelectionImage, emoteX, emoteY + 4);
        }

        // Draw emote icon
        EmoteDB::getByIndex(i).sprite->draw(static_cast<Graphics*>(graphics), emoteX, emoteY);
    }
}

void EmotePopup::mouseExited(gcn::MouseEvent &event)
{
    Popup::mouseExited(event);

    mHoveredEmoteIndex = -1;
}

void EmotePopup::mousePressed(gcn::MouseEvent &event)
{
    if (event.getButton() != gcn::MouseEvent::LEFT)
        return;

    const int index = getIndexAt(event.getX(), event.getY());
    if (index != -1)
    {
        const int emoteId = EmoteDB::getByIndex(index).id;

        setSelectedEmoteId(emoteId);
        emoteShortcut->setEmoteSelected(emoteId);
    }
}

void EmotePopup::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    mHoveredEmoteIndex = getIndexAt(event.getX(), event.getY());
}

int EmotePopup::getSelectedEmoteId() const
{
    return mSelectedEmoteId;
}

void EmotePopup::setSelectedEmoteId(int emoteId)
{
    if (emoteId == mSelectedEmoteId)
        return;

    mSelectedEmoteId = emoteId;
    distributeValueChangedEvent();
}

int EmotePopup::getIndexAt(int x, int y) const
{
    if (mColumnCount <= 0)
        return -1;

    // Take into account the border
    x -= 2;
    y -= 4;

    const int row = y / gridHeight;

    // Take into account that the last row is centered
    if (row == mRowCount - 1)
    {
        const int emotesLeft = EmoteDB::getEmoteCount() % mColumnCount;
        const int emotesMissing = mColumnCount - emotesLeft;
        x -= emotesMissing * gridWidth / 2;
        if (x < 0)
            return -1;
    }

    const int column = std::min(x / gridWidth, mColumnCount - 1);
    const int index = column + (row * mColumnCount);

    if (index >= 0 && index < EmoteDB::getEmoteCount())
        return index;

    return -1;
}

void EmotePopup::recalculateSize()
{
    const int emoteCount = EmoteDB::getEmoteCount();

    if (emoteCount > 0) {
        mRowCount = emoteCount / MAX_COLUMNS;
        if (emoteCount % MAX_COLUMNS > 0)
            ++mRowCount;

        mColumnCount = std::min(MAX_COLUMNS, emoteCount);
    } else {
        mRowCount = 0;
        mColumnCount = 0;
    }

    setContentSize(mColumnCount * gridWidth, mRowCount * gridHeight);
}

void EmotePopup::distributeValueChangedEvent()
{
    const gcn::SelectionEvent event(this);

    for (auto &listener : mListeners)
    {
        listener->valueChanged(event);
    }
}
