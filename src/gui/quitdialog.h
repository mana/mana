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

#ifndef QUITDIALOG_H
#define QUITDIALOG_H

#include "gui/widgets/window.h"

#include "guichanfwd.h"
#include "main.h"

#include <guichan/actionlistener.hpp>

/**
 * The quit dialog.
 *
 * \ingroup Interface
 */
class QuitDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor
         *
         * @quitGame;    to be used for getting out of the while loop in Game
         * @pointerToMe  will be set to NULL when the QuitDialog is destroyed
         */
        QuitDialog(bool *quitGame, QuitDialog **pointerToMe);

        /**
         * Destructor
         */
        ~QuitDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

    private:
        gcn::RadioButton *mLogoutQuit;
        gcn::RadioButton *mForceQuit;
        gcn::RadioButton *mSwitchAccountServer;
        gcn::RadioButton *mSwitchCharacter;
        gcn::Button *mOkButton;
        gcn::Button *mCancelButton;

        bool *mQuitGame;
        QuitDialog **mMyPointer;
};

#endif
