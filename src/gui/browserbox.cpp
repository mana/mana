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
 *  $Id: browserbox.cpp 4348 2008-06-14 12:42:49Z the_enemy $
 */

#include <algorithm>

#include <guichan/graphics.hpp>
#include <guichan/imagefont.hpp>
#include <guichan/mouseinput.hpp>

#include "browserbox.h"
#include "linkhandler.h"
#include "colour.h"

#ifdef USE_OPENGL
#include "../configuration.h"
#include "../resources/resourcemanager.h"

int BrowserBox::instances = 0;
gcn::ImageFont* BrowserBox::browserFont = NULL;
#endif

BrowserBox::BrowserBox(unsigned int mode):
    gcn::Widget(),
    mMode(mode), mHighMode(UNDERLINE | BACKGROUND),
    mOpaque(true),
    mUseLinksAndUserColors(true),
    mSelectedLink(-1),
    mMaxRows(0)
{
    setFocusable(true);
    addMouseListener(this);

#ifdef USE_OPENGL
    if (config.getValue("opengl", 0.0f)) {
        if (instances == 0) {
            browserFont = new gcn::ImageFont(
                    "graphics/gui/browserfont.png",
                    " abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567"
                    "89:@!\"$%&/=?^+*#[]{}()<>_;'.,\\|-~`"
                    "¯Â·¡È…ÌÕÛ”˙⁄ÁÎ•£¢°ø‡„ıÍÒ—ˆ¸‰÷‹ƒﬂË»≈");
        }
        setFont(browserFont);
        instances++;
    }
#endif
}

BrowserBox::~BrowserBox()
{
#ifdef USE_OPENGL
    instances--;

    if (instances == 0)
    {
        // Clean up static resource font
        delete browserFont;
        browserFont = NULL;
    }
#endif
}

void BrowserBox::setLinkHandler(LinkHandler* linkHandler)
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
    std::string tmp = row;
    std::string newRow;
    BROWSER_LINK bLink;
    int idx1, idx2, idx3;
    gcn::ImageFont *font = dynamic_cast<gcn::ImageFont*>(getFont());

    // Use links and user defined colors
    if (mUseLinksAndUserColors)
    {
        // Check for links in format "@@link|Caption@@"
        idx1 = tmp.find("@@");
        while (idx1 >= 0)
        {
            idx2 = tmp.find("|", idx1);
            idx3 = tmp.find("@@", idx2);
            bLink.link = tmp.substr(idx1 + 2, idx2 - (idx1 + 2));
            bLink.caption = tmp.substr(idx2 + 1, idx3 - (idx2 + 1));
            bLink.y1 = mTextRows.size() * font->getHeight();
            bLink.y2 = bLink.y1 + font->getHeight();

            newRow += tmp.substr(0, idx1);

            std::string tmp2 = newRow;
            idx1 = tmp2.find("##");
            while (idx1 >= 0)
            {
                tmp2.erase(idx1, 3);
                idx1 = tmp2.find("##");
            }
            bLink.x1 = font->getWidth(tmp2) - 1;
            bLink.x2 = bLink.x1 + font->getWidth(bLink.caption) + 1;

            mLinks.push_back(bLink);

            newRow += "##<" + bLink.caption;

            tmp.erase(0, idx3 + 2);
            if(tmp != "")
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
        while (mTextRows.size() > mMaxRows) mTextRows.pop_front();
    }

    // Auto size mode
    if (mMode == AUTO_SIZE)
    {
        std::string plain = newRow;
        for (idx1 = plain.find("##"); idx1 >= 0; idx1 = plain.find("##"))
            plain.erase(idx1, 3);

        // Adjust the BrowserBox size
        int w = font->getWidth(plain);
        if (w > getWidth())
            setWidth(w);
    }

    if (mMode == AUTO_WRAP)
    {
        unsigned int j, y = 0;
        unsigned int nextChar;
        char hyphen = '~';
        int hyphenWidth = font->getWidth(hyphen);
        int x = 0;

        for (TextRowIterator i = mTextRows.begin(); i != mTextRows.end(); i++)
        {
            std::string row = *i;
            for (j = 0; j < row.size(); j++)
            {
                x += font->getWidth(row.at(j));
                nextChar = j + 1;

                // Wraping between words (at blank spaces)
                if ((nextChar < row.size()) && (row.at(nextChar) == ' '))
                {
                    int nextSpacePos = row.find(" ", (nextChar + 1));
                    if (nextSpacePos <= 0)
                    {
                        nextSpacePos = row.size() - 1;
                    }
                    int nextWordWidth = font->getWidth(
                            row.substr(nextChar,
                                (nextSpacePos - nextChar)));

                    if ((x + nextWordWidth + 10) > getWidth())
                    {
                        x = 15; // Ident in new line
                        y += 1;
                        j++;
                    }
                }
                // Wrapping looong lines (brutal force)
                else if ((x + 2 * hyphenWidth) > getWidth())
                {
                    x = 15; // Ident in new line
                    y += 1;
                }
            }
        }

        setHeight(font->getHeight() * (mTextRows.size() + y));
    }
    else
    {
        setHeight(font->getHeight() * mTextRows.size());
    }
}

