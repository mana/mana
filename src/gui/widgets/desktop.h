/*
 *  Desktop widget
 *  Copyright (c) 2009-2010  The Mana World Development Team
 *  Copyright (C) 2010-2012  The Mana Developers
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

#ifndef DESKTOP_H
#define DESKTOP_H

#include "guichanfwd.h"

#include "gui/widgets/container.h"

#include "resources/resource.h"

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
        ~Desktop() override;

        /**
         * Has to be called after updates have been loaded.
         */
        void reloadWallpaper();

        void widgetResized(const gcn::Event &) override;

        void draw(gcn::Graphics *graphics) override;

    private:
        void setBestFittingWallpaper();

        ResourceRef<Image> mWallpaper;
        gcn::Label *mVersionLabel;
};

#endif // DESKTOP_H
