/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "gui/widgets/avatarlistbox.h"

#include "graphics.h"

#include "gui/gui.h"
#include "gui/palette.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include <guichan/font.hpp>

int AvatarListBox::instances = 0;
Image *AvatarListBox::onlineIcon = 0;
Image *AvatarListBox::offlineIcon = 0;

AvatarListBox::AvatarListBox(AvatarListModel *model):
    ListBox(model)
{
    instances++;

    if (instances == 1)
    {
        onlineIcon = ResourceManager::getInstance()->getImage("graphics/gui/circle-green.png");
        offlineIcon = ResourceManager::getInstance()->getImage("graphics/gui/circle-gray.png");
    }

    setWidth(200);
}

AvatarListBox::~AvatarListBox()
{
    instances--;

    if (instances == 0)
    {
        onlineIcon->decRef();
        offlineIcon->decRef();
    }
}

void AvatarListBox::draw(gcn::Graphics *gcnGraphics)
{
    AvatarListModel* model = dynamic_cast<AvatarListModel*>(mListModel);

    if (!model)
        return;

    updateAlpha();

    Graphics *graphics = static_cast<Graphics*>(gcnGraphics);

    graphics->setColor(guiPalette->getColor(Palette::HIGHLIGHT,
            (int)(mAlpha * 255.0f)));
    graphics->setFont(getFont());

    const int fontHeight = getFont()->getHeight();

    // Draw filled rectangle around the selected list element
    if (mSelected >= 0)
        graphics->fillRectangle(gcn::Rectangle(0, fontHeight * mSelected,
                                               getWidth(), fontHeight));

    int newWidth = 0;
    int width = 0;

    // Draw the list elements
    graphics->setColor(guiPalette->getColor(Palette::TEXT));
    for (int i = 0, y = 0;
         i < model->getNumberOfElements();
         ++i, y += fontHeight)
    {
        Avatar *a = model->getAvatarAt(i);
        // Draw online status
        Image *icon = a->getOnline() ? onlineIcon : offlineIcon;
        if (icon)
            graphics->drawImage(icon, 2, y + 1);

        if (a->getDisplayBold())
            graphics->setFont(boldFont);

        // Draw Name
        graphics->drawText(a->getName(), 15, y);

        width = getFont()->getWidth(a->getName()) + 17; // Extra right padding

        if (width > newWidth)
        {
            newWidth = width;
        }

        if (a->getDisplayBold())
            graphics->setFont(getFont());
    }

    setWidth(newWidth);
}

void AvatarListBox::mousePressed(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        int y = event.getY();
        setSelected(y / getFont()->getHeight());
        distributeActionEvent();
    }
    // TODO: Add support for context menu
    else if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        // Show context menu
    }
}
