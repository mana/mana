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

#include "gui/widgets/browserbox.h"

#include "client.h"

#include "gui/truetypefont.h"
#include "gui/widgets/linkhandler.h"

#include "resources/theme.h"

#include <guichan/graphics.hpp>
#include <guichan/font.hpp>
#include <guichan/cliprectangle.hpp>

#include <algorithm>

struct LayoutContext
{
    LayoutContext(const gcn::Font *font);

    int y = 0;
    const gcn::Font *font;
    const int fontHeight;
    const int minusWidth;
    const int tildeWidth;
    int lineHeight;
    gcn::Color selColor;
    const gcn::Color textColor;
};

LayoutContext::LayoutContext(const gcn::Font *font)
    : font(font)
    , fontHeight(font->getHeight())
    , minusWidth(font->getWidth("-"))
    , tildeWidth(font->getWidth("~"))
    , lineHeight(fontHeight)
    , selColor(Theme::getThemeColor(Theme::TEXT))
    , textColor(Theme::getThemeColor(Theme::TEXT))
{
    if (auto *trueTypeFont = dynamic_cast<const TrueTypeFont*>(font))
        lineHeight = trueTypeFont->getLineHeight();
}


BrowserBox::BrowserBox(unsigned int mode):
    mMode(mode)
{
    setFocusable(true);
    addMouseListener(this);
}

BrowserBox::~BrowserBox()
{
}

void BrowserBox::addRow(const std::string &row)
{
    TextRow &newRow = mTextRows.emplace_back();

    // Use links and user defined colors
    if (mUseLinksAndUserColors)
    {
        std::string tmp = row;

        // Check for links in format "@@link|Caption@@"
        auto idx1 = tmp.find("@@");
        while (idx1 != std::string::npos)
        {
            const auto idx2 = tmp.find("|", idx1);
            const auto idx3 = tmp.find("@@", idx2);

            if (idx2 == std::string::npos || idx3 == std::string::npos)
                break;

            BrowserLink &link = newRow.links.emplace_back();
            link.link = tmp.substr(idx1 + 2, idx2 - (idx1 + 2));
            link.caption = tmp.substr(idx2 + 1, idx3 - (idx2 + 1));

            newRow.text += tmp.substr(0, idx1);
            newRow.text += "##<" + link.caption;

            tmp.erase(0, idx3 + 2);
            if (!tmp.empty())
            {
                newRow.text += "##>";
            }
            idx1 = tmp.find("@@");
        }

        newRow.text += tmp;
    }
    // Don't use links and user defined colors
    else
    {
        newRow.text = row;
    }

    // Layout the newly added row
    LayoutContext context(getFont());
    context.y = getHeight();
    layoutTextRow(newRow, context);

    // Auto size mode
    if (mMode == AUTO_SIZE && newRow.width > getWidth())
        setWidth(newRow.width);

    // Discard older rows when a row limit has been set
    // (this might invalidate the newRow reference)
    int removedHeight = 0;
    while (mMaxRows > 0 && mTextRows.size() > mMaxRows)
    {
        removedHeight += mTextRows.front().height;
        mTextRows.pop_front();
    }
    if (removedHeight > 0)
    {
        for (auto &row : mTextRows)
        {
            for (auto &part : row.parts)
            {
                part.y -= removedHeight;
            }

            for (auto &link : row.links)
            {
                link.y1 -= removedHeight;
                link.y2 -= removedHeight;
            }
        }
    }

    setHeight(context.y - removedHeight);
}

void BrowserBox::clearRows()
{
    mTextRows.clear();
    setSize(0, 0);
    mHoveredLink.reset();
    maybeRelayoutText();
}

void BrowserBox::mousePressed(gcn::MouseEvent &event)
{
    if (!mLinkHandler)
        return;

    updateHoveredLink(event.getX(), event.getY());

    if (mHoveredLink)
        mLinkHandler->handleLink(mHoveredLink->link);
}

void BrowserBox::mouseMoved(gcn::MouseEvent &event)
{
    updateHoveredLink(event.getX(), event.getY());
}

