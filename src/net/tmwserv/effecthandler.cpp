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

#include "net/tmwserv/effecthandler.h"

#include "net/tmwserv/protocol.h"

#include "net/messagein.h"

#include "effectmanager.h"

namespace TmwServ {

EffectHandler::EffectHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_CREATE_EFFECT,
        0
    };
    handledMessages = _messages;
}

void EffectHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_CREATE_EFFECT:
            handleCreateEffects(msg);
            break;
        default:
            break;
    }
}

void EffectHandler::handleCreateEffects(MessageIn &msg)
{
     int id = msg.readInt16();
     Uint16 x = msg.readInt16();
     Uint16 y = msg.readInt16();
     effectManager->trigger(id, x, y);
}

} // namespace TmwServ
