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
 */

#ifndef __TMW_BROWSERBOX_H__
#define __TMW_BROWSERBOX_H__

#include <iosfwd>
#include <vector>

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>

#include "../guichanfwd.h"
#include "../main.h"

class LinkHandler;

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
        void setLinkHandler(LinkHandler *linkHandler);

        /**
         * Sets the BrowserBox opacity.
         */
        void setOpaque(bool opaque);

        /**
         * Sets the Highlight mode for links.
         */
        void setHighlightMode(unsigned int highMode);

        /**
         * Sets the maximum numbers of rows in the browser box. 0 = no limit.
         */
        void setMaxRow(int max) {mMaxRows = max; };

        /**
         * Disable links & user defined colors to be used in chat input.
         */
        void disableLinksAndUserColors();

        /**
         * Adds a text row to the browser.
         */
        void addRow(const std::string &row);

        /**
         * Remove all rows.
         */
        void clearRows();

        /**
         * Handles mouse actions.
         */
        void mousePressed(gcn::MouseEvent &event);
        void mouseMoved(gcn::MouseEvent &event);

        /**
         * Draws the browser box.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * BrowserBox modes.
         */
        enum {
            AUTO_SIZE,
            AUTO_WRAP       /**< Maybe it needs a fix or to be redone. */
        };

        /**
         * Some colours used in the browser box
         */

        enum {
            BLACK = 0x000000,
            BGCOLOR = 0xffffff,
            HIGHLIGHT = 0xcacaca
        };

        /**
         * Highlight modes for links.
         * This can be used for a bitmask.
         */
        enum {
            UNDERLINE  = 1,
            BACKGROUND = 2
        };

    private:
        typedef std::list<std::string> TextRows;
        typedef TextRows::iterator TextRowIterator;
        TextRows mTextRows;

        typedef std::vector<BROWSER_LINK> Links;
        typedef Links::iterator LinkIterator;
        Links mLinks;

        LinkHandler *mLinkHandler;
        unsigned int mMode;
        unsigned int mHighMode;
        bool mOpaque;
        bool mUseLinksAndUserColors;
        int mSelectedLink;
        unsigned int mMaxRows;
};

#endif
