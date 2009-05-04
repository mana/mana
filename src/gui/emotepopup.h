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

#ifndef EMOTEPOPUP_H
#define EMOTEPOPUP_H

#include "gui/widgets/popup.h"

#include <guichan/mouselistener.hpp>

#include <list>
#include <vector>

class AnimatedSprite;
class Image;

namespace gcn {
    class SelectionListener;
}

/**
 * An emote popup. Used to activate emotes and assign them to shortcuts.
 *
 * \ingroup GUI
 */
class EmotePopup : public Popup,
                   public gcn::MouseListener
{
    public:
        /**
         * Constructor. Initializes the graphic.
         */
        EmotePopup();

        virtual ~EmotePopup();

        /**
         * Draws the emotes.
         */
        void draw(gcn::Graphics *graphics);

        void mousePressed(gcn::MouseEvent &event);
        void mouseMoved(gcn::MouseEvent &event);

        /**
         * Returns the selected emote.
         */
        int getSelectedEmote() const;

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
        void setSelectedEmoteIndex(int index);

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

        std::vector<const AnimatedSprite*> mEmotes;
        Image *mSelectionImage;
        int mSelectedEmoteIndex;
        int mHoveredEmoteIndex;

        int mRowCount;
        int mColumnCount;

        typedef std::list<gcn::SelectionListener*> Listeners;

        Listeners mListeners;

        static const int gridWidth;
        static const int gridHeight;
};

#endif
