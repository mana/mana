/*
 *  The Mana Fire Client
 *  Copyright (C) 2011-2012  The Mana Developers
 *  Copyright (C) 2012-2012  The Land of Fire Developers
 *
 *  This file is part of The Mana Fire Client.
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

#ifndef CUSTOMSERVERDIALOG_H
#define CUSTOMSERVERDIALOG_H

class Button;
class Label;
class TextField;
class ServerDialog;

#include "gui/widgets/window.h"

#include "net/serverinfo.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/listmodel.hpp>


/**
 * The custom server addition dialog.
 *
 * \ingroup Interface
 */
class CustomServerDialog : public Window,
                     public gcn::ActionListener,
                     public gcn::KeyListener
{
    public:
        CustomServerDialog(ServerDialog *parent, int index = -1);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        void keyPressed(gcn::KeyEvent &keyEvent);

        void logic();

    private:
        TextField *mServerAddressField;
        TextField *mPortField;
        TextField  *mNameField;
        TextField *mDescriptionField;
        Button *mOkButton;
        Button *mCancelButton;
        ServerDialog *mServerDialog;
        // The index of the entry to modify, -1 when only adding a new entry.
        int mIndex;
};

#endif // CUSTOMSERVERDIALOG_H
