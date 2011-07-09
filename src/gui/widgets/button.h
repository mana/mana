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

#ifndef BUTTON_H
#define BUTTON_H

#include <guichan/widgets/button.hpp>

class ImageRect;
class Image;
class TextPopup;

/**
 * Button widget. Same as the Guichan button but with custom look.
 *
 * \ingroup GUI
 */
class Button : public gcn::Button
{
    public:
        /**
         * Default constructor.
         */
        Button();

        /**
         * Constructor, sets the caption of the button to the given string and
         * adds the given action listener.
         */
        Button(const std::string &caption, const std::string &actionEventId,
                gcn::ActionListener *listener);

        ~Button();

        /**
         * Draws the button.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Update the alpha value to the button components.
         */
        void updateAlpha();

        void adjustSize();

        void setCaption(const std::string& caption);

        /**
         * Set the icons available next to the text
         *
         * @note: The image given must be formatted to give horizontally
         * frames of the given width and height for the following states:
         * Standard, Highlighted, Pressed, and Disabled.
         * If the image is too short, the missing states won't be loaded.
         */
        bool setButtonIcon(const std::string& iconFile = std::string());

        /**
         * Set the button popup text when hovering it for a few seconds.
         *
         * @note: An empty text will disable the popup.
         */
        void setButtonPopupText(const std::string& text = std::string());

        void logic();
        void mouseMoved(gcn::MouseEvent &event);
        void mouseExited(gcn::MouseEvent &event);

    private:
        void init();

        void removeButtonIcon();

        static ImageRect* mButton;  /**< Button state graphics */
        static int mInstances;      /**< Number of button instances */
        static float mAlpha;

        Image** mButtonIcon;        /**< Button Icons graphics */

        static TextPopup* mTextPopup;       /**< The buttons popup */
        std::string mPopupText;             /**< the current button text */
};

#endif
