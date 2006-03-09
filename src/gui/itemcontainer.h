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

#ifndef _TMW_ITEMCONTAINER_H__
#define _TMW_ITEMCONTAINER_H__

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>

class Image;
class Inventory;
class Item;
class Spriteset;

/**
 * An item container. Used to show items in inventory and trade dialog.
 *
 * \ingroup GUI
 */
class ItemContainer : public gcn::Widget, public gcn::MouseListener
{
    public:
        /**
         * Constructor. Initializes the graphic.
         */
        ItemContainer(Inventory *inventory);

        /**
         * Destructor.
         */
        virtual ~ItemContainer();

        /**
         * Handles the logic of the ItemContainer
         */
        void logic();

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Sets the width of the container. This is used to determine the new
         * height of the container.
         */
        void setWidth(int width);

        /**
         * Handles mouse click.
         */
        void mousePress(int mx, int my, int button);

        /**
         * Returns the selected item.
         */
        Item* getItem();

        /**
         * Set selected item to -1.
         */
        void selectNone();

    private:
        Inventory *mInventory;
        Spriteset *mItemset;
        Image *mSelImg;
        Item *mSelectedItem;

        int mMaxItems;
};

#endif
