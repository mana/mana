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

#include "gui/widgets/listbox.h"

#include "gui/palette.h"
#include "gui/sdlinput.h"

#include "configuration.h"

#include <guichan/font.hpp>
#include <guichan/graphics.hpp>
#include <guichan/key.hpp>
#include <guichan/listmodel.hpp>

float ListBox::mAlpha = 1.0;

ListBox::ListBox(gcn::ListModel *listModel):
    gcn::ListBox(listModel)
{
}

void ListBox::draw(gcn::Graphics *graphics)
{
    if (!mListModel)
        return;

    if (config.getValue("guialpha", 0.8) != mAlpha)
        mAlpha = config.getValue("guialpha", 0.8);

    graphics->setColor(guiPalette->getColor(Palette::HIGHLIGHT,
            (int)(mAlpha * 255.0f)));
    graphics->setFont(getFont());

    const int fontHeight = getFont()->getHeight();

    // Draw filled rectangle around the selected list element
    if (mSelected >= 0)
        graphics->fillRectangle(gcn::Rectangle(0, fontHeight * mSelected,
                                               getWidth(), fontHeight));

    // Draw the list elements
    graphics->setColor(guiPalette->getColor(Palette::TEXT));
    for (int i = 0, y = 0; i < mListModel->getNumberOfElements();
         ++i, y += fontHeight)
    {
        graphics->drawText(mListModel->getElementAt(i), 1, y);
    }
}

void ListBox::keyPressed(gcn::KeyEvent& keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ENTER || key.getValue() == Key::SPACE)
    {
        distributeActionEvent();
        keyEvent.consume();
    }
    else if (key.getValue() == Key::UP)
    {
        if (getSelected() > 0)
            setSelected(mSelected - 1);
        else if (getSelected() == 0 && mWrappingEnabled)
            setSelected(getListModel()->getNumberOfElements() - 1);
        keyEvent.consume();
    }
    else if (key.getValue() == Key::DOWN)
    {
        if (getSelected() < (getListModel()->getNumberOfElements() - 1))
            setSelected(mSelected + 1);
        else if (getSelected() == (getListModel()->getNumberOfElements() - 1) &&
                 mWrappingEnabled)
            setSelected(0);
        keyEvent.consume();
    }
    else if (key.getValue() == Key::HOME)
    {
        setSelected(0);
        keyEvent.consume();
    }
    else if (key.getValue() == Key::END)
    {
        setSelected(getListModel()->getNumberOfElements() - 1);
        keyEvent.consume();
    }
}

// Don't do anything on scrollwheel. ScrollArea will deal with that.

void ListBox::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
}

void ListBox::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
}

void ListBox::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() != gcn::MouseEvent::LEFT)
        return;

    // Make list selection update on drag, but guard against negative y
    int y = std::max(0, event.getY());
    setSelected(y / getRowHeight());
}
