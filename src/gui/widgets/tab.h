/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include <guichan/widgets/tab.hpp>

class ImageRect;
class TabbedArea;

/**
 * A tab, the same as the Guichan tab in 0.8, but extended to allow
 * transparency.
 */
class Tab : public gcn::Tab
{
    public:
        Tab();
        ~Tab() override;

        /**
         * Update the alpha value to the graphic components.
         */
        static void updateAlpha();

        /**
         * Draw the tabbed area.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Set the normal color fo the tab's text.
         */
        void setTabColor(const gcn::Color *color);

        /**
         * Set tab flashing state
         */
        void setFlash(bool flash);

    protected:
        friend class TabbedArea;
        virtual void setCurrent() {}

    private:
        /** Load images if no other instances exist yet */
        void init();

        static ImageRect tabImg[4];    /**< Tab state graphics */
        static int mInstances;         /**< Number of tab instances */
        static float mAlpha;

        const gcn::Color *mTabColor;
        bool mFlash;
};
