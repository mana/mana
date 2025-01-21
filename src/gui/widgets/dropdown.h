/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
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
         * @see ListModel
         */
        DropDown(gcn::ListModel *listModel = nullptr);

        ~DropDown() override;

        /**
         * Update the alpha value to the graphic components.
         */
        static void updateAlpha();

        void draw(gcn::Graphics *graphics) override;

        void drawFrame(gcn::Graphics *graphics) override;

        // Inherited from FocusListener

        void focusLost(const gcn::Event& event) override;

        // Inherited from KeyListener

        void keyPressed(gcn::KeyEvent& keyEvent) override;

        // Inherited from MouseListener

        void mousePressed(gcn::MouseEvent& mouseEvent) override;

        void mouseWheelMovedUp(gcn::MouseEvent& mouseEvent) override;

        void mouseWheelMovedDown(gcn::MouseEvent& mouseEvent) override;

    protected:
        /**
         * Draws the button with the little down arrow.
         *
         * @param graphics a Graphics object to draw with.
         */
        void drawButton(gcn::Graphics *graphics) override;

        // Add own Images.
        static int instances;
        static Image *buttons[2][2];
        static ImageRect skin;
        static float mAlpha;
};
