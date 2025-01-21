/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#pragma once

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
        void action(const gcn::ActionEvent &event) override;

        /**
         * Sets default amount value.
         */
        void resetAmount();

        // MouseListener
        void mouseMoved(gcn::MouseEvent &event) override;
        void mouseExited(gcn::MouseEvent &event) override;

        /**
         * Schedules the Item Amount window for deletion.
         */
        void close() override;

        void keyReleased(gcn::KeyEvent &keyEvent) override;

        /**
         * Creates the dialog, or bypass it if there aren't enough items.
         */
        static void showWindow(Usage usage, Window *parent, Item *item,
                         int maxRange = 0);

        ~ItemAmountWindow() override;

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

        bool mEnabledKeyboard;
};
