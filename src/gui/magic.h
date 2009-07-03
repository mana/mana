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

#ifndef MAGIC_H
#define MAGIC_H

#include <vector>

#include "gui/widgets/window.h"

#include "guichanfwd.h"

#include <guichan/actionlistener.hpp>

/**
 * The magic interface.
 *
 * This window is hacked together quickly to test the spell
 * recharge netcode.
 * It does in no way represent how the interface is going to
 * look in the final version. Optimization / cleanup is
 * pointless, as it will be redesigned from scratch.
 *
 * \ingroup Interface
 */
class MagicDialog : public Window, public gcn::ActionListener
{
    public:
        MagicDialog();

        ~MagicDialog();

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Update the tabs in this dialog
         */
        void update();

        /**
          * Draw this window.
          */
        void draw(gcn::Graphics *g);

    private:
        std::vector<gcn::Button *> mSpellButtons;
};

extern MagicDialog *magicDialog;

#endif
