/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

        void mousePressed(gcn::MouseEvent &event) override;
        void mouseDragged(gcn::MouseEvent &event) override;
        void mouseReleased(gcn::MouseEvent &event) override;

    protected:
        /**
         * Draws the minimap.
         */
        void drawContent(gcn::Graphics *graphics) override;

    private:
        Map *mMap = nullptr;
        ResourceRef<Image> mMapImage;
        float mWidthProportion = 0.5;
        float mHeightProportion = 0.5;
        int mPanOffsetX = 0;
        int mPanOffsetY = 0;
        int mLastPlayerX = 0;
        int mLastPlayerY = 0;
        int mPanMouseX = 0;
        int mPanMouseY = 0;
        bool mPanning = false;
        bool mResetView = true;
};

extern Minimap *minimap;
