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
 */

#include "focushandler.h"

void FocusHandler::requestModalFocus(gcn::Widget *widget)
{
    /* If there is another widget with modal focus, remove its modal focus
     * and put it on the modal widget stack.
     */
    if (mModalFocusedWidget != NULL && mModalFocusedWidget != widget)
    {
        mModalStack.push_front(mModalFocusedWidget);
        mModalFocusedWidget = NULL;
    }

    gcn::FocusHandler::requestModalFocus(widget);
}

void FocusHandler::releaseModalFocus(gcn::Widget *widget)
{
    mModalStack.remove(widget);

    if (mModalFocusedWidget == widget)
    {
        gcn::FocusHandler::releaseModalFocus(widget);

        /* Check if there were any previously modal widgets that'd still like
         * to regain their modal focus.
         */
        if (mModalStack.size() > 0)
        {
            gcn::FocusHandler::requestModalFocus(mModalStack.front());
            mModalStack.pop_front();
        }
    }
}

void FocusHandler::remove(gcn::Widget *widget)
{
    releaseModalFocus(widget);

    gcn::FocusHandler::remove(widget);
}
