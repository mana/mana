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

#include "being.h"
#include "guichanfwd.h"
#include "lockedarray.h"

#include <guichan/actionlistener.hpp>

class LocalPlayer;
class LoginData;
class Player;
class PlayerBox;

/**
 * Character selection dialog.
 *
 * \ingroup Interface
 */
class CharSelectDialog : public Window, public gcn::ActionListener
{
    public:
        friend class CharDeleteConfirm;
        /**
         * Constructor.
         */
        CharSelectDialog(LockedArray<LocalPlayer*> *charInfo,
                         LoginData *loginData);

        void action(const gcn::ActionEvent &event);

        void updatePlayerInfo();

        void logic();

        bool selectByName(const std::string &name);

    private:
        LockedArray<LocalPlayer*> *mCharInfo;

        gcn::Button *mSelectButton;
        gcn::Button *mCancelButton;
        gcn::Button *mPreviousButton;
        gcn::Button *mNextButton;
        gcn::Button *mChangePasswordButton;

        gcn::Label *mNameLabel;
        gcn::Label *mLevelLabel;
        gcn::Label *mMoneyLabel;
        gcn::Label *mAccountNameLabel;
        std::string mMoney;

        LoginData *mLoginData;

        PlayerBox *mPlayerBox;

        bool mCharSelected;

#ifdef TMWSERV_SUPPORT
        gcn::Button *mNewCharButton;
        gcn::Button *mDelCharButton;
        gcn::Button *mUnRegisterButton;
        gcn::Button *mChangeEmailButton;

#else
        gcn::Button *mNewDelCharButton;
        gcn::Label *mJobLevelLabel;
        Gender mGender;
#endif

        /**
         * Communicate character deletion to the server.
         */
        void attemptCharDelete();

        /**
         * Communicate character selection to the server.
         */
        void attemptCharSelect();
};

#endif
