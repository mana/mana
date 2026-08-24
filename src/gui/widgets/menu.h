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

#include "gui/widgets/popup.h"

#include <guichan/keylistener.hpp>
#include <guichan/rectangle.hpp>

#include <functional>
#include <string>
#include <vector>

/**
 * A menu with a vertical list of items, as used for context menus.
 *
 * Each item is given the action to perform when it is activated:
 *
 *     menu->addItem(_("Attack"), [] { local_player->attack(); });
 *     menu->addSeparator();
 *     menu->addItem(_("Cancel"));
 *     menu->showAt(x, y);
 *
 * The frame is drawn with the Popup skin, while the items use the MenuItem
 * and MenuSeparator skins. A MenuItem skin state without a "text" element
 * falls back to the text color of the Popup skin palette.
 *
 * An open menu is modal: it covers the screen and takes the modal focus, so
 * that nothing behind it reacts to the mouse or the keyboard. Pressing Escape
 * or clicking outside of the items closes it.
 *
 * \ingroup GUI
 */
class Menu : public Popup, public gcn::KeyListener
{
    public:
        using Action = std::function<void ()>;

        explicit Menu(const std::string &name = "Menu");
        ~Menu() override;

        /**
         * Returns whether any menu is currently open. Used to keep the game
         * from acting on input that belongs to the menu.
         */
        static bool isAnyOpen() { return mOpenMenus > 0; }

        /**
         * Adds an item that performs \a action when it is activated.
         */
        void addItem(std::string caption, Action action);

        /**
         * Adds a line separating groups of items. Separators at the start or
         * the end of the menu, as well as repeated ones, are left out.
         */
        void addSeparator();

        /**
         * Removes all items from the menu.
         */
        void clear();

        bool isEmpty() const { return mItems.empty(); }

        /**
         * Shows the menu with its top-left corner at the given position,
         * moving it as needed to keep it on screen.
         */
        void showAt(int x, int y);

        /**
         * Shows the menu below \a widget, aligned with its left edge.
         */
        void showBelow(gcn::Widget *widget);

        /**
         * Hides the menu and removes its items.
         */
        void close();

        void draw(gcn::Graphics *graphics) override;

        void mouseMoved(gcn::MouseEvent &event) override;
        void mouseExited(gcn::MouseEvent &event) override;
        void mousePressed(gcn::MouseEvent &event) override;

        void keyPressed(gcn::KeyEvent &event) override;

    private:
        struct Entry
        {
            std::string caption;
            Action action;
            bool separator = false;

            int y = 0;          /**< Set by updateLayout */
            int height = 0;     /**< Set by updateLayout */
        };

        /**
         * Hides the menu and gives up the modal focus, keeping the items.
         */
        void hide();

        /**
         * Positions the items and sizes the menu box to fit them.
         */
        void updateLayout();

        /**
         * Returns the index of the item at the given position, or -1 when
         * there is no item there.
         */
        int getItemAt(int x, int y) const;

        /**
         * The area covered by the menu itself. The widget covers the whole
         * screen while the menu is open, so that it can catch clicks meant to
         * dismiss it.
         */
        gcn::Rectangle mBox;

        std::vector<Entry> mItems;
        int mHoveredItem = -1;

        static int mOpenMenus;
};
