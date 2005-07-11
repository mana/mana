/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifndef __TMW_BROWSERBOX_H__
#define __TMW_BROWSERBOX_H__

#include <guichan.hpp>
#include "linkhandler.h"

struct BROWSER_LINK {
    int x1, x2, y1, y2;     /**< Where link is placed */
    std::string link;
    std::string caption;
};

/**
 * A simple browser box able to handle links and forward events to the
 * parent conteiner.
 */
class BrowserBox : public gcn::Widget, public gcn::MouseListener
{
    public:
        /**
         * Constructor.
         */
        BrowserBox(unsigned int mode = AUTO_SIZE);

        /**
         * Destructor.
         */
        ~BrowserBox();

        /**
         * Sets the handler for links.
         */
        void setLinkHandler(LinkHandler* linkHandler);

        /**
         * Sets the BrowserBox opacity.
         */
        void setOpaque(bool opaque);

        /**
         * Sets the Highlight mode for links.
         */
        void setHighlightMode(unsigned int highMode);

        /**
         * Disable links & user defined colors to be used in chat input.
         */
        void disableLinksAndUserColors();

        /**
         * Adds a text row to the browser.
         */
        void addRow(const std::string& row);

        /**
         * Remove all rows.
         */
        void clearRows();

        /**
         * Handles mouse actions.
         */
        void mousePress(int mx, int my, int button);
        void mouseMotion(int mx, int my);

        /**
         * Draws the browser box.
         */
        void draw(gcn::Graphics* graphics);

        /**
         * BrowserBox modes.
         */
        enum {
            AUTO_SIZE,
            AUTO_WRAP       /**< Maybe it needs a fix or to be redone. */
        };

        /**
         * BrowserBox colors.
         *
         * NOTES (by Javila):
         *  - color values is "0x" prefix followed by HTML color style.
         *  - we can add up to 10 different colors: [0..9].
         *  - we need a link and a highlighted link colors.
         *  - not all colors will be fine with all backgrounds due transparent
         *    windows and widgets. So, I think it's better keep BrowserBox
         *    opaque (white background) by default.
         */
        enum {
            BLACK = 0x000000,       /**< Color 0 */
            RED = 0xff0000,         /**< Color 1 */
            GREEN = 0x1fa052,       /**< Color 2 */
            BLUE = 0x0000ff,        /**< Color 3 */
            ORANGE = 0xe0980e,      /**< Color 4 */
            YELLOW = 0xf1dc27,      /**< Color 5 */
            PINK = 0xff00d8,        /**< Color 6 */
            PURPLE = 0x8415e2,      /**< Color 7 */
            GRAY = 0x919191,        /**< Color 8 */
            BROWN = 0x8e4c17,       /**< Color 9 */
            BGCOLOR = 0xffffff,     /**< Bg color for opacity */
            LINK = 0xe50d0d,        /**< Color L */
            HIGHLIGHT = 0xcacaca    /**< Bg color for highlighted link */
        };

        /**
         * Highlight modes for links.
         */
        enum {
            UNDERLINE,
            BACKGROUND,
            BOTH
        };

    private:
        std::vector<std::string> mTextRows;
        std::vector<BROWSER_LINK> mLinks;
        LinkHandler *mLinkHandler;
        unsigned int mMode;
        unsigned int mHighMode;
        bool mOpaque;
        bool mUseLinksAndUserColors;
        int mSelectedLink;

        static int instances;      /**< Number of Window instances */
        static gcn::ImageFont* browserFont;
};

#endif
