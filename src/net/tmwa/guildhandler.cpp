/*
 *  The Mana Client
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

#include "net/tmwa/guildhandler.h"

#include "guild.h"
#include "event.h"
#include "localplayer.h"
#include "log.h"

#include "gui/socialwindow.h"

#include "net/tmwa/messagein.h"
#include "net/tmwa/protocol.h"

#include "net/tmwa/gui/guildtab.h"

#include "utils/gettext.h"

extern Net::GuildHandler *guildHandler;

namespace TmwAthena {

GuildTab *guildTab = 0;
Guild *taGuild;

GuildHandler::GuildHandler()
{
    static const Uint16 _messages[] = {
/*
        SMSG_GUILD_CREATE_RESPONSE,
        SMSG_GUILD_POSITION_INFO,
        SMSG_GUILD_MEMBER_LOGIN,
        SMSG_GUILD_MASTER_OR_MEMBER,
        SMSG_GUILD_BASIC_INFO,
        SMSG_GUILD_ALIANCE_INFO,
        SMSG_GUILD_MEMBER_LIST,
        SMSG_GUILD_POS_NAME_LIST,
        SMSG_GUILD_POS_INFO_LIST,
        SMSG_GUILD_POSITION_CHANGED,
        SMSG_GUILD_MEMBER_POS_CHANGE,
        SMSG_GUILD_EMBLEM,
        SMSG_GUILD_SKILL_INFO,
        SMSG_GUILD_NOTICE,
        SMSG_GUILD_INVITE,
        SMSG_GUILD_INVITE_ACK,
        SMSG_GUILD_LEAVE,
        SMSG_GUILD_EXPULSION,
        SMSG_GUILD_EXPULSION_LIST,
        SMSG_GUILD_MESSAGE,
        SMSG_GUILD_SKILL_UP,
        SMSG_GUILD_REQ_ALLIANCE,
        SMSG_GUILD_REQ_ALLIANCE_ACK,
        SMSG_GUILD_DEL_ALLIANCE,
        SMSG_GUILD_OPPOSITION_ACK,
        SMSG_GUILD_BROKEN,
*/
        0
    };
    handledMessages = _messages;

    guildHandler = this;
}

GuildHandler::~GuildHandler()
{
    delete guildTab;
    guildTab = 0;
}

void GuildHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_GUILD_CREATE_RESPONSE:
            {
                int flag = msg.readInt8();

                if (flag == 0)
                {
                    // Success
                }
                else if (flag == 1)
                {
                    // Already in a guild
                }
                else if (flag == 2)
                {
                    // Unable to make (likely name already in use)
                }
                else if (flag == 3)
                {
                    // Emperium check failed
                }
            }
            break;

        case SMSG_GUILD_POSITION_INFO:
            {
                int guildId =  msg.readInt32();
                int emblem =  msg.readInt32();
                int posMode =  msg.readInt32();
                msg.readInt32(); // Unused
                msg.readInt8(); // Unused
                std::string guildName = msg.readString(24);

                logger->log("Guild position info: %d %d %d %s", guildId,
                            emblem, posMode, guildName.c_str());
            }
            break;

        case SMSG_GUILD_MEMBER_LOGIN:
            msg.readInt32(); // Account ID
            msg.readInt32(); // Char ID
            msg.readInt32(); // Flag
            break;

        case SMSG_GUILD_MASTER_OR_MEMBER:
            msg.readInt32(); // Type (0x57 for member, 0xd7 for master)
            break;

        case SMSG_GUILD_BASIC_INFO:
            {
                int guildID = msg.readInt32(); // Guild ID
                msg.readInt32(); // Guild level
                msg.readInt32(); // 'Connect member' (number online?)
                msg.readInt32(); // 'Max member'
                msg.readInt32(); // Average level
                msg.readInt32(); // Exp
                msg.readInt32(); // Next exp
                msg.skip(16); // unused
                std::string name = msg.readString(24); // Name
                msg.readString(24); // Master's name
                msg.readString(20); // Castles (ie: "Six Castles" or "None Taken")

                Guild *g = Guild::getGuild(guildID);
                g->setName(name);
            }
            break;

        case SMSG_GUILD_ALIANCE_INFO:
            {
                int length = msg.readInt16();
                int count = (length - 4) / 32;

                for (int i = 0; i < count; i++)
                {
                    msg.readInt32(); // 'Opposition'
                    msg.readInt32(); // Other guild ID
                    msg.readString(24); // Other guild name
                }
            }
            break;

        case SMSG_GUILD_MEMBER_LIST:
            {
                int length = msg.readInt16();
                int count = (length - 4) / 104;

                taGuild->clearMembers();

                for (int i = 0; i < count; i++)
                {
                    int id = msg.readInt32(); // Account ID
                    msg.readInt32(); // Char ID
                    msg.readInt16(); // Hair
                    msg.readInt16(); // Hair color
                    msg.readInt16(); // Gender
                    msg.readInt16(); // Class
                    msg.readInt16(); // Level
                    msg.readInt32(); // Exp
                    int online = msg.readInt32(); // Online
                    msg.readInt32(); // Position
                    msg.skip(50); // unused
                    std::string name = msg.readString(24); // Name

                    GuildMember *m = taGuild->addMember(id, name);
                    m->setOnline(online);
                }
            }
            break;

        case SMSG_GUILD_POS_NAME_LIST:
            {
                int length = msg.readInt16();
                int count = (length - 4) / 28;

                for (int i = 0; i < count; i++)
                {
                    msg.readInt32(); // ID
                    msg.readString(24); // Position name
                }
            }
            break;

        case SMSG_GUILD_POS_INFO_LIST:
            {
                int length = msg.readInt16();
                int count = (length - 4) / 16;

                for (int i = 0; i < count; i++)
                {
                    msg.readInt32(); // ID
                    msg.readInt32(); // Mode
                    msg.readInt32(); // Same ID
                    msg.readInt32(); // Exp mode
                }
            }
            break;

        case SMSG_GUILD_POSITION_CHANGED:
            msg.readInt16(); // Always 44
            msg.readInt32(); // ID
            msg.readInt32(); // Mode
            msg.readInt32(); // Same ID
            msg.readInt32(); // Exp mode
            msg.readString(24); // Name
            break;

        case SMSG_GUILD_MEMBER_POS_CHANGE:
            msg.readInt16(); // Always 16
            msg.readInt32(); // Account ID
            msg.readInt32(); // Char ID
            msg.readInt32(); // Position
            break;

        case SMSG_GUILD_EMBLEM:
            {
                int length = msg.readInt16();

                msg.readInt32(); // Guild ID
                msg.readInt32(); // Emblem ID
                msg.skip(length - 12); // Emblem data (unknown format)
            }
            break;

        case SMSG_GUILD_SKILL_INFO:
            {
                int length = msg.readInt16();
                int count = (length - 6) / 37;

                msg.readInt16(); // 'Skill point'

                for (int i = 0; i < count; i++)
                {
                    msg.readInt16(); // ID
                    msg.readInt16(); // 'Info' (unknown atm)
                    msg.readInt16(); // unused
                    msg.readInt16(); // Level
                    msg.readInt16(); // SP
                    msg.readInt16(); // 'Range'
                    msg.skip(24); // unused
                    msg.readInt8(); // Can be increased
                }
            }
            break;

        case SMSG_GUILD_NOTICE:
            msg.readString(60); // Mes1
            msg.readString(120); // Mes2
            break;

        case SMSG_GUILD_INVITE:
            {
                int guildId = msg.readInt32();
                std::string guildName = msg.readString(24);

                socialWindow->showGuildInvite(guildName, guildId, "");
            }
            break;

        case SMSG_GUILD_INVITE_ACK:
            {
                int flag = msg.readInt8();

                switch (flag)
                {
                    case 0:
                        guildTab->chatLog(_("Could not invite user to guild."),
                                          BY_SERVER);
                        break;

                    case 1:
                        guildTab->chatLog(_("User rejected guild invite."),
                                        BY_SERVER);
                        break;

                    case 2:
                        guildTab->chatLog(_("User is now part of your guild."),
                                          BY_SERVER);
                        break;

                    case 3:
                        guildTab->chatLog(_("Your guild is full."),
                                          BY_SERVER);
                        break;

                    default:
                        guildTab->chatLog(_("Unknown guild invite response."),
                                          BY_SERVER);
                        break;
                }
            }
            break;

        case SMSG_GUILD_LEAVE:
            msg.readString(24); // Name
            msg.readString(40); // Message
            break;

        case SMSG_GUILD_EXPULSION:
            msg.readString(24); // Name (of expulsed?)
            msg.readString(40); // Message
            msg.skip(24); // unused ("dummy")
            break;

        case SMSG_GUILD_EXPULSION_LIST:
            {
                int length = msg.readInt16();
                int count = (length - 4) / 88;

                for (int i = 0; i < count; i++)
                {
                    msg.readString(24); // Name (of expulsed?)
                    msg.readString(24); // 'Acc' (name of expulser?)
                    msg.readString(24); // Message
                }
            }
            break;

        case SMSG_GUILD_MESSAGE:
            {
                int msgLength = msg.readInt16() - 4;
                if (msgLength <= 0)
                {
                    return;
                }
                guildTab->chatLog(msg.readString(msgLength));
            }
            break;

        case SMSG_GUILD_SKILL_UP:
            msg.readInt16(); // Skill ID
            msg.readInt16(); // Level
            msg.readInt16(); // SP
            msg.readInt16(); // 'Range'
            msg.readInt8(); // unused? (always 1)
            break;

        case SMSG_GUILD_REQ_ALLIANCE:
            msg.readInt32(); // Account ID
            msg.readString(24); // Name
            break;

        case SMSG_GUILD_REQ_ALLIANCE_ACK:
            msg.readInt32(); // Flag
            break;

        case SMSG_GUILD_DEL_ALLIANCE:
            msg.readInt32(); // Guild ID
            msg.readInt32(); // Flag
            break;

        case SMSG_GUILD_OPPOSITION_ACK:
            msg.readInt8(); // Flag
            break;

        case SMSG_GUILD_BROKEN:
            msg.readInt32(); // Flag
            break;
    }
}

