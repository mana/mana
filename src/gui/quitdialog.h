/*
 *  The Mana World
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "guichanfwd.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include <vector>

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

        /**
         * Destructor
         */
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
        gcn::Button *mOkButton;
        gcn::Button *mCancelButton;

        QuitDialog **mMyPointer;
};

#endif
