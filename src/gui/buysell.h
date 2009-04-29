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

#ifndef BUYSELL_H
#define BUYSELL_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

/**
 * A dialog to choose between buying or selling at a shop.
 *
 * \ingroup Interface
 */
class BuySellDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor. The action listener passed will receive "sell", "buy"
         * or "cancel" events when the respective buttons are pressed.
         *
         * @see Window::Window
         */
        BuySellDialog();

        /**
         * Check for current NPC
         */
        void logic();

        void setVisible(bool visible);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

    private:
        gcn::Button *mBuyButton;
};

extern BuySellDialog *buySellDialog;

#endif
