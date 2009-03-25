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

#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include <guichan/widgets/radiobutton.hpp>

class Image;

/*
 * Guichan based RadioButton with custom look
 */
class RadioButton : public gcn::RadioButton
{
    public:
        /*
         * Constructor.
         */
        RadioButton(const std::string &caption,const std::string &group,
                bool marked = false);

        /**
         * Destructor.
         */
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

    private:
        static int instances;
        static float mAlpha;
        static Image *radioNormal;
        static Image *radioChecked;
        static Image *radioDisabled;
        static Image *radioDisabledChecked;
};

#endif /* RADIOBUTTON_H */