void BrowserBox::clearRows()
{
    mTextRows.clear();
    mLinks.clear();
    setWidth(0);
    mSelectedLink = -1;
}

struct MouseOverLink
{
    MouseOverLink(int x, int y) : mX(x),mY(y) { }
    bool operator() (BROWSER_LINK &link)
    {
        return (mX >= link.x1 && mX < link.x2 &&
                mY >= link.y1 && mY < link.y2);
    }
    int mX, mY;
};

void
BrowserBox::mousePressed(gcn::MouseEvent &event)
{
    LinkIterator i = find_if(mLinks.begin(), mLinks.end(),
            MouseOverLink(event.getX(), event.getY()));

    if (i != mLinks.end()) {
        mLinkHandler->handleLink(i->link);
    }
}

void
BrowserBox::mouseMoved(gcn::MouseEvent &event)
{
    LinkIterator i = find_if(mLinks.begin(), mLinks.end(),
            MouseOverLink(event.getX(), event.getY()));

    mSelectedLink = (i != mLinks.end()) ? (i - mLinks.begin()) : -1;
}

void
BrowserBox::draw(gcn::Graphics *graphics)
{
    if (mOpaque)
    {
        graphics->setColor(gcn::Color(BGCOLOR));
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    }

    if (mSelectedLink >= 0)
    {
        if ((mHighMode & BACKGROUND))
        {
            graphics->setColor(gcn::Color(HIGHLIGHT));
            graphics->fillRectangle(gcn::Rectangle(
                        mLinks[mSelectedLink].x1,
                        mLinks[mSelectedLink].y1,
                        mLinks[mSelectedLink].x2 - mLinks[mSelectedLink].x1,
                        mLinks[mSelectedLink].y2 - mLinks[mSelectedLink].y1
                        ));
        }

        if ((mHighMode & UNDERLINE))
        {
            bool valid;
            graphics->setColor(gcn::Color(textColour->getColour('<', valid)));
            graphics->drawLine(
                    mLinks[mSelectedLink].x1,
                    mLinks[mSelectedLink].y2,
                    mLinks[mSelectedLink].x2,
                    mLinks[mSelectedLink].y2);
        }
    }

    unsigned int j;
    int x = 0, y = 0;
    int wrappedLines = 0;
    gcn::ImageFont *font = dynamic_cast<gcn::ImageFont*>(getFont());

    graphics->setColor(BLACK);
    for (TextRowIterator i = mTextRows.begin(); i != mTextRows.end(); i++)
    {
        int selColor = BLACK;
        int prevColor = selColor;
        std::string row = *(i);
        x = 0;

        for (j = 0; j < row.size(); j++)
        {
            if ( (mUseLinksAndUserColors && (j + 3) <= row.size()) ||
                    (!mUseLinksAndUserColors && (j == 0)) )
            {
                // Check for color change in format "##x"
                if ((row.at(j) == '#') && (row.at(j + 1) == '#'))
                {
                    char c = row.at(j + 2);
                    if (c == '>')
                    {
                        selColor = prevColor;
                    }
                    else
                    {
                        bool valid;
                        int rgb = textColour->getColour(c, valid);
                        if (c == '<')
                        {
                            prevColor = selColor;
                        }
                        if (valid)
                        {
                            selColor = rgb;
                        }
                    }
                    j += 3;

                    if (j == row.size())
                    {
                        break;
                    }
                }
                graphics->setColor(gcn::Color(selColor));
            }

            // Check for line separators in format "---"
            if (row == "---")
            {
                for (x = 0; x < getWidth(); x++)
                {
                    font->drawGlyph(graphics, '-', x, y);
                    x += font->getWidth('-') - 2;
                }
                break;
            }
            // Draw each char
            else
            {
                font->drawGlyph(graphics, row.at(j), x, y);
                x += font->getWidth(row.at(j));

                // Auto wrap mode
                if (mMode == AUTO_WRAP)
                {
                    unsigned int nextChar = j + 1;
                    char hyphen = '~';
                    int hyphenWidth =  font->getWidth(hyphen);

                    // Wraping between words (at blank spaces)
                    if ((nextChar < row.size()) && (row.at(nextChar) == ' '))
                    {
                        int nextSpacePos = row.find(" ", (nextChar + 1));
                        if (nextSpacePos <= 0)
                        {
                            nextSpacePos = row.size() - 1;
                        }
                        int nextWordWidth = font->getWidth(
                                row.substr(nextChar,
                                    (nextSpacePos - nextChar)));

                        if ((x + nextWordWidth + 10) > getWidth())
                        {
                            x = 15; // Ident in new line
                            y += font->getHeight();
                            wrappedLines++;
                            j++;
                        }
                    }

                    // Wrapping looong lines (brutal force)
                    else if ((x + 2 * hyphenWidth) > getWidth())
                    {
                        font->drawGlyph(graphics, hyphen,
                                getWidth() - hyphenWidth, y);
                        x = 15; // Ident in new line
                        y += font->getHeight();
                        wrappedLines++;
                    }
                }
            }
        }
        y += font->getHeight();
        setHeight((mTextRows.size() + wrappedLines) * font->getHeight());
    }
}
