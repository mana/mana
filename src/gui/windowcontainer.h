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

#ifndef _TMW_WINDOWCONTAINER_H_
#define _TMW_WINDOWCONTAINER_H_

#include <guichan.hpp>

/**
 * A window container. This container makes draggable windows possible.
 *
 * \ingroup GUI
 */
class WindowContainer : public gcn::Container {
    public:
        /**
         * Constructor.
         */
        WindowContainer();

        /**
         * Do GUI logic. This functions adds automatic deletion of objects that
         * volunteered to be deleted.
         */
        void logic();

        /**
         * Handles mouse input messages. Differs from standard behaviour in
         * that widget with mouse doesn't change while a button is pressed.
         */
        void _mouseInputMessage(const gcn::MouseInput &mouseInput);

        /**
         * Adds a widget. The widget can be set to be modal, which will ensure
         * only that widget will receive input.
         */
        void add(gcn::Widget *widget, bool modal = false);

        /**
         * Removes a widget.
         */
        void remove(gcn::Widget *widget);

        /**
         * This function is called by the containers children when they get
         * destroyed.
         */
        void _announceDeath(gcn::Widget *widget);

        /**
         * Clears the container of all widgets.
         */
        void clear();

        /**
         * Sets the modal widget. This will ensure only this widget will
         * receive mouse or keyboard input events.
         *
         * @see Window::Window
         */
        void setModalWidget(gcn::Widget *widget);

        /**
         * Returns the current modal widget, or <code>NULL</code> if there
         * is none.
         */
        gcn::Widget *getModalWidget();

        /**
         * Schedule a widget for deletion. It will be deleted at the start of
         * the next logic update.
         */
        void scheduleDelete(gcn::Widget *widget);

    protected:
        bool mouseDown;
        gcn::Widget *modalWidget;

        std::list<gcn::Widget*> deathList;
};

#endif
