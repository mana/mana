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

#include <algorithm>

#include "dropdown.h"

#include "../../graphics.h"

#include "../../resources/image.h"
#include "../../resources/resourcemanager.h"

#include "../../utils/dtor.h"

int DropDown::instances = 0;
Image *DropDown::buttons[2][2];
ImageRect DropDown::skin;

DropDown::DropDown(gcn::ListModel *listModel,
                   gcn::ScrollArea *scrollArea,
                   gcn::ListBox *listBox):
                   gcn::DropDown::DropDown(listModel,
                   scrollArea, listBox)
{
    setFrameSize(2);

    // Initialize graphics
    if (instances == 0)
    {
        // Load the background skin
        ResourceManager *resman = ResourceManager::getInstance();

        // Get the button skin
        buttons[1][0] =
            resman->getImage("graphics/gui/vscroll_up_default.png");
        buttons[0][0] =
            resman->getImage("graphics/gui/vscroll_down_default.png");
        buttons[1][1] =
            resman->getImage("graphics/gui/vscroll_up_pressed.png");
        buttons[0][1] =
            resman->getImage("graphics/gui/vscroll_down_pressed.png");

        // get the border skin
        Image *boxBorder = resman->getImage("graphics/gui/deepbox.png");
        int gridx[4] = {0, 3, 28, 31};
        int gridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++) {
            for (x = 0; x < 3; x++) {
                skin.grid[a] = boxBorder->getSubImage(
                        gridx[x], gridy[y],
                        gridx[x + 1] - gridx[x] + 1,
                        gridy[y + 1] - gridy[y] + 1);
                a++;
            }
        }

        boxBorder->decRef();
    }

    instances++;
}

DropDown::~DropDown()
{
    instances--;
    // Free images memory
    if (instances == 0)
    {
        buttons[0][0]->decRef();
        buttons[0][1]->decRef();
        buttons[1][0]->decRef();
        buttons[1][1]->decRef();

        for_each(skin.grid, skin.grid + 9, dtor<Image*>());
    }
}

void DropDown::draw(gcn::Graphics* graphics)
{
    int h;

    if (mDroppedDown)
    {
        h = mFoldedUpHeight;
    }
    else
    {
        h = getHeight();
    }

    int alpha = getBaseColor().a;
    gcn::Color faceColor = getBaseColor();
    faceColor.a = alpha;
    gcn::Color highlightColor = faceColor + 0x303030;
    highlightColor.a = alpha;
    gcn::Color shadowColor = faceColor - 0x303030;
    shadowColor.a = alpha;


    graphics->setColor(getBackgroundColor());
    graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), h));

    graphics->setColor(getForegroundColor());
    graphics->setFont(getFont());

    if (mListBox->getListModel() && mListBox->getSelected() >= 0)
    {
        graphics->drawText(mListBox->getListModel()->getElementAt(mListBox->getSelected()), 1, 0);
    }

    if (isFocused())
    {
        graphics->setColor(highlightColor);
        graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth() - h, h));
    }

    drawButton(graphics);

    if (mDroppedDown)
    {
        drawChildren(graphics);

        // Draw two lines separating the ListBox with se selected
        // element view.
        graphics->setColor(highlightColor);
        graphics->drawLine(0, h, getWidth(), h);
        graphics->setColor(shadowColor);
        graphics->drawLine(0, h + 1, getWidth(), h + 1);
    }
}

void DropDown::drawFrame(gcn::Graphics *graphics)
{
    const int bs = getFrameSize();
    const int w = getWidth() + bs * 2;
    const int h = getHeight() + bs * 2;

    static_cast<Graphics*>(graphics)->drawImageRect(0, 0, w, h, skin);
}

void DropDown::drawButton(gcn::Graphics *graphics)
{
    int height = mDroppedDown ? mFoldedUpHeight : getHeight();

    static_cast<Graphics*>(graphics)->
        drawImage(buttons[mDroppedDown][mPushed], getWidth() - height, 1);
}
