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

#ifndef GUI_GUILD_DIALOG_H
#define GUI_GUILD_DIALOG_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

class TextField;

/**
* An option dialog.
 *
 * \ingroup GUI
 */
class TextDialog : public Window, public gcn::ActionListener
{
public:
    /**
     * Constructor.
     *
     * @see Window::Window
     */
    TextDialog(const std::string &title, const std::string &msg,
               Window *parent = NULL);

    /**
     * Called when receiving actions from the widgets.
     */
    void action(const gcn::ActionEvent &event);

    /**
     * Get the text in the textfield
     */
    const std::string &getText() const;

    /**
     * Set the OK button action id
     */
    void setOKButtonActionId(const std::string &name);

private:
    TextField *mTextField;
    gcn::Button *mOkButton;
};

#endif
