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

/**
 * A scroll area.
 *
 * Contrary to Guichan's scroll area, this scroll area takes ownership over its
 * content. However, it won't delete a previously set content widget when
 * setContent is called!
 *
 * Also overrides several functions to support fixed-size scroll bar markers.
 *
 * \ingroup GUI
 */
class ScrollArea : public gcn::ScrollArea
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
         * Sets whether the scroll bar buttons are shown.
         */
        void setShowButtons(bool showButtons);

        /**
         * Logic function optionally adapts width or height of contents. This
         * depends on the scrollbar settings.
         */
        void logic() override;

        /**
         * Overridden to draw the frame if its size is 0.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Draws the background and border of the scroll area.
         */
        void drawFrame(gcn::Graphics *graphics) override;

        /**
         * Applies clipping to the contents.
         */
        void drawChildren(gcn::Graphics *graphics) override;

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
        void mouseMoved(gcn::MouseEvent &event) override;

        /**
         * Called when the mouse enteres the widget area.
         */
        void mouseEntered(gcn::MouseEvent &event) override;

        /**
         * Called when the mouse leaves the widget area.
         */
        void mouseExited(gcn::MouseEvent &event) override;

        void mousePressed(gcn::MouseEvent &mouseEvent) override;
        void mouseDragged(gcn::MouseEvent &mouseEvent) override;

    protected:
        /**
         * Initializes the scroll area.
         */
        void init();

        void drawBackground(gcn::Graphics *graphics) override;
        void drawUpButton(gcn::Graphics *graphics) override;
        void drawDownButton(gcn::Graphics *graphics) override;
        void drawLeftButton(gcn::Graphics *graphics) override;
        void drawRightButton(gcn::Graphics *graphics) override;
        void drawVBar(gcn::Graphics *graphics) override;
        void drawHBar(gcn::Graphics *graphics) override;
        void drawVMarker(gcn::Graphics *graphics) override;
        void drawHMarker(gcn::Graphics *graphics) override;

        void checkPolicies() override;

        /**
         * Shadowing these functions from gcn::ScrollArea with versions that
         * support hiding the buttons. We need to make sure we always use these
         * versions.
         */
        gcn::Rectangle getUpButtonDimension();
        gcn::Rectangle getDownButtonDimension();
        gcn::Rectangle getLeftButtonDimension();
        gcn::Rectangle getRightButtonDimension();
        gcn::Rectangle getVerticalBarDimension();
        gcn::Rectangle getHorizontalBarDimension();

        /**
         * Shadowing these functions from gcn::ScrollArea with versions that
         * supports fixed-size scroll bar markers. We need to make sure we
         * always use these versions.
         */
        gcn::Rectangle getVerticalMarkerDimension();
        gcn::Rectangle getHorizontalMarkerDimension();

        int mX = 0;
        int mY = 0;
        bool mHasMouse = false;
        bool mOpaque = true;
        bool mShowButtons = true;
};
