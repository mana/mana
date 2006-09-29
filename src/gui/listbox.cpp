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

#include "listbox.h"

#include "selectionlistener.h"

#include <guichan/font.hpp>
#include <guichan/graphics.hpp>
#include <guichan/listmodel.hpp>
#include <guichan/mouseinput.hpp>

ListBox::ListBox(gcn::ListModel *listModel):
    gcn::ListBox(listModel),
    mMousePressed(false)
{
}

void ListBox::draw(gcn::Graphics *graphics)
{
    if (mListModel == NULL) {
        return;
    }

    graphics->setColor(gcn::Color(110, 160, 255));
    graphics->setFont(getFont());

    int fontHeight = getFont()->getHeight();

    // Draw rectangle below the selected list element
    if (mSelected >= 0) {
        graphics->fillRectangle(gcn::Rectangle(0, fontHeight * mSelected,
                                               getWidth(), fontHeight));
    }

    // Draw the list elements
    for (int i = 0, y = 0; i < mListModel->getNumberOfElements(); ++i, y += fontHeight)
    {
        graphics->drawText(mListModel->getElementAt(i), 1, y);
    }
}

void ListBox::setSelected(int selected)
{
    gcn::ListBox::setSelected(selected);
    fireSelectionChangedEvent();
}

void ListBox::mousePress(int x, int y, int button)
{
    gcn::ListBox::mousePress(x, y, button);

    if (button == gcn::MouseInput::LEFT && hasMouse())
    {
        mMousePressed = true;
    }
}

void ListBox::mouseRelease(int x, int y, int button)
{
    gcn::ListBox::mouseRelease(x, y, button);

    mMousePressed = false;
}

void ListBox::mouseMotion(int x, int y)
{
    gcn::ListBox::mouseMotion(x, y);

    // Pretend mouse is pressed continuously while dragged. Causes list
    // selection to be updated as is default in many GUIs.
    if (mMousePressed)
    {
        mousePress(x, y, gcn::MouseInput::LEFT);
    }
}

void ListBox::fireSelectionChangedEvent()
{
    SelectionEvent event(this);
    SelectionListeners::iterator i_end = mListeners.end();
    SelectionListeners::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
    {
        (*i)->selectionChanged(event);
    }
}
