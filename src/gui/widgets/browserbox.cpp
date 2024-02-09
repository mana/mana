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

BrowserBox::BrowserBox(unsigned int mode, bool opaque):
    mMode(mode),
    mOpaque(opaque)
{
    setFocusable(true);
    addMouseListener(this);
}

BrowserBox::~BrowserBox()
{
}

void BrowserBox::setLinkHandler(LinkHandler *linkHandler)
{
    mLinkHandler = linkHandler;
}

void BrowserBox::setOpaque(bool opaque)
{
    mOpaque = opaque;
}

void BrowserBox::setHighlightMode(unsigned int highMode)
{
    mHighMode = highMode;
}

void BrowserBox::disableLinksAndUserColors()
{
    mUseLinksAndUserColors = false;
}

void BrowserBox::addRow(const std::string &row)
{
    std::string newRow;

    gcn::Font *font = getFont();
    const int fontHeight = font->getHeight();

    int lineHeight = fontHeight;
    if (auto *ttf = dynamic_cast<TrueTypeFont*>(font))
        lineHeight = ttf->getLineHeight();

    // Use links and user defined colors
    if (mUseLinksAndUserColors)
    {
        BrowserLink bLink;
        std::string tmp = row;
        std::string::size_type idx1, idx2, idx3;

        // Check for links in format "@@link|Caption@@"
        idx1 = tmp.find("@@");
        while (idx1 != std::string::npos)
        {
            idx2 = tmp.find("|", idx1);
            idx3 = tmp.find("@@", idx2);

            if (idx2 == std::string::npos || idx3 == std::string::npos)
                break;
            bLink.link = tmp.substr(idx1 + 2, idx2 - (idx1 + 2));
            bLink.caption = tmp.substr(idx2 + 1, idx3 - (idx2 + 1));
            bLink.y1 = static_cast<int>(mTextRows.size()) * lineHeight;
            bLink.y2 = bLink.y1 + fontHeight;

            newRow += tmp.substr(0, idx1);

            std::string tmp2 = newRow;
            idx1 = tmp2.find("##");
            while (idx1 != std::string::npos)
            {
                tmp2.erase(idx1, 3);
                idx1 = tmp2.find("##");
            }
            bLink.x1 = font->getWidth(tmp2) - 1;
            bLink.x2 = bLink.x1 + font->getWidth(bLink.caption) + 1;

            mLinks.push_back(bLink);

            newRow += "##<" + bLink.caption;

            tmp.erase(0, idx3 + 2);
            if (!tmp.empty())
            {
                newRow += "##>";
            }
            idx1 = tmp.find("@@");
        }

        newRow += tmp;
    }
    // Don't use links and user defined colors
    else
    {
        newRow = row;
    }

    mTextRows.push_back(newRow);

    //discard older rows when a row limit has been set
    if (mMaxRows > 0)
    {
        while (mTextRows.size() > mMaxRows)
        {
            mTextRows.pop_front();
            for (unsigned int i = 0; i < mLinks.size(); i++)
            {
                mLinks[i].y1 -= lineHeight;
                mLinks[i].y2 -= lineHeight;

                if (mLinks[i].y1 < 0)
                    mLinks.erase(mLinks.begin() + i);
            }
        }
    }

    // Auto size mode
    if (mMode == AUTO_SIZE)
    {
        std::string plain = newRow;
        std::string::size_type index;
        while ((index = plain.find("##")) != std::string::npos)
            plain.erase(index, 3);

        // Adjust the BrowserBox size
        int w = font->getWidth(plain);
        if (w > getWidth())
            setWidth(w);
    }

    // TODO: Optimize! There's no point in wrapping all text rows again, just
    // do the one that was just added (but take into account discarded rows?).
    if (mMode == AUTO_WRAP)
    {
        unsigned int wrapCount = 0;
        unsigned int nextChar;
        const char *tilde = "~";
        int tildeWidth = font->getWidth(tilde);
        int x = 0;

        for (auto row : mTextRows)
        {
            for (unsigned int j = 0; j < row.size(); ++j)
            {
                std::string character = row.substr(j, 1);
                x += font->getWidth(character);
                nextChar = j + 1;

                // Wraping between words (at blank spaces)
                if (nextChar < row.size() && row.at(nextChar) == ' ')
                {
                    int nextSpacePos = static_cast<int>(
                        row.find(" ", (nextChar + 1)));
                    if (nextSpacePos <= 0)
                        nextSpacePos = static_cast<int>(row.size()) - 1;

                    int nextWordWidth = font->getWidth(
                            row.substr(nextChar,
                                (nextSpacePos - nextChar)));

                    if (x + nextWordWidth + 10 > getWidth())
                    {
                        x = 15; // Indent in new line
                        ++wrapCount;
                        ++j;
                    }
                }
                // Wrapping looong lines (brutal force)
                else if (x + 2 * tildeWidth > getWidth())
                {
                    x = 15; // Ident in new line
                    ++wrapCount;
                }
            }
        }

        setHeight(lineHeight * (mTextRows.size() + wrapCount - 1)
                  + fontHeight);
    }
    else
    {
        setHeight(lineHeight * (mTextRows.size() - 1) + fontHeight);
    }
    mUpdateTime = 0;
    updateHeight();
}

