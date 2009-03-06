/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#include <guichan/font.hpp>
#include <guichan/graphics.hpp>
#include <guichan/key.hpp>
#include <guichan/listmodel.hpp>

#include "color.h"
#include "listbox.h"

#include "../configuration.h"

float ListBox::mAlpha = config.getValue("guialpha", 0.8);

ListBox::ListBox(gcn::ListModel *listModel):
    gcn::ListBox(listModel)
{
}

void ListBox::draw(gcn::Graphics *graphics)
{
    if (!mListModel || !isVisible())
        return;

    if (config.getValue("guialpha", 0.8) != mAlpha)
        mAlpha = config.getValue("guialpha", 0.8);

    bool valid;
    const int red = (textColor->getColor('H', valid) >> 16) & 0xFF;
    const int green = (textColor->getColor('H', valid) >> 8) & 0xFF;
    const int blue = textColor->getColor('H', valid) & 0xFF;
    const int alpha = (int)(mAlpha * 255.0f);

    graphics->setColor(gcn::Color(red, green, blue, alpha));
    graphics->setFont(getFont());

    const int fontHeight = getFont()->getHeight();

    // Draw filled rectangle around the selected list element
    if (mSelected >= 0)
        graphics->fillRectangle(gcn::Rectangle(0, fontHeight * mSelected,
                                               getWidth(), fontHeight));

    // Draw the list elements
    graphics->setColor(gcn::Color(0, 0, 0, 255));
    for (int i = 0, y = 0; i < mListModel->getNumberOfElements();
         ++i, y += fontHeight)
    {
        graphics->drawText(mListModel->getElementAt(i), 1, y);
    }
}

void ListBox::setSelected(int selected)
{
    if (!mListModel)
    {
        mSelected = -1;
    }
    else
    {
        if (selected < 0 && !mWrappingEnabled)
        {
            mSelected = -1;
        }
        else if (selected >= mListModel->getNumberOfElements() &&
                 mWrappingEnabled)
        {
            mSelected = 0;
        }
        else if ((selected >= mListModel->getNumberOfElements() &&
                 !mWrappingEnabled) || (selected < 0 && mWrappingEnabled))
        {
            mSelected = mListModel->getNumberOfElements() - 1;
        }
        else
        {
            mSelected = selected;
        }
    }
    gcn::ListBox::setSelected(mSelected);
}

// -- KeyListener notifications
void ListBox::keyPressed(gcn::KeyEvent& keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == gcn::Key::ENTER || key.getValue() == gcn::Key::SPACE)
    {
        distributeActionEvent();
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::UP)
    {
        setSelected(mSelected - 1);      
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::DOWN)
    {
        setSelected(mSelected + 1);
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::HOME)
    {
        setSelected(0);
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::END)
    {
        setSelected(getListModel()->getNumberOfElements() - 1);
        keyEvent.consume();
    }
}

void ListBox::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    if (isFocused())
    {
        if (getSelected() > 0 || (getSelected() == 0 && mWrappingEnabled))
        {
            setSelected(getSelected() - 1);
        }

        mouseEvent.consume();
    }
}

void ListBox::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    if (isFocused())
    {
        setSelected(getSelected() + 1);

        mouseEvent.consume();
    }
}

void ListBox::mouseDragged(gcn::MouseEvent &event)
{
    // Pretend mouse is pressed continuously while dragged. Causes list
    // selection to be updated as is default in many GUIs.
    mousePressed(event);
}
