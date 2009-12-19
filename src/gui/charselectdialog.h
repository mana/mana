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

#ifndef CHAR_SELECT_H
#define CHAR_SELECT_H

#include "gui/widgets/window.h"

#include "guichanfwd.h"
#include "lockedarray.h"
#include "main.h"
#include "player.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

class CharEntry;
class LocalPlayer;
class LoginData;
class PlayerBox;

namespace Net {
class CharHandler;
}

/**
 * Character selection dialog.
 *
 * \ingroup Interface
 */
class CharSelectDialog : public Window, public gcn::ActionListener,
                         public gcn::KeyListener
{
    public:
        friend class CharDeleteConfirm;

        /**
         * Constructor.
         */
        CharSelectDialog(LockedArray<LocalPlayer*> *charInfo,
                         LoginData *loginData);

        ~CharSelectDialog() { mCharInfo->clear(); };

        void action(const gcn::ActionEvent &event);

        void keyPressed(gcn::KeyEvent &keyEvent);

        bool selectByName(const std::string &name);

        void chooseSelected();

        void update(int slot = -1);

    private:
        /**
         * Communicate character deletion to the server.
         */
        void attemptCharDelete();

        /**
         * Communicate character selection to the server.
         */
        void attemptCharSelect();

        LockedArray<LocalPlayer*> *mCharInfo;

        gcn::Label *mAccountNameLabel;

        gcn::Button *mSwitchLoginButton;
        gcn::Button *mChangePasswordButton;

        CharEntry *mCharEntries[MAX_CHARACTER_COUNT];

        LoginData *mLoginData;

        Net::CharHandler *mCharHandler;
};

#endif
