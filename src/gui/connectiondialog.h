/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "client.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

/**
 * The connection dialog.
 *
 * \ingroup Interface
 */
class ConnectionDialog : public Window, gcn::ActionListener
{
    public:
        /**
         * Constructor.
         *
         * @param text        The text to display
         * @param cancelState The state to enter when Cancel is pressed
         *
         * @see Window::Window
         */
        ConnectionDialog(const std::string &text, State cancelState);

        /**
         * Called when the user presses Cancel. Restores the global state to
         * the previous one.
         */
        void action(const gcn::ActionEvent &);

        void draw(gcn::Graphics *graphics);

    private:
        gcn::Label *mLabel;
        State mCancelState;
};

#endif
