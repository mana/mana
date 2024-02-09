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

#ifndef LISTBOX_H
#define LISTBOX_H

#include <guichan/widgets/listbox.hpp>

class SelectionListener;

/**
 * A list box, meant to be used inside a scroll area. Same as the Guichan list
 * box except this one doesn't have a background, instead completely relying
 * on the scroll area. It also adds selection listener functionality.
 *
 * \ingroup GUI
 */
class ListBox : public gcn::ListBox
{
    public:
        ListBox(gcn::ListModel *listModel);

        ~ListBox() override;

        /**
         * Sets the font to render the text in.
         *
         * @param font the font to use.
         */
        void setFont(gcn::Font *font)
        {
            mFont = font;
        }

        /**
         * Draws the list box.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Update the alpha value to the graphic components.
         */
        static void updateAlpha();

        // Inherited from KeyListener

        void keyPressed(gcn::KeyEvent& keyEvent) override;

        // Inherited from MouseListener

        void mousePressed(gcn::MouseEvent& mouseEvent) override;

        void mouseWheelMovedUp(gcn::MouseEvent& mouseEvent) override;

        void mouseWheelMovedDown(gcn::MouseEvent& mouseEvent) override;

        void mouseDragged(gcn::MouseEvent &event) override;

    private:
        gcn::Font *mFont;

    protected:
        static float mAlpha;
};

#endif
