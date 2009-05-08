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

#ifndef ITEM_AMOUNT_WINDOW_H
#define ITEM_AMOUNT_WINDOW_H

#include "gui/widgets/window.h"

#include <guichan/keylistener.hpp>
#include <guichan/actionlistener.hpp>

class IntTextField;
class Item;
class ItemPopup;
class Icon;

/**
 * Window used for selecting the amount of items to drop, trade or split.
 *
 * \ingroup Interface
 */
class ItemAmountWindow : public Window,
                         public gcn::ActionListener,
                         public gcn::KeyListener
{
    public:
        enum Usage {
            TradeAdd,
            ItemDrop,
            StoreAdd,
            StoreRemove,
            ItemSplit
        };

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Sets default amount value.
         */
        void resetAmount();

        // MouseListener
        void mouseMoved(gcn::MouseEvent &event);
        void mouseExited(gcn::MouseEvent &event);

        /**
         * Schedules the Item Amount window for deletion.
         */
        void close();

        void keyReleased(gcn::KeyEvent &keyEvent);

        /**
         * Creates the dialog, or bypass it if there aren't enough items.
         */
        static void showWindow(Usage usage, Window *parent, Item *item,
                         int maxRange = 0);

    private:
        static void finish(Item *item, int amount, Usage usage);

        ItemAmountWindow(Usage usage, Window *parent, Item *item,
                         int maxRange = 0);

        IntTextField *mItemAmountTextField;   /**< Item amount caption. */
        Item *mItem;
        Icon *mItemIcon;

        int mMax;
        Usage mUsage;
        ItemPopup *mItemPopup;

        /**
         * Item Amount buttons.
         */
        gcn::Slider *mItemAmountSlide;
};

#endif /* ITEM_AMOUNT_WINDOW_H */
