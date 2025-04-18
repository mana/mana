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

#include "gui/widgets/window.h"

#include "resources/resource.h"

class Image;
class Map;

/**
 * Minimap window. Shows a minimap image and the name of the current map.
 *
 * The name of the map is defined by the map property "name". The minimap image
 * is defined by the map property "minimap". The path to the image should be
 * given relative to the root of the client data.
 *
 * \ingroup Interface
 */
class Minimap : public Window
{
    public:
        Minimap();
        ~Minimap() override;

        /**
         * Sets the map image that should be displayed.
         */
        void setMap(Map *map);

        /**
         * Toggles the displaying of the minimap.
         */
        void toggle();

        /**
         * Draws the minimap.
         */
        void draw(gcn::Graphics *graphics) override;

    private:
        Map *mMap = nullptr;
        ResourceRef<Image> mMapImage;
        float mWidthProportion = 0.5;
        float mHeightProportion = 0.5;
};

extern Minimap *minimap;
