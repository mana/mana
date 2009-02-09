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

#include <guichan/actionlistener.hpp>

#include "partyhandler.h"
#include "protocol.h"
#include "messagein.h"

#include "../gui/chat.h"
#include "../gui/confirm_dialog.h"

#include "../beingmanager.h"
#include "../game.h"
#include "../party.h"

PartyHandler::PartyHandler(Party *party) : mParty(party)
{
    static const Uint16 _messages[] = {
        SMSG_PARTY_CREATE,
        SMSG_PARTY_INFO,
        SMSG_PARTY_INVITE,
        SMSG_PARTY_INVITED,
        SMSG_PARTY_SETTINGS,
        SMSG_PARTY_MEMBER_INFO,
        SMSG_PARTY_LEAVE,
        SMSG_PARTY_UPDATE_HP,
        SMSG_PARTY_UPDATE_COORDS,
        SMSG_PARTY_MESSAGE,
        0
    };
    handledMessages = _messages;
}

void PartyHandler::handleMessage(MessageIn *msg)
{
    switch (msg->getId())
    {
        case SMSG_PARTY_CREATE:
            mParty->createResponse(msg->readInt8());
            break;
        case SMSG_PARTY_INFO:
            break;
        case SMSG_PARTY_INVITE:
            {
                std::string nick = msg->readString(24);
                int status = msg->readInt8();
                mParty->inviteResponse(nick, status);
                break;
            }
        case SMSG_PARTY_INVITED:
            {
                int id = msg->readInt32();
                Being *being = beingManager->findBeing(id);
                if (being == NULL)
                {
                    break;
                }
                std::string nick;
                int gender = 0;
                std::string partyName = "";
                if (being->getType() != Being::PLAYER)
                {
                    nick = "";
                }
                else
                {
                    nick = being->getName();
                    gender = being->getGender();
                    partyName = msg->readString(24);
                }
                mParty->invitedAsk(nick, gender, partyName);
                break;
            }
        case SMSG_PARTY_SETTINGS:
            break;
        case SMSG_PARTY_MEMBER_INFO:
            break;
        case SMSG_PARTY_LEAVE:
            {
                /*int id = */msg->readInt32();
                std::string nick = msg->readString(24);
                /*int fail = */msg->readInt8();
                mParty->leftResponse(nick);
                break;
            }
        case SMSG_PARTY_UPDATE_HP:
            break;
        case SMSG_PARTY_UPDATE_COORDS:
            break;
        case SMSG_PARTY_MESSAGE:
            { // new block to enable local variables
                int msgLength = msg->readInt16() - 8;
                if (msgLength <= 0)
                {
                    return;
                }
                int id = msg->readInt32();
                Being *being = beingManager->findBeing(id);
                std::string chatMsg = msg->readString(msgLength);
                mParty->receiveChat(being, chatMsg);
            }
            break;
    }
}
