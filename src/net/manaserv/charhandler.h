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

#ifndef NET_MANASERV_CHARSERVERHANDLER_H
#define NET_MANASERV_CHARSERVERHANDLER_H

#include "net/charhandler.h"

#include "gui/charselectdialog.h"

#include "net/messagehandler.h"

class LoginData;

namespace ManaServ {

/**
 * Deals with incoming messages related to character selection.
 */
class CharHandler : public MessageHandler, public Net::CharHandler
{
    public:
        CharHandler();

        void handleMessage(MessageIn &msg);

        void setCharInfo(LockedArray<LocalPlayer*> *charInfo)
        {
            mCharInfo = charInfo;
        }

        void setCharSelectDialog(CharSelectDialog *window);

        /**
         * Sets the character create dialog. The handler will clean up this
         * dialog when a new character is succesfully created, and will unlock
         * the dialog when a new character failed to be created.
         */
        void setCharCreateDialog(CharCreateDialog *window);

        void getCharacters();

        void chooseCharacter(int slot, LocalPlayer* character);

        void newCharacter(const std::string &name, int slot,
                        bool gender, int hairstyle, int hairColor,
                        std::vector<int> stats);

        void deleteCharacter(int slot, LocalPlayer* character);

    protected:
        void handleCharCreateResponse(MessageIn &msg);

        void handleCharSelectResponse(MessageIn &msg);

        LockedArray<LocalPlayer*> *mCharInfo;
        CharSelectDialog *mCharSelectDialog;
        CharCreateDialog *mCharCreateDialog;
};

} // namespace ManaServ

#endif
