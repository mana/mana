/*
 *  The Mana World
 *  Copyright (C) 2006  The Mana World Development Team
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

#include "gui/widgets/dropdown.h"

#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"

#include "gui/palette.h"
#include "gui/sdlinput.h"

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"

#include <algorithm>

int DropDown::instances = 0;
Image *DropDown::buttons[2][2];
ImageRect DropDown::skin;
float DropDown::mAlpha = 1.0;

DropDown::DropDown(gcn::ListModel *listModel):
    gcn::DropDown::DropDown(listModel,
                            new ScrollArea,
                            new ListBox(listModel))
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

        buttons[0][0]->setAlpha(mAlpha);
        buttons[0][1]->setAlpha(mAlpha);
        buttons[1][0]->setAlpha(mAlpha);
        buttons[1][1]->setAlpha(mAlpha);

        // get the border skin
        Image *boxBorder = resman->getImage("graphics/gui/deepbox.png");
        int gridx[4] = {0, 3, 28, 31};
        int gridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++)
        {
            for (x = 0; x < 3; x++)
            {
                skin.grid[a] = boxBorder->getSubImage(gridx[x], gridy[y],
                                                      gridx[x + 1] -
                                                      gridx[x] + 1,
                                                      gridy[y + 1] -
                                                      gridy[y] + 1);
                skin.grid[a]->setAlpha(mAlpha);
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

    delete mScrollArea;
}

void DropDown::draw(gcn::Graphics* graphics)
{
    int h;

    if (mDroppedDown)
        h = mFoldedUpHeight;
    else
        h = getHeight();

    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        buttons[0][0]->setAlpha(mAlpha);
        buttons[0][1]->setAlpha(mAlpha);
        buttons[1][0]->setAlpha(mAlpha);
        buttons[1][1]->setAlpha(mAlpha);

        for (int a = 0; a < 9; a++)
        {
            skin.grid[a]->setAlpha(mAlpha);
        }
    }

    const int alpha = (int) (mAlpha * 255.0f);
    gcn::Color faceColor = getBaseColor();
    faceColor.a = alpha;
    const gcn::Color *highlightColor =
            &guiPalette->getColor(Palette::HIGHLIGHT, alpha);
    gcn::Color shadowColor = faceColor - 0x303030;
    shadowColor.a = alpha;

    if (mListBox->getListModel() && mListBox->getSelected() >= 0)
    {
        graphics->setFont(getFont());
        graphics->setColor(guiPalette->getColor(Palette::TEXT));
        graphics->drawText(mListBox->getListModel()->getElementAt(mListBox->getSelected()), 1, 0);
    }

    if (isFocused())
    {
        graphics->setColor(*highlightColor);
        graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth() - h, h));
    }

    drawButton(graphics);

    if (mDroppedDown)
    {
        drawChildren(graphics);

        // Draw two lines separating the ListBox with selected
        // element view.
        graphics->setColor(*highlightColor);
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
        drawImage(buttons[mDroppedDown][mPushed], getWidth() - height + 2, 1);
}

// -- KeyListener notifications
void DropDown::keyPressed(gcn::KeyEvent& keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ENTER || key.getValue() == Key::SPACE)
    {
        if (!mDroppedDown)
            dropDown();
        keyEvent.consume();
    }
    else if (key.getValue() == Key::UP)
    {
        setSelected(getSelected() - 1);
        keyEvent.consume();
    }
    else if (key.getValue() == Key::DOWN)
    {
        setSelected(getSelected() + 1);
        keyEvent.consume();
    }
    else if (key.getValue() == Key::HOME)
    {
        setSelected(0);
        keyEvent.consume();
    }
    else if (key.getValue() == Key::END)
    {
        setSelected(mListBox->getListModel()->getNumberOfElements() - 1);
        keyEvent.consume();
    }
}

void DropDown::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    setSelected(getSelected() - 1);
    mouseEvent.consume();
}

void DropDown::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    setSelected(getSelected() + 1);
    mouseEvent.consume();
}
