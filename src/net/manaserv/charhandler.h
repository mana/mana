/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef NET_MANASERV_CHARSERVERHANDLER_H
#define NET_MANASERV_CHARSERVERHANDLER_H

#include "gui/charselectdialog.h"

#include "net/charhandler.h"

#include "net/manaserv/messagehandler.h"

class LoginData;

namespace ManaServ {

/**
 * Deals with incoming messages related to character selection.
 */
class CharHandler : public MessageHandler, public Net::CharHandler
{
    public:
        CharHandler();

        ~CharHandler();

        void handleMessage(Net::MessageIn &msg);

        void setCharSelectDialog(CharSelectDialog *window);

        /**
         * Sets the character create dialog. The handler will clean up this
         * dialog when a new character is succesfully created, and will unlock
         * the dialog when a new character failed to be created.
         */
        void setCharCreateDialog(CharCreateDialog *window);

        void requestCharacters();

        void chooseCharacter(Net::Character *character);

        void newCharacter(const std::string &name, int slot,
                          bool gender, int hairstyle, int hairColor,
                          const std::vector<int> &stats);

        void deleteCharacter(Net::Character *character);

        void switchCharacter();

        int baseSprite() const;

        int hairSprite() const;

        int maxSprite() const;

        void clear();

    private:
        /**
         * Character information needs to be cached since we receive it before
         * we have loaded the dynamic data, so we can't resolve load any
         * sprites yet.
         */
        struct CachedCharacterInfo {
            int slot;
            std::string name;
            Gender gender;
            int hairStyle;
            int hairColor;
            int level;
            int characterPoints;
            int correctionPoints;
            int money;
            int attribute[7];
        };

        void handleCharacterInfo(Net::MessageIn &msg);
        void handleCharacterCreateResponse(Net::MessageIn &msg);
        void handleCharacterDeleteResponse(Net::MessageIn &msg);
        void handleCharacterSelectResponse(Net::MessageIn &msg);

        void updateCharacters();

        /** Cached character information */
        std::vector<CachedCharacterInfo> mCachedCharacterInfos;
};

} // namespace ManaServ

#endif
