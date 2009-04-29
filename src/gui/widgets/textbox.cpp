/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gui/widgets/textbox.h"

#include "gui/palette.h"

#include <guichan/font.hpp>

#include <sstream>

TextBox::TextBox() :
    mTextColor(&guiPalette->getColor(Palette::TEXT))
{
    setOpaque(false);
    setFrameSize(0);
    mMinWidth = getWidth();
}

void TextBox::setTextWrapped(const std::string &text, int minDimension)
{
    // Make sure parent scroll area sets width of this widget
    if (getParent())
        getParent()->logic();

    // Take the supplied minimum dimension as a starting point and try to beat it
    mMinWidth = minDimension;

    std::stringstream wrappedStream;
    std::string::size_type spacePos, newlinePos, lastNewlinePos = 0;
    int minWidth = 0;
    int xpos;

    spacePos = text.rfind(" ", text.size());

    if (spacePos != std::string::npos)
    {
        const std::string word = text.substr(spacePos + 1);
        const int length = getFont()->getWidth(word);

        if (length > mMinWidth)
            mMinWidth = length;
    }

    do
    {
        // Determine next piece of string to wrap
        newlinePos = text.find("\n", lastNewlinePos);

        if (newlinePos == std::string::npos)
            newlinePos = text.size();

        std::string line =
            text.substr(lastNewlinePos, newlinePos - lastNewlinePos);
        std::string::size_type lastSpacePos = 0;
        xpos = 0;

        do
        {
            spacePos = line.find(" ", lastSpacePos);

            if (spacePos == std::string::npos)
                spacePos = line.size();

            std::string word =
                line.substr(lastSpacePos, spacePos - lastSpacePos);

            int width = getFont()->getWidth(word);

            if (xpos == 0 && width > mMinWidth)
            {
                mMinWidth = width;
                xpos = width;
                wrappedStream << word;
            }
            else if (xpos != 0 && xpos + getFont()->getWidth(" ") + width <= 
                     mMinWidth)
            {
                xpos += getFont()->getWidth(" ") + width;
                wrappedStream << " " << word;
            }
            else if (lastSpacePos == 0)
            {
                xpos += width;
                wrappedStream << word;
            }
            else
            {
                if (xpos > minWidth)
                    minWidth = xpos;

                // The window wasn't big enough. Resize it and try again.
                if (minWidth > mMinWidth)
                {
                    mMinWidth = minWidth;
                    wrappedStream.clear();
                    wrappedStream.str("");
                    spacePos = 0;
                    lastNewlinePos = 0;
                    newlinePos = text.find("\n", lastNewlinePos);
                    if (newlinePos == std::string::npos)
                        newlinePos = text.size();
                    line = text.substr(lastNewlinePos, newlinePos -
                                       lastNewlinePos);
                    width = 0;
                    break;
                }
                else
                {
                    wrappedStream << "\n" << word;
                }
                xpos = width;
            }
            lastSpacePos = spacePos + 1;
        }
        while (spacePos != line.size());

        if (text.find("\n", lastNewlinePos) != std::string::npos)
            wrappedStream << "\n";

        lastNewlinePos = newlinePos + 1;
    }
    while (newlinePos != text.size());

    if (xpos > minWidth)
        minWidth = xpos;

    mMinWidth = minWidth;

    gcn::TextBox::setText(wrappedStream.str());
}
