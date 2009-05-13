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
#include "log.h"

#include "gui/chat.h"
#include "gui/partywindow.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "net/ea/protocol.h"
#include "net/ea/gui/partytab.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

PartyTab *partyTab = 0;
Net::PartyHandler *partyHandler = 0;

namespace EAthena {

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
                partyTab->chatLog(_("Could not create party."), BY_SERVER);
            else
            {
                partyTab->chatLog(_("Party successfully created."), BY_SERVER);
                player_node->setInParty(true);
                partyWindow->setVisible(true);
            }
            break;
        case SMSG_PARTY_INFO:
            {
                if (!partyWindow)
                    break;

                partyWindow->clearMembers();

                int length = msg.readInt16();
                std::string party = msg.readString(24);
                partyWindow->setPartyName(party);
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
            {
                if (!partyTab)
                    break;

                // These seem to indicate the sharing mode for exp and items
                short exp = msg.readInt16();
                short item = msg.readInt16();

                switch (exp) {
                    case PARTY_SHARE:
                        if (mShareExp == PARTY_SHARE)
                            break;
                        mShareExp = PARTY_SHARE;
                        partyTab->chatLog(_("Experience sharing enabled."), BY_SERVER);
                        break;
                    case PARTY_SHARE_NO:
                        if (mShareExp == PARTY_SHARE_NO)
                            break;
                        mShareExp =PARTY_SHARE_NO;
                        partyTab->chatLog(_("Experience sharing disabled."), BY_SERVER);
                        break;
                    case PARTY_SHARE_NOT_POSSIBLE:
                        if (mShareExp == PARTY_SHARE_NOT_POSSIBLE)
                            break;
                        mShareExp = PARTY_SHARE_NOT_POSSIBLE;
                        partyTab->chatLog(_("Experience sharing not possible."), BY_SERVER);
                        break;
                    default:
                        logger->log("Unknown party exp option: %d\n", exp);
                }

                switch (item) {
                    case PARTY_SHARE:
                        if (mShareItems == PARTY_SHARE)
                            break;
                        mShareItems = PARTY_SHARE;
                        partyTab->chatLog(_("Item sharing enabled."), BY_SERVER);
                        break;
                    case PARTY_SHARE_NO:
                        if (mShareItems == PARTY_SHARE_NO)
                            break;
                        mShareItems =PARTY_SHARE_NO;
                        partyTab->chatLog(_("Item sharing disabled."), BY_SERVER);
                        break;
                    case PARTY_SHARE_NOT_POSSIBLE:
                        if (mShareItems == PARTY_SHARE_NOT_POSSIBLE)
                            break;
                        mShareItems = PARTY_SHARE_NOT_POSSIBLE;
                        partyTab->chatLog(_("Item sharing not possible."), BY_SERVER);
                        break;
                    default:
                        logger->log("Unknown party item option: %d\n", exp);
                }
                break;
            }
        case SMSG_PARTY_MOVE:
            {
                msg.readInt32();    // id
                msg.skip(4);
                msg.readInt16();    // x
                msg.readInt16();    // y
                msg.readInt8();     // online (if 0)
                msg.readString(24); // party
                msg.readString(24); // nick
                msg.readString(16); // map
            }
            break;
        case SMSG_PARTY_LEAVE:
            {
                int id = msg.readInt32();
                std::string nick = msg.readString(24);
                msg.readInt8();     // fail
                if (id == player_node->getId())
                {
                    partyWindow->clearMembers();
                    partyWindow->setVisible(false);
                    partyTab->chatLog(_("You have left the party."), BY_SERVER);
                }
                else
                {
                    partyTab->chatLog(strprintf(_("%s has left your party."),
                                    nick.c_str()), BY_SERVER);
                    partyWindow->removeMember(id);
                }
                break;
            }
        case SMSG_PARTY_UPDATE_HP:
            {
                int id = msg.readInt32();
                int hp = msg.readInt16();
                int maxhp = msg.readInt16();
                partyWindow->updateMemberHP(id, hp, maxhp);
            }
            break;
        case SMSG_PARTY_UPDATE_COORDS:
            {
                msg.readInt32(); // id
                msg.readInt16(); // x
                msg.readInt16(); // y
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
    // TODO?
}

void PartyHandler::invite(Player *player)
{
    MessageOut outMsg(CMSG_PARTY_INVITE);
    outMsg.writeInt32(player->getId());
}

void PartyHandler::invite(const std::string &name)
{
    partyTab->chatLog(_("Inviting like this isn't supported at the moment."), BY_SERVER);
    // TODO?
}

void PartyHandler::inviteResponse(const std::string &inviter, bool accept)
{
    MessageOut outMsg(CMSG_PARTY_INVITED);
    outMsg.writeInt32(player_node->getId());
    outMsg.writeInt32(accept ? 1 : 0);
    player_node->setInParty(player_node->isInParty() || accept);
}

void PartyHandler::leave()
{
    MessageOut outMsg(CMSG_PARTY_LEAVE);
}

void PartyHandler::kick(Player *player)
{
    MessageOut outMsg(CMSG_PARTY_KICK);
    outMsg.writeInt32(player->getId());
    outMsg.writeString("", 24); //Unused
}

void PartyHandler::kick(const std::string &name)
{
    int id = partyWindow->findMember(name);
    if (id == -1)
    {
        partyTab->chatLog(strprintf(_("%s is not in your party!"), name.c_str()),
                          BY_SERVER);
        return;
    }

    MessageOut outMsg(CMSG_PARTY_KICK);
    outMsg.writeInt32(id);
    outMsg.writeString(name, 24); //Unused
}

void PartyHandler::chat(const std::string &text)
{
    MessageOut outMsg(CMSG_PARTY_MESSAGE);
    outMsg.writeInt16(text.length() + 4);
    outMsg.writeString(text, text.length());
}

void PartyHandler::requestPartyMembers()
{
    // Our eAthena doesn't have this message
    // Not needed anyways
}

void PartyHandler::setShareExperience(PartyShare share)
{
    if (share == PARTY_SHARE_NOT_POSSIBLE)
        return;

    MessageOut outMsg(CMSG_PARTY_SETTINGS);
    outMsg.writeInt16(share);
    outMsg.writeInt16(mShareItems);
}

void PartyHandler::setShareItems(PartyShare share)
{
    if (share == PARTY_SHARE_NOT_POSSIBLE)
        return;

    MessageOut outMsg(CMSG_PARTY_SETTINGS);
    outMsg.writeInt16(mShareExp);
    outMsg.writeInt16(share);
}

} // namespace EAthena
