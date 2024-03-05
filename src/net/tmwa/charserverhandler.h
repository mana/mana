/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#ifndef NET_TA_CHARSERVERHANDLER_H
#define NET_TA_CHARSERVERHANDLER_H

#include "net/charhandler.h"

#include "net/tmwa/messagehandler.h"
#include "net/tmwa/token.h"

class LoginData;

namespace TmwAthena {

/**
 * Deals with incoming messages from the character server.
 */
class CharServerHandler final : public MessageHandler, public Net::CharHandler
{
    public:
        CharServerHandler();

        void handleMessage(MessageIn &msg) override;

        void setCharSelectDialog(CharSelectDialog *window) override;

        /**
         * Sets the character create dialog. The handler will clean up this
         * dialog when a new character is succesfully created, and will unlock
         * the dialog when a new character failed to be created.
         */
        void setCharCreateDialog(CharCreateDialog *window) override;

        void requestCharacters() override;

        void chooseCharacter(Net::Character *character) override;

        void newCharacter(const std::string &name, int slot, bool gender,
                          int hairstyle, int hairColor,
                          const std::vector<int> &stats) override;

        void deleteCharacter(Net::Character *character) override;

        void switchCharacter() override;

        unsigned int baseSprite() const override;

        unsigned int hairSprite() const override;

        unsigned int maxSprite() const override;

        int getCharCreateMinHairColorId() const override;
        int getCharCreateMaxHairColorId() const override;
        int getCharCreateMaxHairStyleId() const override;

        void connect();

    private:
        void readPlayerData(MessageIn &msg, Net::Character *character);
};

} // namespace TmwAthena

#endif // NET_TA_CHARSERVERHANDLER_H
