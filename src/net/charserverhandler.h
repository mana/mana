/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#ifndef NET_CHARSERVERHANDLER_H
#define NET_CHARSERVERHANDLER_H

#include "messagehandler.h"

#include "../lockedarray.h"

class CharCreateDialog;
class LocalPlayer;
class LoginData;

/**
 * Deals with incoming messages from the character server.
 */
class CharServerHandler : public MessageHandler
{
    public:
        CharServerHandler();

        void handleMessage(MessageIn *msg);

        void setCharInfo(LockedArray<LocalPlayer*> *charInfo)
        { mCharInfo = charInfo; }

        void setLoginData(LoginData *loginData)
        { mLoginData = loginData; }

        /**
         * Sets the character create dialog. The handler will clean up this
         * dialog when a new character is succesfully created, and will unlock
         * the dialog when a new character failed to be created.
         */
        void setCharCreateDialog(CharCreateDialog *window)
        { mCharCreateDialog = window; }

    protected:
        LoginData *mLoginData;
        LockedArray<LocalPlayer*> *mCharInfo;
        CharCreateDialog *mCharCreateDialog;

        LocalPlayer* readPlayerData(MessageIn &msg, int &slot);
};

#endif
