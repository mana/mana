/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#ifndef NET_EA_CHARSERVERHANDLER_H
#define NET_EA_CHARSERVERHANDLER_H

#include "net/charhandler.h"
#include "net/serverinfo.h"

#include "net/ea/messagehandler.h"
#include "net/ea/token.h"

class LoginData;

namespace EAthena {

/**
 * Deals with incoming messages from the character server.
 */
class CharServerHandler : public MessageHandler, public Net::CharHandler
{
    public:
        CharServerHandler();

        virtual void handleMessage(Net::MessageIn &msg);

        void setCharSelectDialog(CharSelectDialog *window);

        /**
         * Sets the character create dialog. The handler will clean up this
         * dialog when a new character is succesfully created, and will unlock
         * the dialog when a new character failed to be created.
         */
        void setCharCreateDialog(CharCreateDialog *window);

        void requestCharacters();

        void chooseCharacter(Net::Character *character);

        void newCharacter(const std::string &name, int slot, bool gender,
                          int hairstyle, int hairColor,
                          const std::vector<int> &stats);

        void deleteCharacter(Net::Character *character);

        void switchCharacter();

        int baseSprite() const;

        int hairSprite() const;

        int maxSprite() const;

        void connect();

    private:
        LocalPlayer *readPlayerData(Net::MessageIn &msg, int *slot);
};

} // namespace EAthena

#endif // NET_EA_CHARSERVERHANDLER_H
