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

#include "textfield.h"
#include "../resources/resourcemanager.h"

int TextField::instances = 0;
ImageRect TextField::skin;

TextField::TextField(const std::string& text):
    gcn::TextField(text)
{
    setBorderSize(2);

    if (instances == 0)
    {
        // Load the skin
        ResourceManager *resman = ResourceManager::getInstance();
        Image *textbox = resman->getImage("graphics/gui/deepbox.png");
        int gridx[4] = {0, 3, 28, 31};
        int gridy[4] = {0, 3, 28, 31};
        //Image *textbox = resman->getImage("graphics/gui/textbox.png");
        //int gridx[4] = {0, 5, 26, 31};
        //int gridy[4] = {0, 5, 26, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++) {
            for (x = 0; x < 3; x++) {
                skin.grid[a] = textbox->getSubImage(
                        gridx[x], gridy[y],
                        gridx[x + 1] - gridx[x] + 1,
                        gridy[y + 1] - gridy[y] + 1);
                a++;
            }
        }

        textbox->decRef();
    }

    instances++;
}

TextField::~TextField()
{
    instances--;

    if (instances == 0)
    {
        for (int a = 0; a < 9; a++) {
            delete skin.grid[a];
        }
    }
}

void TextField::draw(gcn::Graphics *graphics)
{
    int x, y, w, h;
    getAbsolutePosition(x, y);
    w = getWidth();
    h = getHeight();

    if (hasFocus()) {
        drawCaret(graphics,
                getFont()->getWidth(mText.substr(0, mCaretPosition)) -
                mXScroll);
    }

    graphics->setColor(getForegroundColor());
    graphics->setFont(getFont());
    graphics->drawText(mText, 1 - mXScroll, 1);
}

void TextField::drawBorder(gcn::Graphics *graphics)
{
    int x, y, w, h, bs;
    getAbsolutePosition(x, y);
    bs = getBorderSize();
    w = getWidth() + bs * 2;
    h = getHeight() + bs * 2;
    x -= bs;
    y -= bs;

    ((Graphics*)graphics)->drawImageRect(x, y, w, h, skin);
}
