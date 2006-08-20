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

#include "maploginhandler.h"

#include "messagein.h"
#include "protocol.h"

#include "../localplayer.h"
#include "../log.h"
#include "../main.h"

MapLoginHandler::MapLoginHandler()
{
    static const Uint16 _messages[] = {
        SMSG_LOGIN_SUCCESS,
        0x0081,
        0
    };
    handledMessages = _messages;
}

void MapLoginHandler::handleMessage(MessageIn *msg)
{
    //unsigned char direction;

    switch (msg->getId())
    {
        case SMSG_LOGIN_SUCCESS:
            msg->readLong();   // server tick
            //logger->log("Protocol: Player start position: (%d, %d), Direction: %d",
            //        player_node->mX, player_node->mY, direction);
            state = STATE_GAME;
            break;

        case 0x0081:
            logger->log("Warning: Map server D/C");
            state = STATE_ERROR;
            break;
    }
}
