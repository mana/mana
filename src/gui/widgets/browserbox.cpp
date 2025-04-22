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

#include "keyboardconfig.h"
#include "textrenderer.h"

#include "gui/gui.h"
#include "gui/truetypefont.h"
#include "gui/widgets/linkhandler.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"
#include "resources/theme.h"

#include "utils/stringutils.h"

#include <guichan/graphics.hpp>
#include <guichan/font.hpp>
#include <guichan/cliprectangle.hpp>

#include <algorithm>

/**
 * Check for key replacements in format "###key;"
 */
static void replaceKeys(std::string &text)
{
    auto keyStart = text.find("###");

    while (keyStart != std::string::npos)
    {
        const auto keyEnd = text.find(";", keyStart + 3);
        if (keyEnd == std::string::npos)
            break;

        std::string_view key(text.data() + keyStart + 3, keyEnd - keyStart - 3);

        // Remove "key" prefix
        if (key.size() > 3 && key.substr(0, 3) == "key")
            key.remove_prefix(3);

        const auto keyName = keyboard.getKeyName(key);
        if (!keyName.empty())
        {
            text.replace(keyStart, keyEnd - keyStart + 1, keyName);
            keyStart = text.find("###", keyStart + keyName.size());
        }
        else
        {
            keyStart = text.find("###", keyEnd + 1);
        }
    }
}

struct LayoutContext
{
    LayoutContext(gcn::Font *font, const Palette &palette);

    int y = 0;
    gcn::Font *font;
    const int fontHeight;
    const int minusWidth;
    const int tildeWidth;
    int lineHeight;
    const gcn::Color textColor;
    gcn::Color selColor;
};

LayoutContext::LayoutContext(gcn::Font *font, const Palette &palette)
    : font(font)
    , fontHeight(font->getHeight())
    , minusWidth(font->getWidth("-"))
    , tildeWidth(font->getWidth("~"))
    , lineHeight(fontHeight)
    , textColor(palette.getColor(Theme::TEXT))
    , selColor(textColor)
{
    if (auto *trueTypeFont = dynamic_cast<const TrueTypeFont*>(font))
        lineHeight = trueTypeFont->getLineHeight();
}


BrowserBox::BrowserBox(Mode mode):
    mMode(mode)
{
    setFocusable(true);
    addMouseListener(this);
}

BrowserBox::~BrowserBox() = default;

void BrowserBox::addRows(std::string_view rows)
{
    std::string_view::size_type start = 0;
    std::string_view::size_type end = 0;
    while (end != std::string::npos)
    {
        end = rows.find('\n', start);
        addRow(rows.substr(start, end - start));
        start = end + 1;
    }
}

void BrowserBox::addRow(std::string_view row)
{
    TextRow &newRow = mTextRows.emplace_back();

    // Use links and user defined colors
    if (mUseLinksAndUserColors)
    {
        // Check for links in format "@@link|Caption@@"
        auto linkStart = row.find("@@");
        while (linkStart != std::string::npos)
        {
            const auto linkSep = row.find("|", linkStart);
            const auto linkEnd = row.find("@@", linkSep);

            if (linkSep == std::string::npos || linkEnd == std::string::npos)
                break;

            BrowserLink &link = newRow.links.emplace_back();
            link.link = row.substr(linkStart + 2, linkSep - (linkStart + 2));
            link.caption = row.substr(linkSep + 1, linkEnd - (linkSep + 1));

            if (link.caption.empty())
            {
                const int id = atoi(link.link.c_str());
                if (id)
                    link.caption = itemDb->get(id).name;
                else
                    link.caption = link.link;
            }

            newRow.text += row.substr(0, linkStart);
            newRow.text += "##<" + link.caption;

            row = row.substr(linkEnd + 2);
            if (!row.empty())
            {
                newRow.text += "##>";
            }
            linkStart = row.find("@@");
        }

        newRow.text += row;
    }
    // Don't use links and user defined colors
    else
    {
        newRow.text = row;
    }

    if (mEnableKeys)
        replaceKeys(newRow.text);

    // Layout the newly added row
    LayoutContext context(getFont(), gui->getTheme()->getPalette(mPalette));
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
                part.y -= removedHeight;

            for (auto &link : row.links)
                link.rect.y -= removedHeight;
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

    if (mHoveredLink) {
        mLinkHandler->handleLink(mHoveredLink->link);
        gui->setCursorType(Cursor::Pointer);
    }
}

void BrowserBox::mouseMoved(gcn::MouseEvent &event)
{
    updateHoveredLink(event.getX(), event.getY());
    gui->setCursorType(mHoveredLink ? Cursor::Hand : Cursor::Pointer);
    event.consume();        // Suppress mouse cursor change by parent
}

