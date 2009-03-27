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

#ifndef _CHAR_SELECT_H
#define _CHAR_SELECT_H

#include <guichan/actionlistener.hpp>

#include "window.h"

#include "../being.h"
#include "../guichanfwd.h"
#include "../lockedarray.h"

#ifdef TMWSERV_SUPPORT
#include "../logindata.h"
#else
class Network;
#endif

class LocalPlayer;
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
#ifdef TMWSERV_SUPPORT
        CharSelectDialog(LockedArray<LocalPlayer*> *charInfo,
                         LoginData *loginData);
#else
        CharSelectDialog(Network *network,
                         LockedArray<LocalPlayer*> *charInfo,
                         Gender gender);
#endif

        void action(const gcn::ActionEvent &event);

        void updatePlayerInfo();

        void logic();

        bool selectByName(const std::string &name);

    private:
#ifdef EATHENA_SUPPORT
        Network *mNetwork;
#endif
        LockedArray<LocalPlayer*> *mCharInfo;

        gcn::Button *mSelectButton;
        gcn::Button *mCancelButton;
        gcn::Button *mPreviousButton;
        gcn::Button *mNextButton;

        gcn::Label *mNameLabel;
        gcn::Label *mLevelLabel;
        gcn::Label *mMoneyLabel;
        std::string mMoney;

        PlayerBox *mPlayerBox;

        bool mCharSelected;

#ifdef TMWSERV_SUPPORT
        gcn::Button *mNewCharButton;
        gcn::Button *mDelCharButton;
        gcn::Button *mUnRegisterButton;
        gcn::Button *mChangePasswordButton;
        gcn::Button *mChangeEmailButton;
        gcn::Label *mAccountNameLabel;

        LoginData *mLoginData;
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

/**
 * Character creation dialog.
 *
 * \ingroup Interface
 */
class CharCreateDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
#ifdef TMWSERV_SUPPORT
        CharCreateDialog(Window *parent, int slot);
#else
        CharCreateDialog(Window *parent, int slot, Network *network,
                         Gender gender);
#endif

        /**
         * Destructor.
         */
        ~CharCreateDialog();

        void action(const gcn::ActionEvent &event);

        /**
         * Unlocks the dialog, enabling the create character button again.
         */
        void unlock();

    private:
#ifdef TMWSERV_SUPPORT
        int getDistributedPoints();

        void updateSliders();
#endif

        /**
         * Returns the name of the character to create.
         */
        std::string getName();

        /**
         * Communicate character creation to the server.
         */
        void attemptCharCreate();

#ifdef EATHENA_SUPPORT
        Network *mNetwork;
#endif
        gcn::TextField *mNameField;
        gcn::Label *mNameLabel;
        gcn::Button *mNextHairColorButton;
        gcn::Button *mPrevHairColorButton;
        gcn::Label *mHairColorLabel;
        gcn::Button *mNextHairStyleButton;
        gcn::Button *mPrevHairStyleButton;
        gcn::Label *mHairStyleLabel;

#ifdef TMWSERV_SUPPORT
        gcn::RadioButton *mMale;
        gcn::RadioButton *mFemale;

        gcn::Slider *mAttributeSlider[6];
        gcn::Label *mAttributeLabel[6];
        gcn::Label *mAttributeValue[6];
        gcn::Label *mAttributesLeft;

        static const int mMaxPoints = 60;
        int mUsedPoints;
#endif

        gcn::Button *mCreateButton;
        gcn::Button *mCancelButton;

        Player *mPlayer;
        PlayerBox *mPlayerBox;

        int mSlot;
};

#endif
