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

#include "npc.h"
#include "button.h"
#include "scrollarea.h"
#include "listbox.h"
#include "../game.h"

NpcListDialog::NpcListDialog():
    Window("NPC")
{
    itemList = new ListBox(this);
    scrollArea = new ScrollArea(itemList);
    okButton = new Button("OK");
    cancelButton = new Button("Cancel");

    setSize(260, 175);
    scrollArea->setDimension(gcn::Rectangle(
                5, 5, 250, 160 - okButton->getHeight()));
    cancelButton->setPosition(
            260 - 5 - cancelButton->getWidth(),
            175 - 5 - cancelButton->getHeight());
    okButton->setPosition(
            cancelButton->getX() - 5 - okButton->getWidth(),
            cancelButton->getY());

    itemList->setEventId("item");
    okButton->setEventId("ok");
    cancelButton->setEventId("cancel");

    itemList->addActionListener(this);
    okButton->addActionListener(this);
    cancelButton->addActionListener(this);

    add(scrollArea);
    add(okButton);
    add(cancelButton);

    setLocationRelativeTo(getParent());
}

NpcListDialog::~NpcListDialog()
{
    delete okButton;
    delete cancelButton;
    delete itemList;
    delete scrollArea;
}

int NpcListDialog::getNumberOfElements()
{
    return items.size();
}

std::string NpcListDialog::getElementAt(int i)
{
    return items[i];
}

void NpcListDialog::parseItems(const char *string) {
    char *copy = new char[strlen(string) + 1];
    strcpy(copy, string);

    char *token = strtok(copy, ":");
    while (token != NULL) {
        char *temp = (char*)malloc(strlen(token) + 1);
        strcpy(temp, token);
        items.push_back(std::string(temp));
        token = strtok(NULL, ":");
    }
    
    delete[] copy;
} 

void NpcListDialog::reset() {
    items.clear();
}

void NpcListDialog::action(const std::string& eventId)
{
    if (eventId == "ok") {
        // Send the selected index back to the server
        int selectedIndex = itemList->getSelected();
        if (selectedIndex > -1) {
            WFIFOW(0) = net_w_value(0x00b8);
            WFIFOL(2) = net_l_value(current_npc);
            WFIFOB(6) = net_b_value(selectedIndex + 1);
            WFIFOSET(7);
            setVisible(false);
            current_npc = 0;
            reset();
           
        }
    }
    else if (eventId == "cancel") {
        // 0xff packet means cancel
        WFIFOW(0) = net_w_value(0x00b8);
        WFIFOL(2) = net_l_value(current_npc);
        WFIFOB(6) = net_b_value(0xff);
        WFIFOSET(7);
        setVisible(false);
        reset();
        current_npc = 0;
    }
}
