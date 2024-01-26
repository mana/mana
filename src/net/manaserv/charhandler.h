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

#ifndef NET_MANASERV_CHARSERVERHANDLER_H
#define NET_MANASERV_CHARSERVERHANDLER_H

#include "gui/charselectdialog.h"

#include "net/charhandler.h"

#include "net/manaserv/messagehandler.h"

#include <map.h>

class LoginData;

namespace ManaServ {

/**
 * Deals with incoming messages related to character selection.
 */
class CharHandler : public MessageHandler, public Net::CharHandler
{
    public:
        CharHandler();

        ~CharHandler() override;

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

        void newCharacter(const std::string &name, int slot,
                          bool gender, int hairstyle, int hairColor,
                          const std::vector<int> &stats) override;

        void deleteCharacter(Net::Character *character) override;

        void switchCharacter() override;

        unsigned int baseSprite() const override;

        unsigned int hairSprite() const override;

        unsigned int maxSprite() const override;

        // No limitation on Manaserv
        int getCharCreateMaxHairColorId() const override
        { return 0; }

        // No limitation on Manaserv
        int getCharCreateMaxHairStyleId() const override
        { return 0; }

        void clear();

    private:
        /**
         * Character information needs to be cached since we receive it before
         * we have loaded the dynamic data, so we can't resolve load any
         * sprites yet.
         */
        struct CachedAttrbiute {
            double base;
            double mod;
        };

        using CachedAttributes = std::map<int, CachedAttrbiute>;

        struct CachedCharacterInfo {
            int slot;
            std::string name;
            Gender gender;
            int hairStyle;
            int hairColor;
            int level;
            int characterPoints;
            int correctionPoints;
            CachedAttributes attribute;
        };

        void handleCharacterInfo(MessageIn &msg);
        void handleCharacterCreateResponse(MessageIn &msg);
        void handleCharacterDeleteResponse(MessageIn &msg);
        void handleCharacterSelectResponse(MessageIn &msg);

        void updateCharacters();

        /** Cached character information */
        std::vector<CachedCharacterInfo> mCachedCharacterInfos;
};

} // namespace ManaServ

#endif
