/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "gui/widgets/button.h"

#include "equipment.h"
#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"

#include "gui/equipmentwindow.h"
#include "gui/itempopup.h"
#include "gui/setup.h"
#include "gui/viewport.h"

#include "gui/widgets/playerbox.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>

EquipmentWindow::EquipmentWindow(Equipment *equipment):
    Window(_("Equipment")),
    mEquipment(equipment)
{
    mItemPopup = new ItemPopup;
    setupWindow->registerWindowForReset(this);

    // Control that shows the Player
    auto *playerBox = new PlayerBox;
    playerBox->setDimension(gcn::Rectangle(50, 80, 74, 123));
    playerBox->setPlayer(local_player);

    setWindowName("Equipment");
    setCloseButton(true);
    setSaveVisible(true);
    setContentSize(175, 290);
    setDefaultSize(getWidth(), getHeight(), ImageRect::CENTER);
    loadWindowState();

    mUnequip = new Button(_("Unequip"), "unequip", this);
    const gcn::Rectangle &area = getChildrenArea();
    mUnequip->setPosition(area.width  - mUnequip->getWidth() - getPadding(),
                          area.height - mUnequip->getHeight() - getPadding());
    mUnequip->setEnabled(false);

    add(playerBox);
    add(mUnequip);

    loadEquipBoxes();
}

void EquipmentWindow::loadEquipBoxes()
{
    mBoxes.resize(mEquipment->getSlotNumber());

    for (size_t i = 0; i < mBoxes.size(); ++i)
    {
        auto &box = mBoxes[i];

        Position boxPosition = Net::getInventoryHandler()->getBoxPosition(i);
        box.posX = boxPosition.x + getPadding();
        box.posY = boxPosition.y + getTitleBarHeight();

        const std::string &backgroundFile =
            Net::getInventoryHandler()->getBoxBackground(i);

        if (!backgroundFile.empty())
            box.backgroundImage = Theme::getImageFromTheme(backgroundFile);
    }
}

EquipmentWindow::~EquipmentWindow()
{
    delete mItemPopup;
}

void EquipmentWindow::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);

    // Draw equipment boxes
    auto *g = static_cast<Graphics*>(graphics);

    auto &boxSkin = gui->getTheme()->getSkin(SkinType::EquipmentBox);

    for (size_t i = 0; i < mBoxes.size(); i++)
    {
        const auto &box = mBoxes[i];

        WidgetState boxState(gcn::Rectangle(box.posX, box.posY, boxSkin.width, boxSkin.height));
        if (static_cast<int>(i) == mSelected)
            boxState.flags |= STATE_SELECTED;

        boxSkin.draw(g, boxState);

        // When there is a background image, draw it centered in the box:
        if (box.backgroundImage)
        {
            int posX = box.posX
                + (boxSkin.width - box.backgroundImage->getWidth()) / 2;
            int posY = box.posY
                + (boxSkin.height - box.backgroundImage->getHeight()) / 2;
            g->drawImage(box.backgroundImage, posX, posY);
        }

        if (Item *item = mEquipment->getEquipment(i))
        {
            if (Image *image = item->getImage())
            {
                image->setAlpha(1.0f);
                g->drawImage(image,
                             box.posX + boxSkin.padding,
                             box.posY + boxSkin.padding);
            }

            if (i == TmwAthena::EQUIP_PROJECTILE_SLOT)
            {
                g->setColor(Theme::getThemeColor(Theme::TEXT));
                graphics->drawText(toString(item->getQuantity()),
                                   box.posX + (boxSkin.width / 2),
                                   box.posY - getFont()->getHeight(),
                                   gcn::Graphics::CENTER);
            }
        }
    }
}

void EquipmentWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "unequip" && mSelected > -1)
    {
        mEquipment->triggerUnequip(mSelected);
        setSelected(-1);
    }
}

/**
 * Returns an index of an equipment box at the given position, or -1 if there
 * is no box.
 */
int EquipmentWindow::getBoxIndex(int x, int y) const
{
    auto &boxSkin = gui->getTheme()->getSkin(SkinType::EquipmentBox);

    for (size_t i = 0; i < mBoxes.size(); ++i)
    {
        const auto &box = mBoxes[i];
        const gcn::Rectangle tRect(box.posX, box.posY, boxSkin.width, boxSkin.height);
        if (tRect.isPointInRect(x, y))
            return i;
    }

    return -1;
}

Item *EquipmentWindow::getItem(int x, int y) const
{
    const int index = getBoxIndex(x, y);
    return index != -1 ? mEquipment->getEquipment(index) : nullptr;
}

std::string EquipmentWindow::getSlotName(int x, int y) const
{
    const int index = getBoxIndex(x, y);
    return index != -1 ? mEquipment->getSlotName(index) : std::string();
}

void EquipmentWindow::mousePressed(gcn::MouseEvent& mouseEvent)
{
    Window::mousePressed(mouseEvent);

    const int x = mouseEvent.getX();
    const int y = mouseEvent.getY();
    Item *item = nullptr;

    const int index = getBoxIndex(x, y);
    if (index != -1)
    {
        item = mEquipment->getEquipment(index);
        if (item)
            setSelected(index);
    }

    if (mouseEvent.getButton() == gcn::MouseEvent::RIGHT)
    {
        if (item)
        {
            /* Convert relative to the window coordinates to absolute screen
             * coordinates.
             */
            const int mx = x + getX();
            const int my = y + getY();
            viewport->showPopup(this, mx, my, item, true, false);
        }
    }
}

void EquipmentWindow::mouseMoved(gcn::MouseEvent &event)
{
    Window::mouseMoved(event);

    const int x = event.getX();
    const int y = event.getY();

    // Show ItemTooltip
    std::string slotName = getSlotName(x, y);
    if (!slotName.empty())
    {
        mItemPopup->setEquipmentText(slotName);

        if (Item *item = getItem(x, y))
            mItemPopup->setItem(item->getInfo());
        else
            mItemPopup->setNoItem();

        mItemPopup->position(x + getX(), y + getY());
    }
    else
    {
        mItemPopup->setVisible(false);
    }
}

void EquipmentWindow::mouseExited(gcn::MouseEvent &event)
{
    Window::mouseExited(event);

    mItemPopup->setVisible(false);
}

void EquipmentWindow::setSelected(int index)
{
    mSelected = index;
    mUnequip->setEnabled(mSelected != -1);
}
