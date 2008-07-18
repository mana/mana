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
 *  $Id: itemhandler.cpp 2150 2006-02-06 02:56:48Z der_doener $
 */

#include "itemhandler.h"

#include "messagein.h"
#include "protocol.h"

#include "../engine.h"
#include "../flooritemmanager.h"

ItemHandler::ItemHandler()
{
    static const Uint16 _messages[] = {
        SMSG_ITEM_VISIBLE,
        SMSG_ITEM_DROPPED,
        SMSG_ITEM_REMOVE,
        0
    };
    handledMessages = _messages;
}

void ItemHandler::handleMessage(MessageIn *msg)
{
    Uint32 id;
    Uint16 x, y;
    Sint16 itemId;

    switch (msg->getId())
    {
        case SMSG_ITEM_VISIBLE:
        case SMSG_ITEM_DROPPED:
            id = msg->readInt32();
            itemId = msg->readInt16();
            msg->readInt8();  // identify flag
            x = msg->readInt16();
            y = msg->readInt16();
            msg->skip(4);     // amount,subX,subY / subX,subY,amount

            floorItemManager->create(id, itemId, x, y, engine->getCurrentMap());
            break;

        case SMSG_ITEM_REMOVE:
            FloorItem *item;
            item = floorItemManager->findById(msg->readInt32());
            if (item)
                floorItemManager->destroy(item);
            break;
    }
}