void BrowserBox::draw(gcn::Graphics *graphics)
{
    const gcn::ClipRectangle &cr = graphics->getCurrentClipArea();
    int yStart = cr.y - cr.yOffset;
    int yEnd = yStart + cr.height;
    if (yStart < 0)
        yStart = 0;

    if (getWidth() != mLastLayoutWidth)
        maybeRelayoutText();

    if (mHoveredLink)
    {
        auto &link = *mHoveredLink;

        if (mHighlightMode & BACKGROUND)
        {
            graphics->setColor(Theme::getThemeColor(Theme::HIGHLIGHT));
            graphics->fillRectangle(gcn::Rectangle(
                        link.x1,
                        link.y1,
                        link.x2 - link.x1,
                        link.y2 - link.y1
                        ));
        }

        if (mHighlightMode & UNDERLINE)
        {
            graphics->setColor(Theme::getThemeColor(Theme::HYPERLINK));
            graphics->drawLine(
                    link.x1,
                    link.y2,
                    link.x2,
                    link.y2);
        }
    }

    for (const auto &row : mTextRows)
    {
        for (const auto &part : row.parts)
        {
            if (part.y + 50 < yStart)
                continue;
            if (part.y > yEnd)
                return;

            // Use the correct font
            graphics->setFont(getFont());

            // Handle text shadows
            if (mShadows)
            {
                graphics->setColor(Theme::getThemeColor(Theme::SHADOW,
                                                        part.color.a / 2));

                if (mOutline)
                    graphics->drawText(part.text, part.x + 2, part.y + 2);
                else
                    graphics->drawText(part.text, part.x + 1, part.y + 1);
            }

            if (mOutline)
            {
                // Text outline
                graphics->setColor(Theme::getThemeColor(Theme::OUTLINE,
                                                        part.color.a / 4));
                graphics->drawText(part.text, part.x + 1, part.y);
                graphics->drawText(part.text, part.x - 1, part.y);
                graphics->drawText(part.text, part.x, part.y + 1);
                graphics->drawText(part.text, part.x, part.y - 1);
            }

            // the main text
            graphics->setColor(part.color);
            graphics->drawText(part.text, part.x, part.y);
        }
    }
}

/**
 * Relayouts all text rows and returns the new height of the BrowserBox.
 */
void BrowserBox::relayoutText()
{
    LayoutContext context(getFont());

    for (auto &row : mTextRows)
        layoutTextRow(row, context);

    mLastLayoutWidth = getWidth();
    mLastLayoutTime = tick_time;
    setHeight(context.y);
}

/**
 * Layers out the given \a row of text starting at the given \a context position.
 * @return the context position for the next row.
 */
