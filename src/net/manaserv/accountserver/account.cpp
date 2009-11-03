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

#include "account.h"

#include "internal.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

#include "utils/sha256.h"

#include <string>

void ManaServ::AccountServer::Account::createCharacter(
        const std::string &name, char hairStyle, char hairColor, char gender,
        short strength, short agility, short vitality,
        short intelligence, short dexterity, short willpower)
{
    MessageOut msg(PAMSG_CHAR_CREATE);

    msg.writeString(name);
    msg.writeInt8(hairStyle);
    msg.writeInt8(hairColor);
    msg.writeInt8(gender);
    msg.writeInt16(strength);
    msg.writeInt16(agility);
    msg.writeInt16(vitality);
    msg.writeInt16(intelligence);
    msg.writeInt16(dexterity);
    msg.writeInt16(willpower);

    ManaServ::AccountServer::connection->send(msg);
}

void ManaServ::AccountServer::Account::deleteCharacter(char slot)
{
    MessageOut msg(PAMSG_CHAR_DELETE);

    msg.writeInt8(slot);

    ManaServ::AccountServer::connection->send(msg);
}

void ManaServ::AccountServer::Account::selectCharacter(char slot)
{
    MessageOut msg(PAMSG_CHAR_SELECT);

    msg.writeInt8(slot);

    ManaServ::AccountServer::connection->send(msg);
}

void ManaServ::AccountServer::Account::unregister(const std::string &username,
                                             const std::string &password)
{
    MessageOut msg(PAMSG_UNREGISTER);

    msg.writeString(username);
    msg.writeString(sha256(username + password));

    ManaServ::AccountServer::connection->send(msg);
}

void ManaServ::AccountServer::Account::changeEmail(const std::string &email)
{
    MessageOut msg(PAMSG_EMAIL_CHANGE);

    // Email is sent clearly so the server can validate the data.
    // Encryption is assumed server-side.
    msg.writeString(email);

    ManaServ::AccountServer::connection->send(msg);
}

void ManaServ::AccountServer::Account::changePassword(
                            const std::string &username,
                            const std::string &oldPassword,
                            const std::string &newPassword)
{
    MessageOut msg(PAMSG_PASSWORD_CHANGE);

    // Change password using SHA2 encryption
    msg.writeString(sha256(username + oldPassword));
    msg.writeString(sha256(username + newPassword));

    ManaServ::AccountServer::connection->send(msg);
}
