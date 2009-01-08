/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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

#ifndef _TMW_ITEMSHORTCUTCONTAINER_H__
#define _TMW_ITEMSHORTCUTCONTAINER_H__

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

#include "../guichanfwd.h"
#include "shortcutcontainer.h"

class Image;
class Item;

/**
 * An item shortcut container. Used to quickly use items.
 *
 * \ingroup GUI
 */
class ItemShortcutContainer : public ShortcutContainer
{
    public:
        /**
         * Constructor. Initializes the graphic.
         */
        ItemShortcutContainer();

        /**
         * Destructor.
         */
        virtual ~ItemShortcutContainer();

        /**
         * Handles the logic of the ItemContainer
         */
        void logic();

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Handles mouse when dragged.
         */
        void mouseDragged(gcn::MouseEvent &event);

        /**
         * Handles mouse when pressed.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Handles mouse release.
         */
        void mouseReleased(gcn::MouseEvent &event);

    private:
        bool mItemClicked;
        Item *mItemMoved;
};

#endif
