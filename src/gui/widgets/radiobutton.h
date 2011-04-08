/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include <guichan/widgets/radiobutton.hpp>

class Image;

/**
 * Guichan based RadioButton with custom look
 */
class RadioButton : public gcn::RadioButton
{
    public:
        RadioButton(const std::string &caption,const std::string &group,
                bool marked = false);

        ~RadioButton();

        /**
         * Draws the radiobutton, not the caption.
         */
        void drawBox(gcn::Graphics* graphics);

        /**
         * Implementation of the draw methods.
         * Thus, avoiding the rhomb around the radio button.
         */
        void draw(gcn::Graphics* graphics);

        /**
         * Called when the mouse enteres the widget area.
         */
        void mouseEntered(gcn::MouseEvent& event);

        /**
         * Called when the mouse leaves the widget area.
         */
        void mouseExited(gcn::MouseEvent& event);

    private:
        static int instances;
        static float mAlpha;
        bool mHasMouse;
        static Image *radioNormal;
        static Image *radioChecked;
        static Image *radioDisabled;
        static Image *radioDisabledChecked;
        static Image *radioNormalHi;
        static Image *radioCheckedHi;
};

#endif // RADIOBUTTON_H