void BrowserBox::clearRows()
{
    mTextRows.clear();
    mLinks.clear();
    setWidth(0);
    setHeight(0);
    mSelectedLink = -1;
    updateHeight();
}

struct MouseOverLink
{
    MouseOverLink(int x, int y)
        : mX(x), mY(y)
    {}

    bool operator() (BrowserLink &link) const
    {
        return (mX >= link.x1 && mX < link.x2 &&
                mY >= link.y1 && mY < link.y2);
    }

    int mX, mY;
};

void BrowserBox::mousePressed(gcn::MouseEvent &event)
{
    if (!mLinkHandler)
        return;

    auto i = find_if(mLinks.begin(), mLinks.end(),
            MouseOverLink(event.getX(), event.getY()));

    if (i != mLinks.end())
        mLinkHandler->handleLink(i->link);
}

void BrowserBox::mouseMoved(gcn::MouseEvent &event)
{
    auto i = find_if(mLinks.begin(), mLinks.end(),
            MouseOverLink(event.getX(), event.getY()));

    mSelectedLink = (i != mLinks.end())
        ? static_cast<int>(i - mLinks.begin()) : -1;
}

void BrowserBox::draw(gcn::Graphics *graphics)
{
    gcn::ClipRectangle cr = graphics->getCurrentClipArea();
    mYStart = cr.y - cr.yOffset;
    int yEnd = mYStart + cr.height;
    if (mYStart < 0)
        mYStart = 0;

    if (getWidth() != mWidth)
        updateHeight();

    if (mOpaque)
    {
        graphics->setColor(Theme::getThemeColor(Theme::BACKGROUND));
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    }

    if (mSelectedLink >= 0 && (unsigned) mSelectedLink < mLinks.size())
    {
        if ((mHighMode & BACKGROUND))
        {
            graphics->setColor(Theme::getThemeColor(Theme::HIGHLIGHT));
            graphics->fillRectangle(gcn::Rectangle(
                        mLinks[mSelectedLink].x1,
                        mLinks[mSelectedLink].y1,
                        mLinks[mSelectedLink].x2 - mLinks[mSelectedLink].x1,
                        mLinks[mSelectedLink].y2 - mLinks[mSelectedLink].y1
                        ));
        }

        if ((mHighMode & UNDERLINE))
        {
            graphics->setColor(Theme::getThemeColor(Theme::HYPERLINK));
            graphics->drawLine(
                    mLinks[mSelectedLink].x1,
                    mLinks[mSelectedLink].y2,
                    mLinks[mSelectedLink].x2,
                    mLinks[mSelectedLink].y2);
        }
    }

    for (auto &part : mLineParts)
    {
        if (part.getY() + 50 < mYStart)
            continue;
        if (part.getY() > yEnd)
            break;

        // Use the correct font
        graphics->setFont(getFont());

        // Handle text shadows
        if (mShadows)
        {
            graphics->setColor(Theme::getThemeColor(Theme::SHADOW,
                                                    part.getColor().a / 2));
            if (mOutline)
            {
                graphics->drawText(part.getText(), part.getX() + 2,
                                   part.getY() + 2);
            }
            else
            {
                graphics->drawText(part.getText(), part.getX() + 1,
                                   part.getY() + 1);
            }
        }

        if (mOutline)
        {
            // Text outline
            graphics->setColor(Theme::getThemeColor(Theme::OUTLINE,
                                                    part.getColor().a / 4));
            graphics->drawText(part.getText(), part.getX() + 1, part.getY());
            graphics->drawText(part.getText(), part.getX() - 1, part.getY());
            graphics->drawText(part.getText(), part.getX(), part.getY() + 1);
            graphics->drawText(part.getText(), part.getX(), part.getY() - 1);
        }

        // the main text
        graphics->setColor(part.getColor());
        graphics->drawText(part.getText(), part.getX(), part.getY());
    }

    return;
}

