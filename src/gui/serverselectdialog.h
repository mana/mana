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

#ifndef _CHAR_SEL_SERVER_H
#define _CHAR_SEL_SERVER_H

#include "gui/widgets/window.h"

#include "main.h"

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>

class LoginData;
class ServerListModel;

/**
 * The server select dialog.
 *
 * \ingroup Interface
 */
class ServerSelectDialog : public Window, public gcn::ActionListener {
    public:
        /**
         * Constructor
         *
         * @see Window::Window
         */
        ServerSelectDialog(LoginData *loginData, State nextState);

        /**
         * Destructor.
         */
        ~ServerSelectDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

    private:
        LoginData *mLoginData;
        ServerListModel *mServerListModel;
        gcn::ListBox *mServerList;
        gcn::Button *mOkButton;
        State mNextState;
};

#endif
