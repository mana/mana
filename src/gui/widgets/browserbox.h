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

#pragma once

#include "utils/time.h"

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>

#include <deque>
#include <optional>
#include <vector>

class LinkHandler;
struct LayoutContext;

struct BrowserLink
{
    gcn::Rectangle rect;    /**< Where link is placed */
    std::string link;
    std::string caption;

    bool contains(int x, int y) const
    {
        return rect.isPointInRect(x, y);
    }
};

struct LinePart
{
    int x;
    int y;
    gcn::Color color;
    std::optional<gcn::Color> outlineColor;
    std::string text;
    gcn::Font *font;
};

struct TextRow
{
    std::string text;
    std::vector<LinePart> parts;
    std::vector<BrowserLink> links;
    int width = 0;
    int height = 0;
};

/**
 * A simple browser box able to handle links and forward events to the
 * parent conteiner.
 */
class BrowserBox : public gcn::Widget,
                   public gcn::MouseListener
{
    public:
        enum Mode
        {
            AUTO_SIZE,
            AUTO_WRAP       /**< Maybe it needs a fix or to be redone. */
        };

        BrowserBox(Mode mode = AUTO_SIZE);
        ~BrowserBox() override;

        /**
         * Sets the handler for links.
         */
        void setLinkHandler(LinkHandler *handler) { mLinkHandler = handler; }

        void setPalette(int palette) { mPalette = palette; }

        /**
         * Sets the Highlight mode for links.
         */
        void setHighlightMode(unsigned int mode) { mHighlightMode = mode; }

        /**
         * Sets the wrap indent for the browser box.
         */
        void setWrapIndent(int indent) { mWrapIndent = indent; }

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
        void setMaxRows(unsigned maxRows) { mMaxRows = maxRows; }

        /**
         * Disable links & user defined colors to be used in chat input.
         */
        void disableLinksAndUserColors() { mUseLinksAndUserColors = false; }

        /**
         * Enable or disable the replacement of keys.
         */
        void setEnableKeys(bool enable) { mEnableKeys = enable; }

        /**
         * Adds one or more text rows to the browser, separated by '\n'.
         */
        void addRows(std::string_view rows);

        /**
         * Adds a text row to the browser.
         */
        void addRow(std::string_view row);

        /**
         * Remove all rows.
         */
        void clearRows();

        /**
         * Handles mouse actions.
         */
        void mousePressed(gcn::MouseEvent &event) override;
        void mouseMoved(gcn::MouseEvent &event) override;
        void mouseExited(gcn::MouseEvent &event) override;

        /**
         * Draws the browser box.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Overridden to avoid drawing the default frame.
         */
        void drawFrame(gcn::Graphics *) override {}

        /**
         * Highlight modes for links.
         * This can be used for a bitmask.
         */
        enum LinkHighlightMode
        {
            UNDERLINE  = 1,
            BACKGROUND = 2
        };

    private:
        void relayoutText();
        void layoutTextRow(TextRow &row, LayoutContext &context);
        void updateHoveredLink(int x, int y);
        void maybeRelayoutText();

        std::deque<TextRow> mTextRows;

        LinkHandler *mLinkHandler = nullptr;
        int mPalette = 0;
        Mode mMode;
        unsigned int mHighlightMode = UNDERLINE | BACKGROUND;
        int mWrapIndent = 0;
        bool mShadows = false;
        bool mOutline = false;
        bool mUseLinksAndUserColors = true;
        bool mEnableKeys = false;
        std::optional<BrowserLink> mHoveredLink;
        unsigned int mMaxRows = 0;
        int mLastLayoutWidth = 0;
        Timer mLayoutTimer;
};
