/*
 *  The Mana Client
 *  Copyright (C) 2008-2010  The Mana World Development Team
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

#ifndef ICON_H
#define ICON_H

#include <guichan/widget.hpp>

class Image;

/**
 * An icon.
 *
 * \ingroup GUI
 */
class Icon : public gcn::Widget
{
    public:
        /**
         * Constructor.
         */
        Icon(const std::string &filename);

        /**
         * Constructor, uses an existing Image.
         */
        Icon(Image *image);

        /** 
         * Gets the current Image.
         */
        Image *getImage() const { return mImage; }

        /**
         * Sets the image to display.
         */
        void setImage(Image *image);

        /**
         * Draws the Icon.
         */
        void draw(gcn::Graphics *g);

    private:
        Image *mImage;
};

#endif // ICON_H