void GuildHandler::create(const std::string &name)
{
    SERVER_NOTICE(_("Guild creation isn't supported yet."))
    return;

    MessageOut msg(CMSG_GUILD_CREATE);
    msg.writeInt32(0); // Unused
    msg.writeString(name, 24);
}

void GuildHandler::invite(int guildId, const std::string &name)
{
    // TODO?
}

void GuildHandler::invite(int guildId, Being *being)
{
    /* Disabled serverside anyway
    MessageOut msg(CMSG_GUILD_INVITE);
    msg.writeInt32(being->getId());
    msg.writeInt32(0); // Unused
    msg.writeInt32(0); // Unused */
}

void GuildHandler::inviteResponse(int guildId, bool response)
{
    MessageOut msg(CMSG_GUILD_INVITE_REPLY);
    msg.writeInt32(guildId);
    msg.writeInt8(response);
    msg.writeInt8(0); // Unused
    msg.writeInt16(0); // Unused
}

void GuildHandler::leave(int guildId)
{
    MessageOut msg(CMSG_GUILD_LEAVE);
    msg.writeInt32(guildId);
    msg.writeInt32(0); // Account ID
    msg.writeInt32(local_player->getId());
    msg.writeString("", 30); // Message
}

void GuildHandler::kick(GuildMember *member, std::string reason)
{
    MessageOut msg(CMSG_GUILD_EXPULSION);
    msg.writeInt32(member->getGuild()->getId());
    msg.writeInt32(member->getID()); // Account ID
    msg.writeInt32(0); // Char ID
    msg.writeString(reason, 40); // Message
}

void GuildHandler::chat(int guildId, const std::string &text)
{
    MessageOut msg(CMSG_GUILD_MESSAGE);
    msg.writeInt16(text.size() + 4);
    msg.writeString(text);
}

void GuildHandler::memberList(int guildId)
{
    // TODO four types of info requests:
    // 0 = basic info + alliance info
    // 1 = position name list + member list
    // 2 = position name list + position info list
    // 3 = skill info
    // 4 = expulsion list

    MessageOut msg(CMSG_GUILD_REQUEST_INFO);
    msg.writeInt32(1); // Request member list
}

void GuildHandler::changeMemberPostion(GuildMember *member, int level)
{
    // TODO
}

void GuildHandler::requestAlliance(int guildId, int otherGuildId)
{
    // TODO
}

void GuildHandler::requestAllianceResponse(int guildId, int otherGuildId,
                             bool response)
{
    // TODO
}

void GuildHandler::endAlliance(int guildId, int otherGuildId)
{
    // TODO
}

} // namespace TmwAthena
