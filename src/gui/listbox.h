/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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
 *
 *  $Id$
 */

#ifndef _TMW_LISTBOX_H
#define _TMW_LISTBOX_H

#include <guichan/widgets/listbox.hpp>

class SelectionListener;

/**
 * A list box, meant to be used inside a scroll area. Same as the Guichan list
 * box except this one doesn't have a background, instead completely relying
 * on the scroll area. It also adds selection listener functionality.
 *
 * \ingroup GUI
 */
class ListBox : public gcn::ListBox
{
    public:
        /**
         * Constructor.
         */
        ListBox(gcn::ListModel *listModel);

        /**
         * Draws the list box.
         */
        void draw(gcn::Graphics *graphics);

        void mousePress(int x, int y, int button);
        void mouseRelease(int x, int y, int button);
        void mouseMotion(int x, int y);

        /**
         * Adds a listener to the list that's notified each time a change to
         * the selection occurs.
         */
        void addSelectionListener(SelectionListener *listener)
        {
            mListeners.push_back(listener);
        }

        /**
         * Removes a listener from the list that's notified each time a change
         * to the selection occurs.
         */
        void removeSelectionListener(SelectionListener *listener)
        {
            mListeners.remove(listener);
        }

        /**
         * Sets the index of the selected element.
         */
        void setSelected(int selected);

    private:
        /**
         * Sends out selection events to the list of selection listeners.
         */
        void fireSelectionChangedEvent();

        bool mMousePressed;    /**< Keeps track of mouse pressed status. */

        std::list<SelectionListener*> mListeners;
};

#endif
