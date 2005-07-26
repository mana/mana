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
#include "../graphics.h"
#include "../main.h"

ListBox::ListBox():
    gcn::ListBox()
{
}

ListBox::ListBox(gcn::ListModel *listModel):
    gcn::ListBox(listModel)
{
}

void ListBox::draw(gcn::Graphics *graphics)
{
    if (mListModel == NULL) {
        return;
    }

    graphics->setColor(gcn::Color(110, 160, 255));
    graphics->setFont(getFont());

    int i;
    int fontHeight = getFont()->getHeight();
    int y = 0;

    for (i = 0; i < mListModel->getNumberOfElements(); ++i)
    {
        if (i == mSelected) {
            if (useOpenGL) {
                dynamic_cast<gcn::OpenGLGraphics*>(graphics)->fillRectangle(
                    gcn::Rectangle(0, y, getWidth(), fontHeight));
            }
            else {
                dynamic_cast<gcn::SDLGraphics*>(graphics)->fillRectangle(
                    gcn::Rectangle(0, y, getWidth(), fontHeight));
            }
        }

        graphics->drawText(mListModel->getElementAt(i), 1, y);

        y += fontHeight;
    }
}
