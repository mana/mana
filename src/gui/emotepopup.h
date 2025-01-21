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

#pragma once

#include "gui/widgets/popup.h"

#include <guichan/mouselistener.hpp>

#include <list>

class Image;

namespace gcn {
    class SelectionListener;
}

/**
 * An emote popup. Used to activate emotes and assign them to shortcuts.
 *
 * \ingroup GUI
 */
class EmotePopup : public Popup
{
    public:
        /**
         * Constructor. Initializes the graphic.
         */
        EmotePopup();

        ~EmotePopup() override;

        /**
         * Draws the emotes.
         */
        void draw(gcn::Graphics *graphics) override;

        void mouseExited(gcn::MouseEvent &event) override;
        void mousePressed(gcn::MouseEvent &event) override;
        void mouseMoved(gcn::MouseEvent &event) override;

        /**
         * Returns the selected emote.
         */
        int getSelectedEmoteId() const;

        /**
         * Adds a listener to the list that's notified each time a change to
         * the selection occurs.
         */
        void addSelectionListener(gcn::SelectionListener *listener)
        {
            mListeners.push_back(listener);
        }

        /**
         * Removes a listener from the list that's notified each time a change
         * to the selection occurs.
         */
        void removeSelectionListener(gcn::SelectionListener *listener)
        {
            mListeners.remove(listener);
        }

    private:
        /**
         * Sets the index of the currently selected emote.
         */
        void setSelectedEmoteId(int emoteId);

        /**
         * Returns the index at the specified coordinates. Returns -1 when
         * there is no valid index.
         */
        int getIndexAt(int x, int y) const;

        /**
         * Determine and set the size of the container.
         */
        void recalculateSize();

        /**
         * Sends out selection events to the list of selection listeners.
         */
        void distributeValueChangedEvent();

        Image *mSelectionImage;
        int mSelectedEmoteId = -1;
        int mHoveredEmoteIndex = -1;

        int mRowCount = 1;
        int mColumnCount = 1;

        std::list<gcn::SelectionListener *> mListeners;

        static const int gridWidth;
        static const int gridHeight;
};
