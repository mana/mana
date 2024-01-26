/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *  Copyright (C) 2009  Aethyra Development Team
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

#ifndef BROWSERBOX_H
#define BROWSERBOX_H

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>

#include <list>
#include <vector>

class LinkHandler;

struct BrowserLink
{
    int x1, x2, y1, y2;     /**< Where link is placed */
    std::string link;
    std::string caption;
};

class LinePart
{
    public:
        LinePart(int x, int y, gcn::Color color, std::string text) :
            mX(x), mY(y), mColor(color), mText(text)
        {
        }

        int getX() const
        { return mX; }

        int getY() const
        { return mY; }

        const std::string &getText() const
        { return mText; }

        const gcn::Color &getColor() const
        { return mColor; }

    private:
        int mX, mY;
        gcn::Color mColor;
        std::string mText;
};

/**
 * A simple browser box able to handle links and forward events to the
 * parent conteiner.
 */
class BrowserBox : public gcn::Widget,
                   public gcn::MouseListener
{
    public:
        BrowserBox(unsigned int mode = AUTO_SIZE, bool opaque = true);

        ~BrowserBox() override;

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
         * Sets whether the font will use a shadow for text.
         */
        void setShadowedText(bool shadows) { mShadows = shadows; }

        /**
         * Sets whether the font will use a shadow for text.
         */
        void setOutlinedText(bool outline) { mOutline = outline; }

        /**
         * Sets the maximum numbers of rows in the browser box. 0 = no limit.
         */
        void setMaxRow(unsigned max) {mMaxRows = max; }

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
        void mousePressed(gcn::MouseEvent &event) override;
        void mouseMoved(gcn::MouseEvent &event) override;

        /**
         * Draws the browser box.
         */
        void draw(gcn::Graphics *graphics) override;

        void updateHeight();

        /**
         * BrowserBox modes.
         */
        enum
        {
            AUTO_SIZE = 0,
            AUTO_WRAP       /**< Maybe it needs a fix or to be redone. */
        };

        /**
         * BrowserBox colors.
         *
         * NOTES (by Javila):
         *  - color values is "0x" prefix followed by HTML color style.
         *  - we can add up to 10 different colors: [0..9].
         *  - not all colors will be fine with all backgrounds due transparent
         *    windows and widgets. So, I think it's better keep BrowserBox
         *    opaque (white background) by default.
         */
        enum
        {
            RED = 0xff0000,         /**< Color 1 */
            GREEN = 0x009000,       /**< Color 2 */
            BLUE = 0x0000ff,        /**< Color 3 */
            ORANGE = 0xe0980e,      /**< Color 4 */
            YELLOW = 0xf1dc27,      /**< Color 5 */
            PINK = 0xff00d8,        /**< Color 6 */
            PURPLE = 0x8415e2,      /**< Color 7 */
            GRAY = 0x919191,        /**< Color 8 */
            BROWN = 0x8e4c17        /**< Color 9 */
        };

        /**
         * Highlight modes for links.
         * This can be used for a bitmask.
         */
        enum
        {
            UNDERLINE  = 1,
            BACKGROUND = 2
        };

        using TextRows = std::list<std::string>;

        TextRows &getRows()
        { return mTextRows; }

        void setAlwaysUpdate(bool n)
        { mAlwaysUpdate = n; }

    private:
        int calcHeight();

        using TextRowIterator = TextRows::iterator;
        TextRows mTextRows;

        using LinePartList = std::list<LinePart>;
        using LinePartIterator = LinePartList::iterator;
        LinePartList mLineParts;

        using Links = std::vector<BrowserLink>;
        using LinkIterator = Links::iterator;
        Links mLinks;

        LinkHandler *mLinkHandler;
        unsigned int mMode;
        unsigned int mHighMode;
        bool mShadows;
        bool mOutline;
        bool mOpaque;
        bool mUseLinksAndUserColors;
        int mSelectedLink;
        unsigned int mMaxRows;
        int mHeight;
        int mWidth;
        int mYStart;
        int mUpdateTime;
        bool mAlwaysUpdate;
};

#endif
