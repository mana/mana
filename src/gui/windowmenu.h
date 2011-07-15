/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#ifndef WINDOWMENU_H
#define WINDOWMENU_H

#include "keyboardconfig.h"

#include "gui/widgets/container.h"

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

class EmotePopup;

/**
 * The window menu. Allows showing and hiding many of the different windows
 * used in the game.
 *
 * \ingroup Interface
 */
class WindowMenu : public Container,
                   public gcn::ActionListener,
                   public gcn::SelectionListener
{
    public:
        WindowMenu();
        ~WindowMenu();

        void action(const gcn::ActionEvent &event);

        void valueChanged(const gcn::SelectionEvent &event);

    private:
        inline void addButton(const std::string& text, int &x, int &h,
                              const std::string& iconPath = std::string(),
                              KeyboardConfig::KeyAction key =
                                                  KeyboardConfig::KEY_NO_VALUE);

        EmotePopup *mEmotePopup;
};

#endif
