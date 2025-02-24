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

int AvatarListBox::instances = 0;
ResourceRef<Image> AvatarListBox::onlineIcon;
ResourceRef<Image> AvatarListBox::offlineIcon;

AvatarListBox::AvatarListBox(AvatarListModel *model):
    ListBox(model)
{
    instances++;

    if (instances == 1)
    {
        onlineIcon = Theme::getImageFromTheme("circle-green.png");
        offlineIcon = Theme::getImageFromTheme("circle-gray.png");
    }

    setWidth(200);
}

AvatarListBox::~AvatarListBox()
{
    instances--;

    if (instances == 0)
    {
        onlineIcon = nullptr;
        offlineIcon = nullptr;
    }
}

void AvatarListBox::draw(gcn::Graphics *gcnGraphics)
{
    if (!mListModel)
        return;

    auto* model = static_cast<AvatarListModel*>(mListModel);

    updateAlpha();

    auto *graphics = static_cast<Graphics*>(gcnGraphics);

    graphics->setColor(Theme::getThemeColor(Theme::HIGHLIGHT,
                                            (int) (mAlpha * 255.0f)));
    graphics->setFont(getFont());

    const int fontHeight = getFont()->getHeight();

    // Draw filled rectangle around the selected list element
    if (mSelected >= 0)
        graphics->fillRectangle(gcn::Rectangle(0, fontHeight * mSelected,
                                               getWidth(), fontHeight));

    // Draw the list elements
    graphics->setColor(Theme::getThemeColor(Theme::TEXT));
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
        graphics->drawText(text, 15, y);

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
