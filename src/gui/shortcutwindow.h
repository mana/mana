/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef SHORTCUTWINDOW_H
#define SHORTCUTWINDOW_H

#include "gui/widgets/window.h"

class ScrollArea;
class ShortcutContainer;

/**
 * A window around a ShortcutContainer.
 *
 * \ingroup Interface
 */
class ShortcutWindow : public Window
{
    public:
        ShortcutWindow(const std::string &title, ShortcutContainer *content);

        ~ShortcutWindow();

    private:
        ShortcutWindow();
        ShortcutContainer *mItems;

        ScrollArea *mScrollArea;

        static int mBoxesWidth;
};

extern ShortcutWindow *itemShortcutWindow;
extern ShortcutWindow *emoteShortcutWindow;

#endif
