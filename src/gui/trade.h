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
 *  $Id $
 */

#ifndef _TMW_TRADE_H
#define _TMW_TRADE_H 

#include "../log.h"
#include "../net/network.h"
#include "../graphic/spriteset.h"
#include "itemcontainer.h"
#include "gui.h"
#include "window.h"

/**
 * Trade dialog.
 *
 * \ingroup GUI
 */
class TradeWindow : public Window, gcn::ActionListener {
    public:
        /**
         * Constructor.
         */
        TradeWindow();

        /**
         * Destructor.
         */
        ~TradeWindow();

        /**
         * Draws the trade window.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Add an item the trade window.
         */
        int addItem(int index, int id, bool own, int quantity, bool equipment);

        /**
         * Remove a item from the trade window.
         */
        int removeItem(int id, bool own);

        /**
         * Reset both item containers
         */
        int reset();

        /**
         * Change quantity of an item.
         */
        int changeQuantity(int index, bool own, int quantity);

        /**
         * Increase quantity of an item.
         */
        int increaseQuantity(int index, bool own, int quantity);
        
        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);
        
        ItemContainer *my_items;
        ItemContainer *trade_items;
    
    private:
        gcn::Label *nameLabel;
        gcn::Button *addButton, *cancelButton;
};

#endif
