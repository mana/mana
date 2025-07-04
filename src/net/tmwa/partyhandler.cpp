/*
 *  The Mana Client
 *  Copyright (C) 2008  Lloyd Bryant <lloyd_bryant@netzero.net>
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "net/tmwa/partyhandler.h"

#include "actorspritemanager.h"
#include "event.h"
#include "localplayer.h"
#include "log.h"
#include "party.h"

#include "gui/socialwindow.h"

#include "net/tmwa/messagein.h"
#include "net/tmwa/messageout.h"
#include "net/tmwa/protocol.h"

#include "net/tmwa/gui/partytab.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#define PARTY_ID 1

extern Net::PartyHandler *partyHandler;

namespace TmwAthena {

PartyTab *partyTab = nullptr;
Party *taParty;

PartyHandler::PartyHandler():
        mShareExp(PARTY_SHARE_UNKNOWN), mShareItems(PARTY_SHARE_UNKNOWN)
{
    static const Uint16 _messages[] = {
        SMSG_PARTY_CREATE,
        SMSG_PARTY_INFO,
        SMSG_PARTY_INVITE_RESPONSE,
        SMSG_PARTY_INVITED,
        SMSG_PARTY_SETTINGS,
        SMSG_PARTY_LEAVE,
        SMSG_PARTY_UPDATE_HP,
        SMSG_PARTY_UPDATE_COORDS,
        SMSG_PARTY_MESSAGE,
        0
    };
    handledMessages = _messages;
    partyHandler = this;
    taParty = Party::getParty(1);
}

PartyHandler::~PartyHandler()
{
    delete partyTab;
    partyTab = nullptr;
}

void PartyHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_PARTY_CREATE:
            if (msg.readInt8())
                serverNotice(_("Could not create party."));
            else
                serverNotice(_("Party successfully created."));
            break;
        case SMSG_PARTY_INFO:
            {
                taParty->clearMembers();

                int length = msg.readInt16();
                taParty->setName(msg.readString(24));
                int count = (length - 28) / 46;

                for (int i = 0; i < count; i++)
                {
                    int id = msg.readInt32();
                    std::string nick = msg.readString(24);
                    std::string map = msg.readString(16);
                    bool leader = msg.readInt8() == 0;
                    bool online = msg.readInt8() == 0;

                    PartyMember *member = taParty->addMember(id, nick);
                    member->setLeader(leader);
                    member->setOnline(online);
                }

                local_player->setParty(taParty);
            }
            break;
        case SMSG_PARTY_INVITE_RESPONSE:
            {
                if (!partyTab)
                    break;

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
                std::string partyName = msg.readString(24);
                std::string nick;
                Being *being;

                if ((being = actorSpriteManager->findBeing(id)))
                {
                    nick = being->getName();
                }

                socialWindow->showPartyInvite(nick, partyName);
                break;
            }
        case SMSG_PARTY_SETTINGS:
            {
                if (!partyTab)
                {
                    if (!chatWindow)
                        break;

                    partyTab = new PartyTab();
                }

                // These seem to indicate the sharing mode for exp and items
                short exp = msg.readInt16();
                short item = msg.readInt16();

                switch (exp)
                {
                    case PARTY_SHARE:
                        if (mShareExp == PARTY_SHARE)
                            break;
                        mShareExp = PARTY_SHARE;
                        partyTab->chatLog(_("Experience sharing enabled."), BY_SERVER);
                        break;
                    case PARTY_SHARE_NO:
                        if (mShareExp == PARTY_SHARE_NO)
                            break;
                        mShareExp = PARTY_SHARE_NO;
                        partyTab->chatLog(_("Experience sharing disabled."), BY_SERVER);
                        break;
                    case PARTY_SHARE_NOT_POSSIBLE:
                        if (mShareExp == PARTY_SHARE_NOT_POSSIBLE)
                            break;
                        mShareExp = PARTY_SHARE_NOT_POSSIBLE;
                        partyTab->chatLog(_("Experience sharing not possible."), BY_SERVER);
                        break;
                    default:
                        Log::info("Unknown party exp option: %d", exp);
                }

                switch (item)
                {
                    case PARTY_SHARE:
                        if (mShareItems == PARTY_SHARE)
                            break;
                        mShareItems = PARTY_SHARE;
                        partyTab->chatLog(_("Item sharing enabled."), BY_SERVER);
                        break;
                    case PARTY_SHARE_NO:
                        if (mShareItems == PARTY_SHARE_NO)
                            break;
                        mShareItems = PARTY_SHARE_NO;
                        partyTab->chatLog(_("Item sharing disabled."), BY_SERVER);
                        break;
                    case PARTY_SHARE_NOT_POSSIBLE:
                        if (mShareItems == PARTY_SHARE_NOT_POSSIBLE)
                            break;
                        mShareItems = PARTY_SHARE_NOT_POSSIBLE;
                        partyTab->chatLog(_("Item sharing not possible."), BY_SERVER);
                        break;
                    default:
                        Log::info("Unknown party item option: %d", exp);
                }
                break;
            }
        case SMSG_PARTY_LEAVE:
            {
                int id = msg.readInt32();
                std::string nick = msg.readString(24);
                msg.readInt8();     // fail
                if (id == local_player->getId())
                {
                    taParty->removeFromMembers();
                    taParty->clearMembers();
                    serverNotice(_("You have left the party."));
                    if (partyTab)
                    {
                        delete partyTab;
                        partyTab = nullptr;
                    }
                    socialWindow->removeTab(taParty);
                }
                else
                {
                    partyTab->chatLog(strprintf(_("%s has left your party."),
                                    nick.c_str()), BY_SERVER);

                    if (Being *b = actorSpriteManager->findBeing(id))
                    {
                        b->setParty(nullptr);
                    }

                    taParty->removeMember(id);
                }
                break;
            }
        case SMSG_PARTY_UPDATE_HP:
            {
                int id = msg.readInt32();
                int hp = msg.readInt16();
                int maxhp = msg.readInt16();
                PartyMember *m = taParty->getMember(id);
                if (m)
                {
                    m->setHp(hp);
                    m->setMaxHp(maxhp);
                }

                // The server only sends this when the member is in range, so
                // lets make sure they get the party hilight.
                if (Being *b = actorSpriteManager->findBeing(id))
                {
                    b->setParty(taParty);
                }
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

                PartyMember *member = taParty->getMember(id);
                if (member)
                    partyTab->chatLog(member->getName(), chatMsg);
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

void PartyHandler::invite(Being *being)
{
    MessageOut outMsg(CMSG_PARTY_INVITE);
    outMsg.writeInt32(being->getId());
}

void PartyHandler::invite(const std::string &name)
{
    Being *invitee = actorSpriteManager->findBeingByName(name, Being::PLAYER);

    if (invitee)
    {
        invite(invitee);
        partyTab->chatLog(strprintf(_("Invited user %s to party."),
                                    invitee->getName().c_str()), BY_SERVER);
    }
    else if (partyTab)
    {
        partyTab->chatLog(strprintf(_("Inviting failed, because you can't see "
                            "a player called %s."), name.c_str()), BY_SERVER);
    }
    else
    {
        serverNotice(_("You can only invite when you are in a party!"));
    }
}

void PartyHandler::inviteResponse(const std::string &inviter, bool accept)
{
    MessageOut outMsg(CMSG_PARTY_INVITED);
    outMsg.writeInt32(local_player->getId());
    outMsg.writeInt32(accept ? 1 : 0);
}

void PartyHandler::leave()
{
    MessageOut outMsg(CMSG_PARTY_LEAVE);
}

void PartyHandler::kick(Being *being)
{
    MessageOut outMsg(CMSG_PARTY_KICK);
    outMsg.writeInt32(being->getId());
    outMsg.writeString("", 24); //Unused
}

void PartyHandler::kick(const std::string &name)
{
    PartyMember *m = taParty->getMember(name);
    if (!m)
    {
        partyTab->chatLog(strprintf(_("%s is not in your party!"), name.c_str()),
                          BY_SERVER);
        return;
    }

    MessageOut outMsg(CMSG_PARTY_KICK);
    outMsg.writeInt32(m->getID());
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

} // namespace TmwAthena
