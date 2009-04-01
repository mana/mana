/*
 *  The Mana World
 *  Copyright (C) 2008  Lloyd Bryant <lloyd_bryant@netzero.net>
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

#include "net/ea/partyhandler.h"

#include "beingmanager.h"
#include "localplayer.h"

#include "gui/chat.h"
#include "gui/partywindow.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "net/ea/protocol.h"
#include "net/ea/gui/partytab.h"

#include "utils/gettext.h"
#include "utils/strprintf.h"

PartyTab *partyTab;

static void newPartyTab() { partyTab = new PartyTab(); }
static void deletePartyTab() { delete partyTab ; }

PartyHandler *partyHandler;

PartyHandler::PartyHandler()
{
    static const Uint16 _messages[] = {
        SMSG_PARTY_CREATE,
        SMSG_PARTY_INFO,
        SMSG_PARTY_INVITE_RESPONSE,
        SMSG_PARTY_INVITED,
        SMSG_PARTY_SETTINGS,
        SMSG_PARTY_MOVE,
        SMSG_PARTY_LEAVE,
        SMSG_PARTY_UPDATE_HP,
        SMSG_PARTY_UPDATE_COORDS,
        SMSG_PARTY_MESSAGE,
        0
    };
    handledMessages = _messages;
    partyHandler = this;

    //newPartyTab();
}

PartyHandler::~PartyHandler()
{
    //deletePartyTab();
}

void PartyHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_PARTY_CREATE:
            if (msg.readInt8())
            {
                partyTab->chatLog(_("Party successfully created."), BY_SERVER);
                player_node->setInParty(true);
            }
            else
                partyTab->chatLog(_("Could not create party."), BY_SERVER);
            break;
        case SMSG_PARTY_INFO:
            {
                int length = msg.readInt16();
                std::string party = msg.readString(24);
                int count = (length - 28) / 46;

                for (int i = 0; i < count; i++)
                {
                    int id = msg.readInt32();
                    std::string nick = msg.readString(24);
                    std::string map = msg.readString(16);
                    bool leader = msg.readInt8() == 0;
                    bool online = msg.readInt8() == 0;

                    partyWindow->updateMember(id, nick, leader, online);
                }
            }
            break;
        case SMSG_PARTY_INVITE_RESPONSE:
            {
                std::string nick = msg.readString(24);
                switch (msg.readInt8())
                {
                    case 0:
                        partyTab->chatLog(strprintf(_("%s is already a member of a party."),
                                    nick.c_str()), BY_SERVER);
                        break;
                    case 1:
                        partyTab->chatLog(strprintf(_("%s refused your invitation."),
                                    nick.c_str()), BY_SERVER);
                        break;
                    case 2:
                        partyTab->chatLog(strprintf(_("%s is now a member of your party."),
                                    nick.c_str()), BY_SERVER);
                        break;
                    default:
                        partyTab->chatLog(strprintf(_("Unknown invite response for %s."),
                                    nick.c_str()), BY_SERVER);
                        break;
                }
                break;
            }
        case SMSG_PARTY_INVITED:
            {
                int id = msg.readInt32();
                Being *being = beingManager->findBeing(id);
                if (!being)
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
                    partyName = msg.readString(24);
                }
                partyWindow->showPartyInvite(nick, partyName);
                break;
            }
        case SMSG_PARTY_SETTINGS:
            // I don't see this in eAthena's source
            printf("Party settings!\n");
            break;
        case SMSG_PARTY_MOVE:
            {
                int id = msg.readInt32();
                msg.skip(4);
                int x = msg.readInt16();
                int y = msg.readInt16();
                bool online = msg.readInt8() == 0;
                std::string party = msg.readString(24);
                std::string nick = msg.readString(24);
                std::string map = msg.readString(16);
            }
            break;
        case SMSG_PARTY_LEAVE:
            {
                int id = msg.readInt32();
                std::string nick = msg.readString(24);
                int fail = msg.readInt8();
                partyTab->chatLog(strprintf(_("%s has left your party."),
                                    nick.c_str()), BY_SERVER);
                partyWindow->removeMember(id);
                break;
            }
        case SMSG_PARTY_UPDATE_HP:
            {
                int id = msg.readInt32();
                int hp = msg.readInt16();
                int hpMax = msg.readInt16();
            }
            break;
        case SMSG_PARTY_UPDATE_COORDS:
            {
                int id = msg.readInt32();
                int x = msg.readInt16();
                int y = msg.readInt16();
            }
            break;
        case SMSG_PARTY_MESSAGE:
            {
                int msgLength = msg.readInt16() - 8;
                if (msgLength <= 0)
                {
                    return;
                }
                int id = msg.readInt32();
                std::string chatMsg = msg.readString(msgLength);

                Being *being = beingManager->findBeing(id);
                if (being)
                    being->setSpeech(chatMsg, SPEECH_TIME);

                PartyMember *member = partyWindow->findMember(id);
                if (member)                
                    partyTab->chatLog(member->name, chatMsg);
                else
                    partyTab->chatLog(strprintf(_("An unknown member tried to "
                                    "say: %s"), chatMsg.c_str()), BY_SERVER);
            }
            break;
    }
}

void PartyHandler::create(const std::string &name)
{
    MessageOut outMsg(CMSG_PARTY_CREATE);
    outMsg.writeString(name.substr(0, 23), 24);
}

void PartyHandler::join(int partyId)
{
}

void PartyHandler::invite(int playerId)
{
    MessageOut outMsg(CMSG_PARTY_INVITE);
    outMsg.writeInt32(playerId);
}

void PartyHandler::inviteResponse(bool accept)
{
    MessageOut outMsg(CMSG_PARTY_INVITED);
    outMsg.writeInt32(player_node->getId());
    outMsg.writeInt32(accept ? 1 : 0);
    player_node->setInParty(player_node->getInParty() || accept);
}

void PartyHandler::leave()
{
    MessageOut outMsg(CMSG_PARTY_LEAVE);
    partyTab->chatLog(_("Left party."), BY_SERVER);
    player_node->setInParty(false);
}

void PartyHandler::kick(int playerId)
{
    // TODO
}

void PartyHandler::chat(const std::string &text)
{
    MessageOut outMsg(CMSG_PARTY_MESSAGE);
    outMsg.writeInt16(text.length() + 4);
    outMsg.writeString(text, text.length());
}
