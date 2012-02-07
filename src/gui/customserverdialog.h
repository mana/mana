/*
 *  The Mana Client
 *  Copyright (C) 2011-2012  The Mana Developers
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

#ifndef CUSTOMSERVERDIALOG_H
#define CUSTOMSERVERDIALOG_H

class Button;
class Label;
class TextField;
class DropDown;
class ServerDialog;
class TypeListModel;

#include "gui/widgets/window.h"

#include "net/serverinfo.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/listmodel.hpp>


/**
 * Server Type List Model
 */
class TypeListModel : public gcn::ListModel
{
    public:
        TypeListModel() {}

        /**
         * Used to get number of line in the list
         */
        int getNumberOfElements() { return 2; }

        /**
         * Used to get an element from the list
         */
        std::string getElementAt(int elementIndex);
};

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

        ~CustomServerDialog();

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
#ifdef ENABLE_MANASERV
        DropDown *mTypeField;
        TypeListModel *mTypeListModel;
#endif
        ServerDialog *mServerDialog;
        // The index of the entry to modify, -1 when only adding a new entry.
        int mIndex;
};

#endif // CUSTOMSERVERDIALOG_H
