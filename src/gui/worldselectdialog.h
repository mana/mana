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

#ifndef WORLD_SELECT_DIALOG_H
#define WORLD_SELECT_DIALOG_H

#include "gui/widgets/window.h"

#include "net/worldinfo.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/listmodel.hpp>

#include <vector>

class LoginData;
class WorldListModel;

/**
 * The server select dialog.
 *
 * \ingroup Interface
 */
class WorldSelectDialog : public Window, public gcn::ActionListener,
                          public gcn::KeyListener {
    public:
        WorldSelectDialog(Worlds worlds);

        ~WorldSelectDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        void keyPressed(gcn::KeyEvent &keyEvent);

        void mouseClicked(gcn::MouseEvent &mouseEvent);

    private:
        WorldListModel *mWorldListModel;
        gcn::ListBox *mWorldList;
        gcn::Button *mChangeLoginButton;
        gcn::Button *mChooseWorld;
};

#endif // WORLD_SELECT_DIALOG_H
