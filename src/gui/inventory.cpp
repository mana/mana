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

#include "../graphic/graphic.h"
#include "inventory.h"
#include "../resources/resourcemanager.h"
#include "../resources/image.h"
#include "button.h"
#include "../being.h"
#include <sstream>

InventoryWindow::InventoryWindow():
    Window("Inventory")
{
    setSize(322, 80);
    useButton = new Button("Use");
    useButton->setPosition(20, 50);
    dropButton = new Button("Drop");
    dropButton->setPosition(60, 50);
    
    useButton->setEventId("use");
    dropButton->setEventId("drop");
    useButton->addActionListener(this);
    dropButton->addActionListener(this);
    
    add(useButton);
    add(dropButton);  

    ResourceManager *resman = ResourceManager::getInstance();
    Image *itemImg = resman->getImage("graphic/items.bmp");
    if (!itemImg) error("Unable to load items.bmp");
    itemset = new Spriteset(itemImg, 20, 20);

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        items[i].id = -1;
        items[i].quantity = 0;
    }
    
    selectedItem = 4; /**< No item selected */
}

InventoryWindow::~InventoryWindow()
{
    delete useButton;
    delete dropButton;
}

void InventoryWindow::draw(gcn::Graphics *graphics)
{
    int x, y;
    getAbsolutePosition(x, y);
    
    if(items[selectedItem].quantity <= 0)
        selectedItem = -1;

    // Draw window graphics
    Window::draw(graphics);

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].quantity > 0) {
            if (items[i].id >= 501 && items[i].id <= 511) {
                itemset->spriteset[items[i].id - 501]->draw(screen,
                        x + 24 * i,
                        y + 26);
            }

            std::stringstream ss;
            ss << items[i].quantity;
            graphics->drawText(ss.str(), 24 * i + 10, 54,
                    gcn::Graphics::CENTER);
        }
    }
    
    if (selectedItem >= 0) {
        graphics->drawRectangle(gcn::Rectangle(24 * selectedItem + 1, 26,
            22, 22));
    }
    
}


int InventoryWindow::addItem(int index, int id, int quantity) {
    items[index].id = id;
    items[index].quantity += quantity;
    return 0;
}

int InventoryWindow::removeItem(int id) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].id == id) {
            items[i].id = -1;
            items[i].quantity = 0;
        }
    }
    return 0;
}

int InventoryWindow::changeQuantity(int index, int quantity) {
    items[index].quantity = quantity;
    return 0;
}

int InventoryWindow::increaseQuantity(int index, int quantity) {
    items[index].quantity += quantity;
    return 0;
}

int InventoryWindow::useItem(int index, int id) {
    WFIFOW(0) = net_w_value(0x00a7);
    WFIFOW(2) = net_w_value(index);
    WFIFOL(4) = net_l_value(id);
    // Note: id is dest of item, usually player_node->account_ID ??
    WFIFOSET(8);
    while ((out_size > 0)) flush();
    return 0;
}

int InventoryWindow::dropItem(int index, int quantity) {
    WFIFOW(0) = net_w_value(0x00a2);
    WFIFOW(2) = net_w_value(index);
    WFIFOW(4) = net_w_value(quantity);
    WFIFOSET(6);
    while ((out_size > 0)) flush();
    return 0;
}

void InventoryWindow::action(const std::string &eventId)
{
    if(selectedItem >= 0 && selectedItem <= INVENTORY_SIZE) {
        if (eventId == "use") {
            useItem(selectedItem, items[selectedItem].id);
        } else if (eventId == "drop") {
            dropItem(selectedItem, items[selectedItem].quantity);
            // Temp: drop all the items, you should choose quantity instead
        }       
    }
}

void InventoryWindow::mousePress(int mx, int my, int button) {
    if (button == gcn::MouseInput::LEFT)
        selectedItem = mx / 24;
    if (selectedItem > INVENTORY_SIZE)
        selectedItem = INVENTORY_SIZE;
    
    Window::mousePress(mx, my, button);
}

