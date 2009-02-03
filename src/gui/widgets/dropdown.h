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
 */

#ifndef DROPDOWN_H
#define DROPDOWN_H

#include <iosfwd>

#include <guichan/widgets/dropdown.hpp>

#include "../listbox.h"
#include "../scrollarea.h"

#include "../../guichanfwd.h"

class Image;
class ImageRect;

    /**
     * A drop down box from which you can select different values. It is one of
     * the most complicated Widgets you will find in Guichan. For drawing the
     * DroppedDown box it uses one ScrollArea and one ListBox. It also uses an
     * internal FocusHandler to handle the focus of the internal ScollArea and
     * ListBox. DropDown uses a ListModel to handle the list. To be able to use
     * DropDown you must give DropDown an implemented ListModel which represents
     * your list.
     */
class DropDown : public gcn::DropDown
{
    public:
        /**
         * Contructor.
         *
         * @param listModel the ListModel to use.
         * @param scrollArea the ScrollArea to use.
         * @param listBox the listBox to use.
         * @see ListModel, ScrollArea, ListBox.
         */
        DropDown(gcn::ListModel *listModel = NULL,
                 gcn::ScrollArea *scrollArea = NULL,
                 gcn::ListBox *listBox = NULL,
                 bool opacity = true);

        /**
         * Destructor.
         */
        ~DropDown();

        void draw(gcn::Graphics* graphics);

        void drawFrame(gcn::Graphics* graphics);

        /**
         * Sets the widget to be opaque, that is sets the widget to display its
         * background.
         *
         * @param opaque True if the widget should be opaque, false otherwise.
         */
        void setOpaque(bool opaque) {mOpaque = opaque;}

        /**
         * Checks if the widget is opaque, that is if the widget area displays
         * its background.
         *
         * @return True if the widget is opaque, false otherwise.
         */
        bool isOpaque() const {return mOpaque;}


    protected:
        /**
         * Draws the button with the little down arrow.
         *
         * @param graphics a Graphics object to draw with.
         */
        void drawButton(gcn::Graphics *graphics);

        // Add own Images.
        static int instances;
        static Image *buttons[2][2];
        static ImageRect skin;
        static float mAlpha;

        bool mOpaque;
};

#endif // end DROPDOWN_H

