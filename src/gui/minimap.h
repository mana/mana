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

#ifndef MINIMAP_H
#define MINIMAP_H

#include <guichan/widget.hpp>

class Image;
class Map;

/**
 * Shows a minimap image.
 *
 * The name of the map is defined by the map property "name". The minimap image
 * is defined by the map property "minimap". The path to the image should be
 * given relative to the root of the client data.
 *
 * \ingroup Interface
 */
class Minimap : public gcn::Widget
{
    public:
        Minimap();
        ~Minimap();

        /**
         * Sets the map image that should be displayed.
         */
        void setMap(Map *map);

        /**
         * Draws the minimap.
         */
        void draw(gcn::Graphics *graphics);

    private:
        Map *mMap;
        Image *mMapImage;
        float mWidthProportion;
        float mHeightProportion;
};

#endif
