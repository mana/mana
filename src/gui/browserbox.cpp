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

#include <guichan/imagefont.hpp>

#include "../main.h"
#ifdef USE_OPENGL
#include "../resources/resourcemanager.h"
#endif
#include "browserbox.h"
#include "linkhandler.h"
#include "gui.h"

int BrowserBox::instances = 0;
gcn::ImageFont* BrowserBox::browserFont;

BrowserBox::BrowserBox(unsigned int mode):
    gcn::Widget()
{
    mMode = mode;
    setOpaque(true);
    setHighlightMode(BOTH);
    mUseLinksAndUserColors = true;
    mSelectedLink = -1;
    setFocusable(true);
    addMouseListener(this);

    if (instances == 0)
    {
#ifdef USE_OPENGL
        if (useOpenGL) {
            browserFont = new gcn::ImageFont(
                    ResourceManager::getInstance()->getRealPath(
                        "graphics/gui/browserfont.png"),
                    " abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567"
                    "89:@!\"$%&/=?^+*#[]{}()<>_;'.,\\|-~`");
        } else
#endif
        {
            browserFont = gui->getFont();
        }
    }

    instances++;
}

BrowserBox::~BrowserBox()
{
    instances--;

    if (instances == 0)
    {
#ifdef USE_OPENGL
        // Clean up static resource font
        delete browserFont;
#endif
    }
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

void BrowserBox::addRow(const std::string& row)
{
    std::string tmp = row;
    std::string newRow;
    BROWSER_LINK bLink;
    int idx1, idx2, idx3;

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
            bLink.y1 = mTextRows.size() * browserFont->getHeight();
            bLink.y2 = bLink.y1 + browserFont->getHeight();

            newRow += tmp.substr(0, idx1);

            std::string tmp2 = newRow;
            idx1 = tmp2.find("##");
            while (idx1 >= 0)
            {
                tmp2.erase(idx1, 3);
                idx1 = tmp2.find("##");
            }
            bLink.x1 = browserFont->getWidth(tmp2) - 1;
            bLink.x2 = bLink.x1 + browserFont->getWidth(bLink.caption) + 1;

            mLinks.push_back(bLink);

            newRow += "##L" + bLink.caption;

            tmp.erase(0, idx3 + 2);
            if(tmp != "")
            {
                newRow += "##P";
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

    newRow == (newRow == "" ? " " : newRow);
    mTextRows.push_back(newRow);

    // Auto size mode
    if (mMode == AUTO_SIZE)
    {
        std::string plain = newRow;
        for (idx1 = plain.find("##"); idx1 >= 0; idx1 = plain.find("##"))
            plain.erase(idx1, 3);

        // Adjust the BrowserBox size
        int w = browserFont->getWidth(plain);
        if (w > getWidth())
            setWidth(w);
    }
    setHeight(browserFont->getHeight() * mTextRows.size());
}

void BrowserBox::clearRows()
{
    mTextRows.clear();
    mLinks.clear();
    setWidth(0);
    mSelectedLink = -1;
}

void BrowserBox::mousePress(int mx, int my, int button)
{
    if ((button == gcn::MouseInput::LEFT) && mLinkHandler)
    {
        for (unsigned int i = 0; i < mLinks.size(); i++)
        {
            if ((mx >= mLinks[i].x1) && (mx < mLinks[i].x2) &&
                    (my > mLinks[i].y1) && (my < mLinks[i].y2))
            {
                mLinkHandler->handleLink(mLinks[i].link);
                return;
            }
        }
    }
}

void BrowserBox::mouseMotion(int mx, int my)
{
    for (unsigned int i = 0; i < mLinks.size(); i++)
    {
        if ((mx >= mLinks[i].x1) && (mx < mLinks[i].x2) &&
                (my > mLinks[i].y1) && (my < mLinks[i].y2))
        {
            mSelectedLink = (int) i;
            return;
        }
        else
        {
            mSelectedLink = -1;
        }
    }
}

void BrowserBox::draw(gcn::Graphics* graphics)
{
    if (mOpaque)
    {
        graphics->setColor(gcn::Color(BGCOLOR));
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    }

    if (mSelectedLink >= 0)
    {
        if ((mHighMode == BACKGROUND) || (mHighMode == BOTH))
        {
            graphics->setColor(gcn::Color(HIGHLIGHT));
            graphics->fillRectangle(gcn::Rectangle(
                        mLinks[mSelectedLink].x1,
                        mLinks[mSelectedLink].y1,
                        mLinks[mSelectedLink].x2 - mLinks[mSelectedLink].x1,
                        mLinks[mSelectedLink].y2 - mLinks[mSelectedLink].y1
                        ));
        }

        if ((mHighMode == UNDERLINE) || (mHighMode == BOTH))
        {
            graphics->setColor(gcn::Color(LINK));
            graphics->drawLine(
                        mLinks[mSelectedLink].x1,
                        mLinks[mSelectedLink].y2,
                        mLinks[mSelectedLink].x2,
                        mLinks[mSelectedLink].y2);
        }
    }

    unsigned int i, j;
    int x = 0, y = 0;
    int wrappedLines = 0;
    for (i = 0; i < mTextRows.size(); i++)
    {
        int selColor = BLACK;
        int prevColor = selColor;
        std::string row = mTextRows[i];
        x = 0;

        for (j = 0; j < row.size(); j++)
        {
            if (mUseLinksAndUserColors || (!mUseLinksAndUserColors && (j == 0)))
            {
                // Check for color change in format "##x", x = [L,P,0..9]
                if ((row.at(j) == '#') && (row.at(j + 1) == '#'))
                {
                    switch (row.at(j + 2))
                    {
                        case 'L': // Link color
                            prevColor = selColor;
                            selColor = LINK;
                            break;
                        case 'P': // Previous color
                            selColor = prevColor;
                            break;
                        case '1':
                            prevColor = selColor;
                            selColor = RED;
                            break;
                        case '2':
                            prevColor = selColor;
                            selColor = GREEN;
                            break;
                        case '3':
                            prevColor = selColor;
                            selColor = BLUE;
                            break;
                        case '4':
                            prevColor = selColor;
                            selColor = ORANGE;
                            break;
                        case '5':
                            prevColor = selColor;
                            selColor = YELLOW;
                            break;
                        case '6':
                            prevColor = selColor;
                            selColor = PINK;
                            break;
                        case '7':
                            prevColor = selColor;
                            selColor = PURPLE;
                            break;
                        case '8':
                            prevColor = selColor;
                            selColor = GRAY;
                            break;
                        case '9':
                            prevColor = selColor;
                            selColor = BROWN;
                            break;
                        case '0':
                        default:
                            prevColor = selColor;
                            selColor = BLACK;
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
            if ((j <= 3) && (row.at(j) == '-') && (row.at(j + 1) == '-') &&
                    (row.at(j + 2) == '-'))
            {
                for (x = 0; x < getWidth(); x++)
                {
                    browserFont->drawGlyph(graphics, '-', x, y);
                    x += browserFont->getWidth('-') - 2;
                }
                break;
            }
            // Draw each char
            else
            {
                browserFont->drawGlyph(graphics, row.at(j), x, y);
                x += browserFont->getWidth(row.at(j));

                // Auto wrap mode
                if (mMode == AUTO_WRAP)
                {
                    unsigned int nextChar = j + 1;
                    char hyphen = '~';
                    int hyphenWidth =  browserFont->getWidth(hyphen);

                    // Wraping between words (at blank spaces)
                    if ((nextChar < row.size()) && (row.at(nextChar) == ' '))
                    {
                        int nextSpacePos = row.find(" ", (nextChar + 1));
                        if (nextSpacePos <= 0)
                        {
                            nextSpacePos = row.size() - 1;
                        }
                        int nextWordWidth = browserFont->getWidth(
                                row.substr(nextChar,
                                    (nextSpacePos - nextChar)));

                        if ((x + nextWordWidth + 10) > getWidth())
                        {
                            x = 15; // Ident in new line
                            y += browserFont->getHeight();
                            wrappedLines++;
                            j++;
                        }
                    }

                    // Wrapping looong lines (brutal force)
                    else if ((x + 2 * hyphenWidth) > getWidth())
                    {
                        browserFont->drawGlyph(graphics, hyphen,
                                getWidth() - hyphenWidth, y);
                        x = 15; // Ident in new line
                        y += browserFont->getHeight();
                        wrappedLines++;
                    }
                }
            }
        }
        y += browserFont->getHeight();
        setHeight((mTextRows.size() + wrappedLines) * browserFont->getHeight());
    }
}
