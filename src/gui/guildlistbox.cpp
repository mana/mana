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

#include "gui/guildlistbox.h"

#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include <guichan/font.hpp>

GuildListBox::GuildListBox():
    ListBox(NULL)
{
    onlineIcon = ResourceManager::getInstance()->getImage("graphics/gui/circle-green.png");
    offlineIcon = ResourceManager::getInstance()->getImage("graphics/gui/circle-gray.png");
}

void GuildListBox::draw(gcn::Graphics *gcnGraphics)
{
    if (!mListModel)
        return;

    Graphics *graphics = static_cast<Graphics*>(gcnGraphics);

    graphics->setColor(gcn::Color(110, 160, 255));
    graphics->setFont(getFont());

    int fontHeight = getFont()->getHeight();

    // Draw rectangle below the selected list element
    if (mSelected >= 0) {
        graphics->fillRectangle(gcn::Rectangle(0, fontHeight * mSelected,
                                               getWidth(), fontHeight));
    }

    // Draw the list elements
    for (int i = 0, y = 0;
         i < mListModel->getNumberOfElements();
         ++i, y += fontHeight)
    {
        // Draw online status
        bool online = false;
        UserMap::iterator itr = mUsers.find(mListModel->getElementAt(i));
        if (itr != mUsers.end())
        {
            online = itr->second;
        }
        Image *icon = online ? onlineIcon : offlineIcon;
        if (icon)
            graphics->drawImage(icon, 1, y);
        // Draw Name
        graphics->setColor(gcn::Color(0, 0, 0));
        graphics->drawText(mListModel->getElementAt(i), 33, y);
    }
}
/*
void GuildListBox::setSelected(int selected)
{
    if (!mListModel)
    {
        mSelected = -1;
    }
    else
    {
        // Update mSelected with bounds checking
        mSelected = std::min(mListModel->getNumberOfElements() - 1,
                             std::max(-1, selected));

        gcn::Widget *parent;
        parent = (gcn::Widget*)getParent();
        if (parent)
        {
            gcn::Rectangle scroll;
            scroll.y = (mSelected < 0) ? 0 : getFont()->getHeight() * mSelected;
            scroll.height = getFont()->getHeight();
            parent->showWidgetPart(this, scroll);
        }
    }

    distributeValueChangedEvent();
}
*/
void GuildListBox::mousePressed(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        int y = event.getY();
        setSelected(y / getFont()->getHeight());
        distributeActionEvent();
    }
    // TODO: Add guild functions, ie private messaging
    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        // Show context menu
    }
}

void GuildListBox::setOnlineStatus(const std::string &user, bool online)
{
    UserMap::iterator itr = mUsers.find(user);
    if (itr == mUsers.end())
    {
        mUsers.insert(std::pair<std::string, bool>(user, online));
    }
    else
    {
        itr->second = online;
    }
    adjustSize();
}
