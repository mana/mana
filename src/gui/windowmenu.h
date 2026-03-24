/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "keyboardconfig.h"

#include "gui/widgets/container.h"

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>
#include <guichan/widgetlistener.hpp>

#include <map>

class Button;
class EmotePopup;
class Window;

/**
 * The window menu. Allows showing and hiding many of the different windows
 * used in the game.
 *
 * \ingroup Interface
 */
class WindowMenu : public Container,
                   public gcn::ActionListener,
                   public gcn::SelectionListener,
                   public gcn::WidgetListener
{
    public:
        WindowMenu();
        ~WindowMenu() override;

        void action(const gcn::ActionEvent &event) override;

        void valueChanged(const gcn::SelectionEvent &event) override;

        /**
         * Update the pop-up captions with new key shortcuts.
         */
        void updatePopUpCaptions();

        void widgetHidden(const gcn::Event &event) override;
        void widgetShown(const gcn::Event &event) override;

    private:
        void addButton(const std::string& text, int &x, int &h,
                       const std::string& iconPath = std::string(),
                       KeyboardConfig::KeyAction key =
                            KeyboardConfig::KEY_NO_VALUE);

        void addWindowButton(const std::string &text, Window *window,
                             int &x, int &h,
                             const std::string &iconPath = std::string(),
                             KeyboardConfig::KeyAction key =
                                  KeyboardConfig::KEY_NO_VALUE);

        std::map<Window *, Button *> mWindowButtons;
        Button *mEmoteButton = nullptr;
        EmotePopup *mEmotePopup = nullptr;
};
