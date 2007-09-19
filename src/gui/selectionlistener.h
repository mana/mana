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

#ifndef _TMW_SELECTIONLISTENER_H__
#define _TMW_SELECTIONLISTENER_H__

#include <guichan/widget.hpp>
#include <guichan/event.hpp>

/**
 * An event that characterizes a change in the current selection.
 *
 * \ingroup GUI
 */
class SelectionEvent : public gcn::Event
{
    public:
        /**
         * Constructor.
         */
        SelectionEvent(gcn::Widget *source):
            gcn::Event(source)
        {
        }
};

/**
 * The listener that's notified when a selection value changes.
 *
 * \ingroup GUI
 */
class SelectionListener
{
    public:
        /**
         * Virtual destructor.
         */
        virtual ~SelectionListener() {}

        /**
         * Called whenever the value of the selection changes.
         */
        virtual void selectionChanged(const SelectionEvent &event) = 0;
};

typedef std::list<SelectionListener*> SelectionListeners;

#endif