void BrowserBox::mouseExited(gcn::MouseEvent &event)
{
    mHoveredLink.reset();
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
        auto &palette = gui->getTheme()->getPalette(mPalette);
        auto &link = *mHoveredLink;

        const gcn::Rectangle &rect = link.rect;

        if (mHighlightMode & BACKGROUND)
        {
            graphics->setColor(palette.getColor(Theme::HIGHLIGHT));
            graphics->fillRectangle(rect);
        }

        if (mHighlightMode & UNDERLINE)
        {
            graphics->setColor(palette.getColor(Theme::HYPERLINK));
            graphics->drawLine(rect.x,
                               rect.y + rect.height,
                               rect.x + rect.width,
                               rect.y + rect.height);
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

            TextRenderer::renderText(graphics,
                                     part.text,
                                     part.x,
                                     part.y,
                                     Graphics::LEFT,
                                     part.color,
                                     part.font,
                                     mOutline,
                                     mShadows);
        }
    }
}

/**
 * Relayouts all text rows and returns the new height of the BrowserBox.
 */
void BrowserBox::relayoutText()
{
    LayoutContext context(getFont(), gui->getTheme()->getPalette(mPalette));

    for (auto &row : mTextRows)
        layoutTextRow(row, context);

    mLastLayoutWidth = getWidth();
    mLayoutTimer.set(33);
    setHeight(context.y);
}

/**
 * Layers out the given \a row of text starting at the given \a context position.
 * @return the context position for the next row.
 */
void BrowserBox::layoutTextRow(TextRow &row, LayoutContext &context)
{
    // each line starts with normal font in default color
    context.font = getFont();
    context.selColor = context.textColor;

    const int startY = context.y;
    row.parts.clear();

    unsigned linkIndex = 0;
    bool wrapped = false;
    int x = 0;

    // Check for separator lines
    if (startsWith(row.text, "---"))
    {
        for (x = 0; x < getWidth(); x += context.minusWidth - 1)
        {
            row.parts.push_back(LinePart {
                                    x,
                                    context.y,
                                    context.selColor,
                                    "-",
                                    context.font
                                });
        }

        context.y += row.height;

        row.width = getWidth();
        row.height = context.y - startY;
        return;
    }

    auto theme = gui->getTheme();
    auto &palette = gui->getTheme()->getPalette(mPalette);
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
            x = mWrapIndent;
            wrapped = false;
        }

        if (mUseLinksAndUserColors || start == 0)
        {
            // Check for color or font change in format "##x", x = [<,>,B,p,0..9]
            while (row.text.size() > start + 2 && row.text.find("##", start) == start)
            {
                const char c = row.text.at(start + 2);
                start += 3;

                switch (c)
                {
                    case '>':
                        context.selColor = prevColor;
                        break;
                    case '<':
                        prevColor = context.selColor;
                        context.selColor = palette.getColor(Theme::HYPERLINK);
                        break;
                    case 'B':
                        context.font = boldFont;
                        break;
                    case 'b':
                        context.font = getFont();
                        break;
                    default: {
                        const auto colorId = Theme::getColorIdForChar(c);
                        context.selColor = colorId ? palette.getColor(*colorId)
                                                   : context.textColor;
                        break;
                    }
                }

                // Update the position of the links
                if (c == '<' && linkIndex < row.links.size())
                {
                    auto &link = row.links[linkIndex];

                    link.rect.x = x;
                    link.rect.y = context.y;
                    link.rect.width = context.font->getWidth(link.caption) + 1;
                    link.rect.height = context.fontHeight - 1;

                    linkIndex++;
                }
            }
        }

        if (start >= row.text.length())
            break;

        // "Tokenize" the string at control sequences
        if (mUseLinksAndUserColors)
            end = row.text.find("##", start + 1);

        std::string::size_type len =
            end == std::string::npos ? end : end - start;

        std::string part = row.text.substr(start, len);
        int partWidth = context.font->getWidth(part);

        // Auto wrap mode
        if (mMode == AUTO_WRAP && getWidth() > 0
            && partWidth > 0
            && (x + partWidth) > getWidth())
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
                partWidth = context.font->getWidth(part);
            }
            while (end > start && partWidth > 0
                   && (x + partWidth) > getWidth());

            if (forced)
            {
                x -= context.tildeWidth; // Remove the wrap-notifier accounting
                row.parts.push_back(LinePart {
                                        getWidth() - context.tildeWidth,
                                        context.y,
                                        context.selColor,
                                        "~",
                                        getFont()
                                    });
                end++; // Skip to the next character
            }
            else
            {
                end += 2; // Skip to after the space
            }

            wrapped = true;
        }

        row.parts.push_back(LinePart {
                                x,
                                context.y,
                                context.selColor,
                                std::move(part),
                                context.font
                            });

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
    if (mTextRows.size() > 1000)
        if (!mLayoutTimer.passed())
            return;

    relayoutText();
}
