/*
 *  Desktop widget
 *  Copyright (c) 2009  The Mana World Development Team
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

#ifndef DESKTOP_H
#define DESKTOP_H

#include "gui/widgets/container.h"

#include <guichan/widgetlistener.hpp>

class Image;

/**
 * Desktop widget, for drawing a background image and color.
 *
 * It picks the best fitting background image. If the image doesn't fit, a
 * background color is drawn and the image is centered.
 *
 * When the desktop widget is resized, the background image is automatically
 * updated.
 *
 * The desktop also displays the client version in the top-right corner.
 *
 * \ingroup GUI
 */
class Desktop : public Container, gcn::WidgetListener
{
    public:
        Desktop();
        ~Desktop();

        /**
         * Has to be called after updates have been loaded.
         */
        void reloadWallpaper();

        void widgetResized(const gcn::Event &event);

        void draw(gcn::Graphics *graphics);

    private:
        void setBestFittingWallpaper();

        Image *mWallpaper;
};

#endif // DESKTOP_H
