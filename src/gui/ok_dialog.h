/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _OK_DIALOG_H
#define _OK_DIALOG_H

#include <guichan/actionlistener.hpp>

#include "button.h"
#include "scrollarea.h"
#include "textbox.h"
#include "window.h"

#include "../guichanfwd.h"

/**
 * An 'Ok' button dialog.
 *
 * \ingroup GUI
 */
class OkDialog : public Window, public gcn::ActionListener {
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        OkDialog(const std::string &title, const std::string &msg,
                Window *parent = NULL);

        unsigned int getNumRows();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

    private:
        TextBox *mTextBox;
        ScrollArea *mTextArea;
        gcn::Button *okButton;
};

#endif