int BrowserBox::calcHeight()
{
    int x = 0, y = 0;
    int wrappedLines = 0;
    unsigned link = 0;
    gcn::Font *font = getFont();

    const int fontHeight = font->getHeight();
    const int minusWidth = font->getWidth("-");
    const int tildeWidth = font->getWidth("~");

    int lineHeight = fontHeight;
    if (auto *ttf = dynamic_cast<TrueTypeFont*>(font))
        lineHeight = ttf->getLineHeight();

    gcn::Color selColor = Theme::getThemeColor(Theme::TEXT);
    const gcn::Color &textColor = Theme::getThemeColor(Theme::TEXT);

    mLineParts.clear();

    for (auto row : mTextRows)
    {
        bool wrapped = false;
        x = 0;

        // Check for separator lines
        if (row.find("---", 0) == 0)
        {
            for (x = 0; x < getWidth(); x++)
            {
                mLineParts.push_back(LinePart(x, y, selColor, "-"));
                x += minusWidth - 2;
            }

            y += lineHeight;
            continue;
        }

        gcn::Color prevColor = selColor;

        // TODO: Check if we must take texture size limits into account here
        // TODO: Check if some of the O(n) calls can be removed
        for (std::string::size_type start = 0, end = std::string::npos;
                start != std::string::npos;
                start = end, end = std::string::npos)
        {
            // Wrapped line continuation shall be indented
            if (wrapped)
            {
                y += lineHeight;
                x = 15;
                wrapped = false;
            }

            // "Tokenize" the string at control sequences
            if (mUseLinksAndUserColors)
                end = row.find("##", start + 1);

            if (mUseLinksAndUserColors ||
                (!mUseLinksAndUserColors && (start == 0)))
            {
                // Check for color change in format "##x", x = [L,P,0..9]
                if (row.find("##", start) == start && row.size() > start + 2)
                {
                    const char c = row.at(start + 2);

                    bool valid;
                    const gcn::Color col = Theme::getThemeColor(c, valid);

                    if (c == '>')
                    {
                        selColor = prevColor;
                    }
                    else if (c == '<')
                    {
                        prevColor = selColor;
                        selColor = col;
                    }
                    else if (valid)
                    {
                        selColor = col;
                    }
                    else
                    {

                        switch (c)
                        {
                            case '1': selColor = RED; break;
                            case '2': selColor = GREEN; break;
                            case '3': selColor = BLUE; break;
                            case '4': selColor = ORANGE; break;
                            case '5': selColor = YELLOW; break;
                            case '6': selColor = PINK; break;
                            case '7': selColor = PURPLE; break;
                            case '8': selColor = GRAY; break;
                            case '9': selColor = BROWN; break;
                            case '0':
                            default:
                                selColor = textColor;
                        }
                    }

                    if (c == '<' && link < mLinks.size())
                    {
                        const int size =
                            font->getWidth(mLinks[link].caption) + 1;

                        mLinks[link].x1 = x;
                        mLinks[link].y1 = y;
                        mLinks[link].x2 = mLinks[link].x1 + size;
                        mLinks[link].y2 = y + fontHeight - 1;
                        link++;
                    }
                    start += 3;

                    if (start == row.size())
                        break;
                }
            }

            if (start >= row.length())
                break;

            std::string::size_type len =
                end == std::string::npos ? end : end - start;

            std::string part = row.substr(start, len);

            // Auto wrap mode
            if (mMode == AUTO_WRAP && getWidth() > 0
                && font->getWidth(part) > 0
                && (x + font->getWidth(part) + 10) > getWidth())
            {
                bool forced = false;

                /* FIXME: This code layout makes it easy to crash remote
                   clients by talking garbage. Forged long utf-8 characters
                   will cause either a buffer underflow in substr or an
                   infinite loop in the main loop. */
                do
                {
                    if (!forced)
                        end = row.rfind(' ', end);

                    // Check if we have to (stupidly) force-wrap
                    if (end == std::string::npos || end <= start)
                    {
                        forced = true;
                        end = row.size();
                        x += tildeWidth; // Account for the wrap-notifier
                        continue;
                    }

                    // Skip to the start of the current character
                    while ((row[end] & 192) == 128)
                        end--;
                    end--; // And then to the last byte of the previous one

                    part = row.substr(start, end - start + 1);
                }
                while (end > start && font->getWidth(part) > 0
                       && (x + font->getWidth(part) + 10) > getWidth());

                if (forced)
                {
                    x -= tildeWidth; // Remove the wrap-notifier accounting
                    mLineParts.push_back(LinePart(getWidth() - tildeWidth,
                                                  y, selColor, "~"));
                    end++; // Skip to the next character
                }
                else
                {
                    end += 2; // Skip to after the space
                }

                wrapped = true;
                wrappedLines++;
            }

            mLineParts.push_back(LinePart(x, y, selColor, part));

            const int partWidth = font->getWidth(part);
            if (mMode == AUTO_WRAP && partWidth == 0)
                break;

            x += partWidth;
        }
        y += lineHeight;
    }
    return (mTextRows.size() + wrappedLines - 1) * lineHeight + fontHeight;
}

void BrowserBox::updateHeight()
{
    if (mAlwaysUpdate || !mUpdateTime || std::abs(mUpdateTime - tick_time) > 10
        || mTextRows.size() < 3)
    {
        mWidth = getWidth();
        mHeight = calcHeight();
        setHeight(mHeight);
        mUpdateTime = tick_time;
    }
}
