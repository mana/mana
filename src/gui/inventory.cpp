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
#include <sstream>

InventoryWindow::InventoryWindow():
    Window("Inventory")
{
    setSize(322, 60);

    ResourceManager *resman = ResourceManager::getInstance();
    Image *itemImg = resman->getImage("graphic/items.bmp");
    if (!itemImg) error("Unable to load items.bmp");
    itemset = new Spriteset(itemImg, 20, 20);

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        items[i].id = -1;
        items[i].quantity = 0;
    }
}

InventoryWindow::~InventoryWindow()
{
}

void InventoryWindow::draw(gcn::Graphics *graphics)
{
    int x, y;
    getAbsolutePosition(x, y);

    // Draw window graphics
    Window::draw(graphics);

    BITMAP *target = ((Graphics*)graphics)->getTarget();

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].quantity > 0) {
            if (items[i].id >= 501 && items[i].id <= 511) {
                itemset->spriteset[items[i].id - 501]->draw(target,
                        x + 24 * i,
                        y + 26);
            }

            std::stringstream ss;
            ss << items[i].quantity;
            graphics->drawText(ss.str(), 24 * i + 10, 44,
                    gcn::Graphics::CENTER);
        }
    }

    /*
    if (mouse_b & 2) {
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            if (items[i].quantity > 0 &&
                    x + 24 * i + 24 > mouse_x &&
                    x + 24 * i < mouse_x &&
                    y + 44 + 24 > mouse_y &&
                    y + 44 < mouse_y)
            {
                itemMeny = 1;
                itemMeny_x = 24 * i;
                itemMeny_y = 44 + 24;
                itemMeny_i = i;
            }
        }
    }

    if (itemMeny) {
        if (y + itemMeny_y < mouse_y && y + itemMeny_y + 10 > mouse_y)
        {
            if (mouse_b & 1) {
                useItem(itemMeny_i,items[itemMeny_i].id);
                itemMeny = 0;
            }
            textprintf_ex(buffer, font, x + itemMeny_x,
                    y + itemMeny_y, makecol(255, 237, 33), -1, "Use item");
        }
        else {
            textprintf_ex(buffer, font, x + itemMeny_x,
                    y + itemMeny_y, makecol(0,0,0), -1, "Use item");
        }
        if (y + itemMeny_y + 10 < mouse_y && y + itemMeny_y + 20 > mouse_y) {
            if (mouse_b & 1) {
                dropItem(itemMeny_i, 1);
                itemMeny = 0;
            }
            textprintf_ex(buffer, font, x + itemMeny_x,
                    y + itemMeny_y + 10,
                    makecol(255, 237, 33), -1, "Del item");
        }
        else {
            textprintf_ex(buffer, font, x + itemMeny_x,
                    y + itemMeny_y + 10, makecol(0,0,0), -1, "Del item");
        }
    }
    */
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
    // Note: id is dest of item, usually player_node->account_ID
    WFIFOSET(8);
    while ((out_size > 0)) flush();
    return 0;
}

int InventoryWindow::dropItem(int index, int amunt) {
    WFIFOW(0) = net_w_value(0x00a7);
    WFIFOW(2) = net_w_value(index);
    WFIFOL(4) = net_l_value(amunt);
    WFIFOSET(8);
    while ((out_size > 0)) flush();
    return 0;
}