void BrowserBox::layoutTextRow(TextRow &row, LayoutContext &context)
{
    const int startY = context.y;
    row.parts.clear();

    unsigned linkIndex = 0;
    bool wrapped = false;
    int x = 0;

    // Check for separator lines
    if (row.text.find("---", 0) == 0)
    {
        for (x = 0; x < getWidth(); x += context.minusWidth - 1)
        {
            row.parts.push_back(LinePart { x, context.y, context.selColor, "-" });
        }

        context.y += row.height;

        row.width = getWidth();
        row.height = context.y - startY;
        return;
    }

    gcn::Color prevColor = context.selColor;

    // TODO: Check if we must take texture size limits into account here
    // TODO: Check if some of the O(n) calls can be removed
    for (std::string::size_type start = 0, end = std::string::npos;
            start != std::string::npos;
            start = end, end = std::string::npos)
    {
        // Wrapped line continuation shall be indented
        if (wrapped)
        {
            context.y += context.lineHeight;
            x = 15;
            wrapped = false;
        }

        // "Tokenize" the string at control sequences
        if (mUseLinksAndUserColors)
            end = row.text.find("##", start + 1);

        if (mUseLinksAndUserColors ||
            (!mUseLinksAndUserColors && (start == 0)))
        {
            // Check for color change in format "##x", x = [L,P,0..9]
            if (row.text.find("##", start) == start && row.text.size() > start + 2)
            {
                const char c = row.text.at(start + 2);

                bool valid;
                const gcn::Color col = Theme::getThemeColor(c, valid);

                if (c == '>')
                {
                    context.selColor = prevColor;
                }
                else if (c == '<')
                {
                    prevColor = context.selColor;
                    context.selColor = col;
                }
                else if (valid)
                {
                    context.selColor = col;
                }
                else
                {
                    switch (c)
                    {
                        case '1': context.selColor = RED; break;
                        case '2': context.selColor = GREEN; break;
                        case '3': context.selColor = BLUE; break;
                        case '4': context.selColor = ORANGE; break;
                        case '5': context.selColor = YELLOW; break;
                        case '6': context.selColor = PINK; break;
                        case '7': context.selColor = PURPLE; break;
                        case '8': context.selColor = GRAY; break;
                        case '9': context.selColor = BROWN; break;
                        case '0':
                        default:
                            context.selColor = context.textColor;
                    }
                }

                // Update the position of the links
                if (c == '<' && linkIndex < row.links.size())
                {
                    auto &link = row.links[linkIndex];
                    const int size = context.font->getWidth(link.caption) + 1;

                    link.x1 = x;
                    link.y1 = context.y;
                    link.x2 = link.x1 + size;
                    link.y2 = context.y + context.fontHeight - 1;

                    linkIndex++;
                }
                start += 3;

                if (start == row.text.size())
                    break;
            }
        }

        if (start >= row.text.length())
            break;

        std::string::size_type len =
            end == std::string::npos ? end : end - start;

        std::string part = row.text.substr(start, len);

        // Auto wrap mode
        if (mMode == AUTO_WRAP && getWidth() > 0
            && context.font->getWidth(part) > 0
            && (x + context.font->getWidth(part) + 10) > getWidth())
        {
            bool forced = false;

            /* FIXME: This code layout makes it easy to crash remote
               clients by talking garbage. Forged long utf-8 characters
               will cause either a buffer underflow in substr or an
               infinite loop in the main loop. */
            do
            {
                if (!forced)
                    end = row.text.rfind(' ', end);

                // Check if we have to (stupidly) force-wrap
                if (end == std::string::npos || end <= start)
                {
                    forced = true;
                    end = row.text.size();
                    x += context.tildeWidth; // Account for the wrap-notifier
                    continue;
                }

                // Skip to the start of the current character
                while ((row.text[end] & 192) == 128)
                    end--;
                end--; // And then to the last byte of the previous one

                part = row.text.substr(start, end - start + 1);
            }
            while (end > start && context.font->getWidth(part) > 0
                   && (x + context.font->getWidth(part) + 10) > getWidth());

            if (forced)
            {
                x -= context.tildeWidth; // Remove the wrap-notifier accounting
                row.parts.push_back(LinePart { getWidth() - context.tildeWidth,
                                               context.y, context.selColor, "~" });
                end++; // Skip to the next character
            }
            else
            {
                end += 2; // Skip to after the space
            }

            wrapped = true;
        }

        row.parts.push_back(LinePart { x, context.y, context.selColor, part });

        const int partWidth = context.font->getWidth(part);
        row.width = std::max(row.width, x + partWidth);

        if (mMode == AUTO_WRAP && partWidth == 0)
            break;

        x += partWidth;
    }

    context.y += context.lineHeight;
    row.height = context.y - startY;
}

void BrowserBox::updateHoveredLink(int x, int y)
{
    mHoveredLink.reset();

    for (const auto &row : mTextRows)
    {
        for (const auto &link : row.links)
        {
            if (link.contains(x, y))
            {
                mHoveredLink = link;
                return;
            }
        }
    }
}

void BrowserBox::maybeRelayoutText()
{
    // Reduce relayouting frequency when there is a lot of text
    if (mTextRows.size() > 100)
        if (mLastLayoutTime && std::abs(mLastLayoutTime - tick_time) < 10)
            return;

    relayoutText();
}
