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
 *
 *  $Id$
 */

#ifndef _TMW_NPC_TEXT_H
#define _TMW_NPC_TEXT_H

#include <iosfwd>

#include <guichan/actionlistener.hpp>

#include "window.h"

#include "../guichanfwd.h"

/**
 * The npc text dialog.
 *
 * \ingroup Interface
 */
class NpcTextDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        NpcTextDialog();

        /**
         * Destructor.
         */
        ~NpcTextDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);

        /**
         * Sets the text shows in the dialog.
         *
         * @param string The new text.
         */
        void setText(const char *string);

        /**
         * Adds the text to the text shows in the dialog. Also adds a newline
         * to the end.
         *
         * @param string The text to add.
         */
        void addText(const char *string);

    private:
        gcn::Button *okButton;
        gcn::TextBox *textBox;
        gcn::ScrollArea *scrollArea;
};

extern NpcTextDialog *npcTextDialog;

#endif
