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

#include "account.h"

#include "internal.h"

#include "../connection.h"
#include "../messageout.h"
#include "../protocol.h"

void Net::AccountServer::Account::createCharacter(
        const std::string &name, char hairStyle, char hairColor, char gender,
        short strength, short agility, short vitality,
        short intelligence, short dexterity, short willpower, short charisma)
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
    msg.writeInt16(charisma);

    Net::AccountServer::connection->send(msg);
}

void Net::AccountServer::Account::deleteCharacter(char slot)
{
    MessageOut msg(PAMSG_CHAR_DELETE);

    msg.writeInt8(slot);

    Net::AccountServer::connection->send(msg);
}

void Net::AccountServer::Account::selectCharacter(char slot)
{
    MessageOut msg(PAMSG_CHAR_SELECT);

    msg.writeInt8(slot);

    Net::AccountServer::connection->send(msg);
}

void Net::AccountServer::Account::unregister(const std::string &username,
                                             const std::string &password)
{
    MessageOut msg(PAMSG_UNREGISTER);

    msg.writeString(username);
    msg.writeString(password);

    Net::AccountServer::connection->send(msg);
}

void Net::AccountServer::Account::changeEmail(const std::string &email)
{
    MessageOut msg(PAMSG_EMAIL_CHANGE);

    msg.writeString(email);

    Net::AccountServer::connection->send(msg);
}

void Net::AccountServer::Account::getEmail()
{
    MessageOut msg(PAMSG_EMAIL_GET);

    Net::AccountServer::connection->send(msg);
}

void Net::AccountServer::Account::changePassword(
        const std::string &oldPassword, const std::string &newPassword)
{
    MessageOut msg(PAMSG_PASSWORD_CHANGE);

    msg.writeString(oldPassword);
    msg.writeString(newPassword);

    Net::AccountServer::connection->send(msg);
}
