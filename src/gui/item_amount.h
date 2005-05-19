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

#ifndef _TMW_ITEM_AMOUNT_WINDOW_H
#define _TMW_ITEM_AMOUNT_WINDOW_H

#include "window.h"
#include <string>
#include <sstream>

#define AMOUNT_TRADE_ADD 1
#define AMOUNT_ITEM_DROP 2

/**
 * Window used for selecting the amount of items to drop.
 *
 * \ingroup Interface
 */
class ItemAmountWindow : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        ItemAmountWindow(int usage, Window *parent);

        /**
         * Destructor.
         */
        virtual ~ItemAmountWindow();

        /**
         * Called when receiving actions from widget.
         */
        void action(const std::string& eventId);

        /**
         * Sets default amount value.
         */
        void resetAmount();

    private:
        int amount;                      /**< Amount of items to be dropped. */
        gcn::Label *itemAmountLabel;     /**< Item amount caption. */

        /**
         * Item Amount buttons.
         */
        gcn::Button *itemAmountPlusButton;
        gcn::Button *itemAmountMinusButton;
        gcn::Button *itemAmountOkButton;
        gcn::Button *itemAmountCancelButton;
};

#endif /* _TMW_ITEM_AMOUNT_WINDOW_H */
