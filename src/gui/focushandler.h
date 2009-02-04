/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef FOCUSHANDLER_H
#define FOCUSHANDLER_H

#include <list>

#include <guichan/focushandler.hpp>

/**
 * The focus handler. This focus handler does exactly the same as the Guichan
 * focus handler, but keeps a stack of modal widgets to be able to handle
 * multiple modal focus requests.
 */
class FocusHandler : public gcn::FocusHandler
{
    public:
        /**
         * Sets modal focus to a widget. When there is already a modal widget
         * then that widget loses modal focus and will regain it after this
         * widget releases his modal focus.
         */
        void requestModalFocus(gcn::Widget *widget);

        /**
         * Releases modal focus of a widget. When this widget had modal focus
         * and there are other widgets that had also requested modal focus,
         * then modal focus will be transfered to the last of those.
         */
        void releaseModalFocus(gcn::Widget *widget);

        /**
         * Removes a widget from the focus handler. Also makes sure no dangling
         * pointers remain in modal focus stack.
         */
        void remove(gcn::Widget *widget);

    private:
        /**
         * Stack of widgets that have requested modal forcus.
         */
        std::list<gcn::Widget*> mModalStack;
};

#endif
