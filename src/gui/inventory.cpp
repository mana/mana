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
 */

#include "inventory.h"
#include <sstream>

InventoryDialog::InventoryDialog(gcn::Container *parent):
    Window(parent, "Inventory")
{
    setSize(322, 60);

    itemset = load_datafile("./data/graphic/items.dat");

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        items[i].id = -1;
        items[i].quantity = 0;
    }
}

InventoryDialog::~InventoryDialog()
{
}

void InventoryDialog::draw(gcn::Graphics *graphics)
{
    int x, y;
    getAbsolutePosition(x, y);

    // Draw window graphics
    Window::draw(graphics);

    gcn::AllegroGraphics *alGraphics = (gcn::AllegroGraphics*)graphics;
    BITMAP *target = alGraphics->getTarget();

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].quantity > 0) {
            if (items[i].id >= 501 && items[i].id <= 511) {
                draw_rle_sprite(target,
                        (RLE_SPRITE *)itemset[items[i].id - 501].dat,
                        x + 24 * i,
                        y + 26);
            }
            //else {
            //    masked_blit((BITMAP *)itemset[0].dat, gui_bitmap, 0, 0,
            //            x + 24 * i, y + 26, 22, 22);
            //}

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
                    y + itemMeny_y, MAKECOL_BLACK, -1, "Use item");
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
                    y + itemMeny_y + 10, MAKECOL_BLACK, -1, "Del item");
        }
    }
    */
}


int InventoryDialog::addItem(int index, int id, int quantity) {
    items[index].id = id;
    items[index].quantity += quantity;
    return 0;
}

int InventoryDialog::removeItem(int id) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].id == id) {
            items[i].id = -1;
            items[i].quantity = 0;
        }
    }
    return 0;
}

int InventoryDialog::changeQuantity(int index, int quantity) {
    items[index].quantity = quantity;
    return 0;
}

int InventoryDialog::increaseQuantity(int index, int quantity) {
    items[index].quantity += quantity;
    return 0;
}

int InventoryDialog::useItem(int index, int id) {
    WFIFOW(0) = net_w_value(0x00a7);
    WFIFOW(2) = net_w_value(index);
    WFIFOL(4) = net_l_value(id);
    // Note: id is dest of item, usually player_node->account_ID
    WFIFOSET(8);
    while ((out_size > 0)) flush();
    return 0;
}

int InventoryDialog::dropItem(int index, int amunt) {
    WFIFOW(0) = net_w_value(0x00a7);
    WFIFOW(2) = net_w_value(index);
    WFIFOL(4) = net_l_value(amunt);
    WFIFOSET(8);
    while ((out_size > 0)) flush();
    return 0;
}

