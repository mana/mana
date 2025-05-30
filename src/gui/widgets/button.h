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

#include <guichan/widgets/button.hpp>

#include <memory>
#include <vector>

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

        ~Button() override;

        /**
         * Draws the button.
         */
        void draw(gcn::Graphics *graphics) override;

        void adjustSize();

        void setCaption(const std::string &caption);

        /**
         * Set the icons available next to the text
         *
         * @note: The image given must be formatted to give horizontally
         * frames of the given width and height for the following states:
         * Standard, Highlighted, Pressed, and Disabled.
         * If the image is too short, the missing states won't be loaded.
         */
        bool setButtonIcon(const std::string &iconFile);

        /**
         * Set the button popup text when hovering it for a few seconds.
         *
         * @note: An empty text will disable the popup.
         */
        void setButtonPopupText(const std::string &text)
        { mPopupText = text; }

        void mouseMoved(gcn::MouseEvent &event) override;
        void mouseExited(gcn::MouseEvent &event) override;

    private:
        void init();

        void removeButtonIcon();

        static int mInstances;          /**< Number of button instances */

        std::vector<std::unique_ptr<Image>> mButtonIcon;  /**< Button Icons graphics */

        /**
         * The buttons popup
         * @note: This is a global object. One for all the buttons.
         */
        static TextPopup *mTextPopup;
        std::string mPopupText;         /**< the current button text */
};
