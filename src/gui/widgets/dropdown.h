/*
 *  The Mana World
 *  Copyright (C) 2006  The Mana World Development Team
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

#ifndef DROPDOWN_H
#define DROPDOWN_H

#include <guichan/widgets/dropdown.hpp>

class Image;
class ImageRect;

/**
 * A drop down box from which you can select different values.
 *
 * A ListModel provides the contents of the drop down. To be able to use
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
        DropDown(gcn::ListModel *listModel = NULL);

        ~DropDown();

        void draw(gcn::Graphics *graphics);

        void drawFrame(gcn::Graphics *graphics);

        // Inherited from KeyListener

        void keyPressed(gcn::KeyEvent& keyEvent);

        // Inherited from MouseListener

        void mouseWheelMovedUp(gcn::MouseEvent& mouseEvent);

        void mouseWheelMovedDown(gcn::MouseEvent& mouseEvent);

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
};

#endif // end DROPDOWN_H

