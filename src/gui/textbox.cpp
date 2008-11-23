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

#include "textbox.h"

#include <sstream>

#include <guichan/basiccontainer.hpp>
#include <guichan/font.hpp>

TextBox::TextBox():
    gcn::TextBox()
{
    setOpaque(false);
    setFrameSize(0);
}

void TextBox::setTextWrapped(const std::string &text)
{
    // Make sure parent scroll area sets width of this widget
    if (getParent())
    {
        getParent()->logic();
    }

    std::stringstream wrappedStream;
    std::string::size_type newlinePos, lastNewlinePos = 0;

    do
    {
        // Determine next piece of string to wrap
        newlinePos = text.find("\n", lastNewlinePos);

        if (newlinePos == std::string::npos)
        {
            newlinePos = text.size();
        }

        std::string line =
            text.substr(lastNewlinePos, newlinePos - lastNewlinePos);
        std::string::size_type spacePos, lastSpacePos = 0;
        int xpos = 0;

        do
        {
            spacePos = line.find(" ", lastSpacePos);

            if (spacePos == std::string::npos)
            {
                spacePos = line.size();
            }

            std::string word =
                line.substr(lastSpacePos, spacePos - lastSpacePos);

            int width = getFont()->getWidth(word);

            if (xpos != 0 && xpos + width < getWidth())
            {
                xpos += width + getFont()->getWidth(" ");
                wrappedStream << " " << word;
            }
            else if (lastSpacePos == 0)
            {
                xpos += width;
                wrappedStream << word;
            }
            else
            {
                xpos = width;
                wrappedStream << "\n" << word;
            }

            lastSpacePos = spacePos + 1;
        }
        while (spacePos != line.size());

        if (text.find("\n", lastNewlinePos) != std::string::npos)
        {
            wrappedStream << "\n";
        }

        lastNewlinePos = newlinePos + 1;
    }
    while (newlinePos != text.size());

    gcn::TextBox::setText(wrappedStream.str());
}
