/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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
#include "gui/gui.h"
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
    setDefaultSize(getWidth(), getHeight(), WindowAlignment::Center);
    loadWindowState();

    mUnequip = new Button(_("Unequip"), "unequip", this);
    const gcn::Rectangle &area = getChildrenArea();
    mUnequip->setPosition(area.width  - mUnequip->getWidth() - getPadding(),
                          area.height - mUnequip->getHeight() - getPadding());
    mUnequip->setEnabled(false);

    add(playerBox);
    add(mUnequip);
}

EquipmentWindow::~EquipmentWindow()
{
    delete mItemPopup;
}

void EquipmentWindow::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);

    auto *g = static_cast<Graphics*>(graphics);

    auto theme = gui->getTheme();
    auto &boxSkin = theme->getSkin(SkinType::EquipmentBox);

    // Draw equipment boxes
    const int boxCount = mEquipment->getSlotNumber();
    for (int i = 0; i < boxCount; ++i)
    {
        Position boxPos = Net::getInventoryHandler()->getBoxPosition(i);
        boxPos.x += getPadding();
        boxPos.y += getTitleBarHeight();

        WidgetState boxState(gcn::Rectangle(boxPos.x, boxPos.y, boxSkin.width, boxSkin.height));
        if (i == mSelected)
            boxState.flags |= STATE_SELECTED;

        boxSkin.draw(g, boxState);

        if (Item *item = mEquipment->getEquipment(i))
        {
            if (Image *image = item->getImage())
            {
                const Drag *drag = gui->getActiveDrag();
                const bool isDragged =
                        drag &&
                        drag->source == this &&
                        drag->sourceIndex == i;
                image->setAlpha(isDragged ? 0.5f : 1.0f);
                g->drawImage(image,
                             boxPos.x + boxSkin.padding,
                             boxPos.y + boxSkin.padding);
            }

            if (i == TmwAthena::EQUIP_PROJECTILE_SLOT)
            {
                g->setColor(Theme::getThemeColor(Theme::TEXT));
                graphics->drawText(toString(item->getQuantity()),
                                   boxPos.x + boxSkin.width / 2,
                                   boxPos.y - getFont()->getHeight(),
                                   gcn::Graphics::CENTER);
            }
        }
        else
        {
            auto &icon = Net::getInventoryHandler()->getBoxIcon(i);
            if (!icon.empty())
                if (auto image = theme->getIcon(icon))
                    g->drawImage(image, boxPos.x + boxSkin.padding, boxPos.y + boxSkin.padding);
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
 * Allows equipping items by dragging them from the inventory, shortcuts, or outfits.
 */
bool EquipmentWindow::handleDrop(const Drag &drag, int absX, int absY)
{
    if (drag.source == this)
    {
        int widgetX = 0;
        int widgetY = 0;
        getAbsolutePosition(widgetX, widgetY);

        const int dropIndex = getBoxIndex(absX - widgetX, absY - widgetY);
        return dropIndex == drag.sourceIndex;
    }

    if (drag.sourceType != Drag::SourceType::Inventory &&
        drag.sourceType != Drag::SourceType::ItemShortcut &&
        drag.sourceType != Drag::SourceType::Outfit)
        return false;

    Item *item = drag.item.get();
    if (!item)
        return false;

    if (!item->isEquippable() || item->isEquipped())
        return true;

    item->doEvent(Event::DoEquip);
    return true;
}

/**
 * Returns an index of an equipment box at the given position, or -1 if there
 * is no box.
 */
int EquipmentWindow::getBoxIndex(int x, int y) const
{
    auto &boxSkin = gui->getTheme()->getSkin(SkinType::EquipmentBox);

    // Translate coordinates to content area
    const auto childrenArea = const_cast<EquipmentWindow*>(this)->getChildrenArea();
    x -= childrenArea.x;
    y -= childrenArea.y;

    const int boxCount = mEquipment->getSlotNumber();
    for (int i = 0; i < boxCount; ++i)
    {
        const Position boxPos = Net::getInventoryHandler()->getBoxPosition(i);
        const gcn::Rectangle tRect(boxPos.x, boxPos.y, boxSkin.width, boxSkin.height);
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
    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
        mClickedIndex = -1;

    const int x = mouseEvent.getX();
    const int y = mouseEvent.getY();
    Item *item = nullptr;

    const int index = getBoxIndex(x, y);
    if (index != -1)
    {
        item = mEquipment->getEquipment(index);
        if (item)
        {
            setSelected(index);
            if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
            {
                mClickedIndex = index;
                mMoved = false;
            }
        }
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

void EquipmentWindow::mouseDragged(gcn::MouseEvent &event)
{
    Window::mouseDragged(event);

    if (event.getButton() != gcn::MouseEvent::LEFT)
        return;

    if (mMoved)
        return;

    if (!gui->getActiveDrag() && mClickedIndex != -1)
    {
        if (Item *item = mEquipment->getEquipment(mClickedIndex))
        {
            gui->startDrag(Drag::fromEquipment(item, this, mClickedIndex));
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

void EquipmentWindow::dragFinished(const Drag &drag, DragResult result)
{
    if (result == DragResult::Ignored &&
        drag.source == this &&
        drag.sourceIndex >= 0)
    {
        mEquipment->triggerUnequip(drag.sourceIndex);
    }

    mClickedIndex = -1;
}

void EquipmentWindow::setSelected(int index)
{
    mSelected = index;
    mUnequip->setEnabled(mSelected != -1);
}
