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

#ifndef QUITDIALOG_H
#define QUITDIALOG_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include <vector>

class Button;

/**
 * The quit dialog.
 *
 * \ingroup Interface
 */
class QuitDialog : public Window, public gcn::ActionListener,
                   public gcn::KeyListener
{
    public:
        /**
         * Constructor
         *
         * @pointerToMe  will be set to NULL when the QuitDialog is destroyed
         */
        QuitDialog(QuitDialog **pointerToMe);

        ~QuitDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        void keyPressed(gcn::KeyEvent &keyEvent);

    private:
        void placeOption(ContainerPlacer &place, gcn::RadioButton *option);
        std::vector<gcn::RadioButton*> mOptions;

        gcn::RadioButton *mLogoutQuit;
        gcn::RadioButton *mForceQuit;
        gcn::RadioButton *mSwitchAccountServer;
        gcn::RadioButton *mSwitchCharacter;
        Button *mOkButton;
        Button *mCancelButton;

        QuitDialog **mMyPointer;
};

#endif
