/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "net/manaserv/effecthandler.h"

#include "beingmanager.h"
#include "effectmanager.h"
#include "log.h"

#include "net/messagein.h"

#include "net/manaserv/protocol.h"

namespace ManaServ {

EffectHandler::EffectHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_CREATE_EFFECT_POS,
        GPMSG_CREATE_EFFECT_BEING,
        0
    };
    handledMessages = _messages;
}

void EffectHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_CREATE_EFFECT_POS:
            handleCreateEffectPos(msg);
            break;
        case GPMSG_CREATE_EFFECT_BEING:
            handleCreateEffectBeing(msg);
            break;
        default:
            break;
    }
}

void EffectHandler::handleCreateEffectPos(Net::MessageIn &msg)
{
    int id = msg.readInt16();
    Uint16 x = msg.readInt16();
    Uint16 y = msg.readInt16();
    effectManager->trigger(id, x, y);
}

void EffectHandler::handleCreateEffectBeing(Net::MessageIn &msg)
{
    int eid = msg.readInt16();
    int bid = msg.readInt16();
    Being* b = beingManager->findBeing(bid);
    if (b)
        effectManager->trigger(eid, b);
    else
        logger->log("Warning: CreateEffect called for unknown being #%d", bid);
}

} // namespace ManaServ
