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

#ifndef CHAR_SELECT_H
#define CHAR_SELECT_H

#include "gui/widgets/window.h"

#include "net/charhandler.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

class CharacterDisplay;
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
        friend class Net::CharHandler;

        /**
         * Constructor.
         */
        CharSelectDialog(LoginData *loginData);

        ~CharSelectDialog();

        void action(const gcn::ActionEvent &event);

        void keyPressed(gcn::KeyEvent &keyEvent);

        enum SelectAction {
            Focus,
            Choose
        };

        /**
         * Attempt to select the character with the given name. Returns whether
         * a character with the given name was found.
         *
         * \param action determines what to do when a character with the given
         *               name was found (just focus or also try to choose this
         *               character).
         */
        bool selectByName(const std::string &name,
                          SelectAction action = Focus);

    private:
        void attemptCharacterDelete(int index);
        void attemptCharacterSelect(int index);

        void setCharacters(const Net::Characters &characters);

        void lock();
        void unlock();
        void setLocked(bool locked);

        bool mLocked;

        gcn::Label *mAccountNameLabel;

        gcn::Button *mSwitchLoginButton;
        gcn::Button *mChangePasswordButton;
        gcn::Button *mUnregisterButton;
        gcn::Button *mChangeEmailButton;

        enum { MAX_CHARACTER_COUNT = 3 };
        CharacterDisplay *mCharacterEntries[MAX_CHARACTER_COUNT];

        LoginData *mLoginData;

        Net::CharHandler *mCharHandler;
};

#endif
