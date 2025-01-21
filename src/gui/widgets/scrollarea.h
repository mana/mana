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

#include <guichan/widgets/scrollarea.hpp>
#include <guichan/widgetlistener.hpp>

class Image;
class ImageRect;

/**
 * A scroll area.
 *
 * Contrary to Guichan's scroll area, this scroll area takes ownership over its
 * content. However, it won't delete a previously set content widget when
 * setContent is called!
 *
 * \ingroup GUI
 */
class ScrollArea : public gcn::ScrollArea, public gcn::WidgetListener
{
    public:
        /**
         * Constructor that takes no content. Needed for use with the DropDown
         * class.
         */
        ScrollArea();

        /**
         * Constructor.
         *
         * @param content the initial content to show in the scroll area
         */
        ScrollArea(gcn::Widget *content);

        /**
         * Destructor. Also deletes the content.
         */
        ~ScrollArea() override;

        /**
         * Logic function optionally adapts width or height of contents. This
         * depends on the scrollbar settings.
         */
        void logic() override;

        /**
         * Update the alpha value to the graphic components.
         */
        static void updateAlpha();

        /**
         * Draws the scroll area.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Draws the background and border of the scroll area.
         */
        void drawFrame(gcn::Graphics *graphics) override;

        /**
         * Sets whether the widget should draw its background or not.
         */
        void setOpaque(bool opaque);

        /**
         * Returns whether the widget draws its background or not.
         */
        bool isOpaque() const { return mOpaque; }

        /**
         * Called when the mouse moves in the widget area.
         */
        void mouseMoved(gcn::MouseEvent& event) override;

        /**
         * Called when the mouse enteres the widget area.
         */
        void mouseEntered(gcn::MouseEvent& event) override;

        /**
         * Called when the mouse leaves the widget area.
         */
        void mouseExited(gcn::MouseEvent& event) override;

        void widgetResized(const gcn::Event &event) override;

    protected:
        enum BUTTON_DIR {
            UP,
            DOWN,
            LEFT,
            RIGHT
        };

        /**
         * Initializes the scroll area.
         */
        void init();

        void drawButton(gcn::Graphics *graphics, BUTTON_DIR dir);
        void drawUpButton(gcn::Graphics *graphics) override;
        void drawDownButton(gcn::Graphics *graphics) override;
        void drawLeftButton(gcn::Graphics *graphics) override;
        void drawRightButton(gcn::Graphics *graphics) override;
        void drawVBar(gcn::Graphics *graphics) override;
        void drawHBar(gcn::Graphics *graphics) override;
        void drawVMarker(gcn::Graphics *graphics) override;
        void drawHMarker(gcn::Graphics *graphics) override;

        static int instances;
        static float mAlpha;
        static ImageRect background;
        static ImageRect vMarker;
        static ImageRect vMarkerHi;
        static Image *buttons[4][2];

        int mX = 0;
        int mY = 0;
        bool mHasMouse = false;
        bool mOpaque = true;
};
