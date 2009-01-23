/*
 *  The Mana World
 *  Copyright (C) 2007  The Mana World Development Team
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

#ifndef _TMW_SHORTCUTWINDOW_H
#define _TMW_SHORTCUTWINDOW_H

#include "window.h"

#include "../guichanfwd.h"

class ShortcutContainer;
class ScrollArea;

/**
 * A window around the ItemShortcutContainer.
 *
 * \ingroup Interface
 */
class ShortcutWindow : public Window
{
    public:
        /**
         * Constructor.
         */
        ShortcutWindow(const char *title, ShortcutContainer *content);

        /**
         * Destructor.
         */
        ~ShortcutWindow();

        /**
         * Called whenever the widget changes size.
         */
        void widgetResized(const gcn::Event &event);

    private:
        ShortcutWindow();
        ShortcutContainer *mItems;

        ScrollArea *mScrollArea;
};

extern ShortcutWindow *itemShortcutWindow;
extern ShortcutWindow *emoteShortcutWindow;

#endif
