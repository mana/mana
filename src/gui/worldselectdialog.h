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

#pragma once

#include "gui/widgets/window.h"

#include "net/worldinfo.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/listmodel.hpp>

#include <memory>

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

        ~WorldSelectDialog() override;

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event) override;

        void keyPressed(gcn::KeyEvent &keyEvent) override;

        void mouseClicked(gcn::MouseEvent &mouseEvent) override;

    private:
        std::unique_ptr<WorldListModel> mWorldListModel;
        gcn::ListBox *mWorldList;
        gcn::Button *mChangeLoginButton;
        gcn::Button *mChooseWorld;
};
