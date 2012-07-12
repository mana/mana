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

static const int BOX_WIDTH = 36;
static const int BOX_HEIGHT = 36;

EquipmentWindow::EquipmentWindow(Equipment *equipment):
    Window(_("Equipment")),
    mEquipBox(0),
    mSelected(-1),
    mEquipment(equipment),
    mBoxesNumber(0)
{
    mItemPopup = new ItemPopup;
    setupWindow->registerWindowForReset(this);

    // Control that shows the Player
    PlayerBox *playerBox = new PlayerBox;
    playerBox->setDimension(gcn::Rectangle(50, 80, 74, 123));
    playerBox->setPlayer(local_player);

    setWindowName("Equipment");
    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize(180, 300, ImageRect::CENTER);
    loadWindowState();

    mUnequip = new Button(_("Unequip"), "unequip", this);
    const gcn::Rectangle &area = getChildrenArea();
    mUnequip->setPosition(area.width  - mUnequip->getWidth() - 5,
                          area.height - mUnequip->getHeight() - 5);
    mUnequip->setEnabled(false);

    add(playerBox);
    add(mUnequip);
}

void EquipmentWindow::loadEquipBoxes()
{
    if (mEquipBox)
        delete[] mEquipBox;

    // Load equipment boxes.
    mBoxesNumber = mEquipment->getSlotNumber();
    mEquipBox = new EquipBox[mBoxesNumber];

    for (int i = 0; i < mBoxesNumber; ++i)
    {
        Position boxPosition = Net::getInventoryHandler()->getBoxPosition(i);
        mEquipBox[i].posX = boxPosition.x + getPadding();
        mEquipBox[i].posY = boxPosition.y + getTitleBarHeight();

        const std::string &backgroundFile =
            Net::getInventoryHandler()->getBoxBackground(i);

        if (!backgroundFile.empty())
        {
            mEquipBox[i].backgroundImage =
                Theme::instance()->getImageFromTheme(backgroundFile);
        }
    }
}

EquipmentWindow::~EquipmentWindow()
{
    delete mItemPopup;
    if (mEquipBox)
        delete[] mEquipBox;
}

void EquipmentWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);

    Window::drawChildren(graphics);

    // Draw equipment boxes
    Graphics *g = static_cast<Graphics*>(graphics);

    for (int i = 0; i < mBoxesNumber; i++)
    {
        // When there is a background image, draw it centered in the box:
        if (mEquipBox[i].backgroundImage)
        {
            int posX = mEquipBox[i].posX
                + (BOX_WIDTH - mEquipBox[i].backgroundImage->getWidth()) / 2;
            int posY = mEquipBox[i].posY
                + (BOX_HEIGHT - mEquipBox[i].backgroundImage->getHeight()) / 2;
            g->drawImage(mEquipBox[i].backgroundImage, posX, posY);
        }

        if (i == mSelected)
        {
            const gcn::Color color = Theme::getThemeColor(Theme::HIGHLIGHT);

            // Set color to the highlight color
            g->setColor(gcn::Color(color.r, color.g, color.b, getGuiAlpha()));
            g->fillRectangle(gcn::Rectangle(mEquipBox[i].posX,
                                            mEquipBox[i].posY,
                                            BOX_WIDTH, BOX_HEIGHT));
        }

        // Set color black
        g->setColor(gcn::Color(0, 0, 0));
        // Draw box border
        g->drawRectangle(gcn::Rectangle(mEquipBox[i].posX, mEquipBox[i].posY,
                                        BOX_WIDTH, BOX_HEIGHT));

        Item *item = mEquipment->getEquipment(i);
        if (item)
        {
            // Draw Item.
            Image *image = item->getImage();
            // Ensure the image is drawn with maximum opacity
            image->setAlpha(1.0f);
            g->drawImage(image,
                          mEquipBox[i].posX + 2,
                          mEquipBox[i].posY + 2);
            if (i == TmwAthena::EQUIP_PROJECTILE_SLOT)
            {
                g->setColor(Theme::getThemeColor(Theme::TEXT));
                graphics->drawText(toString(item->getQuantity()),
                                    mEquipBox[i].posX + (BOX_WIDTH / 2),
                                    mEquipBox[i].posY - getFont()->getHeight(),
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

Item *EquipmentWindow::getItem(int x, int y) const
{
    for (int i = 0; i < mBoxesNumber; ++i)
    {
        gcn::Rectangle tRect(mEquipBox[i].posX, mEquipBox[i].posY,
                                BOX_WIDTH, BOX_HEIGHT);

        if (tRect.isPointInRect(x, y))
            return mEquipment->getEquipment(i);
    }
    return 0;
}

std::string EquipmentWindow::getSlotName(int x, int y) const
{
    for (int i = 0; i < mBoxesNumber; ++i)
    {
        gcn::Rectangle tRect(mEquipBox[i].posX, mEquipBox[i].posY,
                             BOX_WIDTH, BOX_HEIGHT);

        if (tRect.isPointInRect(x, y))
            return mEquipment->getSlotName(i);
    }
    return std::string();
}

void EquipmentWindow::mousePressed(gcn::MouseEvent& mouseEvent)
{
    Window::mousePressed(mouseEvent);

    const int x = mouseEvent.getX();
    const int y = mouseEvent.getY();
    Item *item = 0;

    // Checks if any of the presses were in the equip boxes.
    for (int i = 0; i < mBoxesNumber; ++i)
    {
        item = mEquipment->getEquipment(i);
        gcn::Rectangle tRect(mEquipBox[i].posX, mEquipBox[i].posY,
                                BOX_WIDTH, BOX_HEIGHT);

        if (tRect.isPointInRect(x, y) && item)
        {
            setSelected(i);
            break;
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

void EquipmentWindow::mouseMoved(gcn::MouseEvent &event)
{
    const int x = event.getX();
    const int y = event.getY();

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    mouseX /= graphics->getScale();
    mouseY /= graphics->getScale();

    // Show ItemTooltip
    std::string slotName = getSlotName(x, y);
    if (!slotName.empty())
    {
        mItemPopup->setEquipmentText(slotName);

        Item *item = getItem(x, y);
        if (item)
        {
            mItemPopup->setItem(item->getInfo());
        }
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
    mItemPopup->setVisible(false);
}

void EquipmentWindow::setSelected(int index)
{
    mSelected = index;
    mUnequip->setEnabled(mSelected != -1);
}
