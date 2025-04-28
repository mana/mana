/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/widgets/avatarlistbox.h"

#include "graphics.h"

#include "gui/gui.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"

#include "utils/stringutils.h"

#include <guichan/font.hpp>

AvatarListBox::AvatarListBox(AvatarListModel *model):
    ListBox(model)
{
    setWidth(200);
}

unsigned int AvatarListBox::getRowHeight() const
{
    auto rowHeight = ListBox::getRowHeight();

    auto theme = gui->getTheme();
    if (auto onlineIcon = theme->getIcon("online"))
        rowHeight = std::max<unsigned>(rowHeight, onlineIcon->getHeight() + 2);

    return rowHeight;
}

void AvatarListBox::draw(gcn::Graphics *gcnGraphics)
{
    if (!mListModel)
        return;

    auto *model = static_cast<AvatarListModel *>(mListModel);
    auto *graphics = static_cast<Graphics *>(gcnGraphics);

    graphics->setFont(getFont());

    const int rowHeight = getRowHeight();

    // Draw filled rectangle around the selected list element
    if (mSelected >= 0)
    {
        auto highlightColor = Theme::getThemeColor(Theme::HIGHLIGHT);
        highlightColor.a = gui->getTheme()->getGuiAlpha();
        graphics->setColor(highlightColor);
        graphics->fillRectangle(gcn::Rectangle(0, rowHeight * mSelected,
                                               getWidth(), rowHeight));
    }

    auto theme = gui->getTheme();
    auto onlineIcon = theme->getIcon("online");
    auto offlineIcon = theme->getIcon("offline");

    // Draw the list elements
    graphics->setColor(Theme::getThemeColor(Theme::TEXT));
    for (int i = 0, y = 0;
         i < model->getNumberOfElements();
         ++i, y += rowHeight)
    {
        Avatar *a = model->getAvatarAt(i);
        int x = 1;

        // Draw online status
        if (const Image *icon = a->getOnline() ? onlineIcon : offlineIcon)
        {
            graphics->drawImage(icon, x, y + (rowHeight - icon->getHeight()) / 2);
            x += icon->getWidth() + 4;
        }

        if (a->getDisplayBold())
            graphics->setFont(boldFont);

        std::string text;

        if (a->getMaxHp() > 0)
        {
            text = strprintf("%s %d/%d", a->getName().c_str(),
                             a->getHp(), a->getMaxHp());
        }
        else
        {
            text = a->getName();
        }

        // Draw Name
        graphics->drawText(text, x, y);

        if (a->getDisplayBold())
            graphics->setFont(getFont());
    }
}

void AvatarListBox::mousePressed(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        ListBox::mousePressed(event);
    }
    // TODO: Add support for context menu
    else if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        // Show context menu
    }
}
