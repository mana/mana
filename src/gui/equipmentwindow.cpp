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

#include "gui/widgets/button.h"

#include "gui/equipmentwindow.h"
#include "gui/itempopup.h"
#include "gui/palette.h"
#include "gui/playerbox.h"
#include "gui/viewport.h"

#include "equipment.h"
#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>

static const int BOX_WIDTH = 36;
static const int BOX_HEIGHT = 36;

// Positions of the boxes, 2nd dimension is X and Y respectively.
static const int boxPosition[][2] = {
    { 50,  208 },   // EQUIP_LEGS_SLOT
    { 8,   123 },   // EQUIP_FIGHT1_SLOT
    { 8,   78 },    // EQUIP_GLOVES_SLOT
    { 129, 168 },   // EQUIP_RING2_SLOT
    { 8,   168 },   // EQUIP_RING1_SLOT
    { 129, 123 },   // EQUIP_FIGHT2_SLOT
    { 90,  208 },   // EQUIP_FEET_SLOT
    { 50,  40 },    // EQUIP_CAPE_SLOT
    { 70,  0 },     // EQUIP_HEAD_SLOT
    { 90,  40 },    // EQUIP_TORSO_SLOT
    { 129, 78 }     // EQUIP_AMMO_SLOT
};

#ifdef TMWSERV_SUPPORT
EquipmentWindow::EquipmentWindow(Equipment *equipment):
#else
EquipmentWindow::EquipmentWindow():
#endif
    Window(_("Equipment")),
#ifdef TMWSERV_SUPPORT
    mEquipment(equipment),
#endif
    mSelected(-1)
{
    mItemPopup = new ItemPopup;

    // Control that shows the Player
    PlayerBox *playerBox = new PlayerBox;
    playerBox->setDimension(gcn::Rectangle(50, 80, 74, 123));
    playerBox->setPlayer(player_node);

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

    for (int i = 0; i < EQUIP_VECTOREND; i++)
    {
        mEquipBox[i].posX = boxPosition[i][0] + getPadding();
        mEquipBox[i].posY = boxPosition[i][1] + getTitleBarHeight();
    }

#ifdef EATHENA_SUPPORT
    mEquipment = player_node->mEquipment.get();
    mInventory = player_node->getInventory();
#endif
}

EquipmentWindow::~EquipmentWindow()
{
    delete mItemPopup;
}

void EquipmentWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);

    Graphics *g = static_cast<Graphics*>(graphics);

    Window::drawChildren(graphics);

    for (int i = 0; i < EQUIP_VECTOREND; i++)
    {
        if (i == mSelected)
        {
            const gcn::Color color = guiPalette->getColor(Palette::HIGHLIGHT);

            // Set color to the highlight color
            g->setColor(gcn::Color(color.r, color.g, color.b, getGuiAlpha()));
            g->fillRectangle(gcn::Rectangle(mEquipBox[i].posX, mEquipBox[i].posY,
                                            BOX_WIDTH, BOX_HEIGHT));
        }

        // Set color black
        g->setColor(gcn::Color(0, 0, 0));
        // Draw box border
        g->drawRectangle(gcn::Rectangle(mEquipBox[i].posX, mEquipBox[i].posY,
                                        BOX_WIDTH, BOX_HEIGHT));

#ifdef TMWSERV_SUPPORT
        Item *item = mEquipment->getEquipment(i);
#else
        Item *item = (i != EQUIP_AMMO_SLOT) ?
               mInventory->getItem(mEquipment->getEquipment(i)) :
               mInventory->getItem(mEquipment->getArrows());
#endif
        if (item)
        {
            // Draw Item.
            Image *image = item->getImage();
            g->drawImage(image,
                         mEquipBox[i].posX + 2,
                         mEquipBox[i].posY + 2);
#ifdef EATHENA_SUPPORT
            if (i == EQUIP_AMMO_SLOT)
            {
                g->setColor(guiPalette->getColor(Palette::TEXT));
                graphics->drawText(toString(item->getQuantity()),
                                   mEquipBox[i].posX + (BOX_WIDTH / 2),
                                   mEquipBox[i].posY - getFont()->getHeight(),
                                   gcn::Graphics::CENTER);
            }
#endif
        }
    }
}

void EquipmentWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "unequip" && mSelected > -1)
    {
#ifdef TMWSERV_SUPPORT // TODO: merge these!
        Item *item = mEquipment->getEquipment(mSelected);
#else
        Item *item = (mSelected != EQUIP_AMMO_SLOT) ?
                     mInventory->getItem(mEquipment->getEquipment(mSelected)) :
                     mInventory->getItem(mEquipment->getArrows());
#endif
        Net::getInventoryHandler()->unequipItem(item);
        setSelected(-1);
    }
}

Item *EquipmentWindow::getItem(int x, int y) const
{
    for (int i = 0; i < EQUIP_VECTOREND; i++)
    {
        gcn::Rectangle tRect(mEquipBox[i].posX, mEquipBox[i].posY,
                             BOX_WIDTH, BOX_HEIGHT);

        if (tRect.isPointInRect(x, y))
        {
#ifdef TMWSERV_SUPPORT
            return mEquipment->getEquipment(i);
#else
            return (i != EQUIP_AMMO_SLOT) ?
                    mInventory->getItem(mEquipment->getEquipment(i)) :
                    mInventory->getItem(mEquipment->getArrows());
#endif
        }
    }
    return NULL;
}

void EquipmentWindow::mousePressed(gcn::MouseEvent& mouseEvent)
{
    Window::mousePressed(mouseEvent);

    const int x = mouseEvent.getX();
    const int y = mouseEvent.getY();

    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        // Checks if any of the presses were in the equip boxes.
        for (int i = 0; i < EQUIP_VECTOREND; i++)
        {
#ifdef TMWSERV_SUPPORT
            Item *item = mEquipment->getEquipment(i);
#else
            Item *item = (i != EQUIP_AMMO_SLOT) ?
                    mInventory->getItem(mEquipment->getEquipment(i)) :
                    mInventory->getItem(mEquipment->getArrows());
#endif
            gcn::Rectangle tRect(mEquipBox[i].posX, mEquipBox[i].posY,
                                 BOX_WIDTH, BOX_HEIGHT);

            if (tRect.isPointInRect(x, y) && item)
                setSelected(i);
        }
    }
    else if (mouseEvent.getButton() == gcn::MouseEvent::RIGHT)
    {
        if (Item *item = getItem(x, y))
        {
            /* Convert relative to the window coordinates to absolute screen
             * coordinates.
             */
            const int mx = x + getX();
            const int my = y + getY();
            viewport->showPopup(mx, my, item);
        }
    }
}

// Show ItemTooltip
void EquipmentWindow::mouseMoved(gcn::MouseEvent &event)
{
    const int x = event.getX();
    const int y = event.getY();

    Item *item = getItem(x, y);

    if (item)
    {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        mItemPopup->setItem(item->getInfo());
        mItemPopup->view(x + getX(), y + getY());
    }
    else
    {
        mItemPopup->setVisible(false);
    }
}

// Hide ItemTooltip
void EquipmentWindow::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}

void EquipmentWindow::setSelected(int index)
{
    mSelected = index;
    mUnequip->setEnabled(mSelected != -1);
}
