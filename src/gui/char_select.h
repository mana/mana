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
#include "../lockedarray.h"

class LocalPlayer;
class Network;
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
        CharSelectDialog(Network *network,
                         LockedArray<LocalPlayer*> *charInfo,
                         Gender gender);

        void action(const gcn::ActionEvent &event);

        void updatePlayerInfo();

        void logic();

        bool selectByName(const std::string &name);

    private:
        Network *mNetwork;
        LockedArray<LocalPlayer*> *mCharInfo;

        gcn::Button *mSelectButton;
        gcn::Button *mCancelButton;
        gcn::Button *mNewDelCharButton;
        gcn::Button *mPreviousButton;
        gcn::Button *mNextButton;

        gcn::Label *mNameLabel;
        gcn::Label *mLevelLabel;
        gcn::Label *mJobLevelLabel;
        gcn::Label *mMoneyLabel; std::string mMoney;

        PlayerBox *mPlayerBox;

        Gender mGender;
        bool mCharSelected;

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
 * \ingroup GUI
 */
class CharCreateDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        CharCreateDialog(Window *parent, int slot, Network *network,
                         Gender gender);

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
        /**
         * Returns the name of the character to create.
         */
        std::string getName();

        /**
         * Communicate character creation to the server.
         */
        void attemptCharCreate();

        Network *mNetwork;
        gcn::TextField *mNameField;
        gcn::Label *mNameLabel;
        gcn::Button *mNextHairColorButton;
        gcn::Button *mPrevHairColorButton;
        gcn::Label *mHairColorLabel;
        gcn::Button *mNextHairStyleButton;
        gcn::Button *mPrevHairStyleButton;
        gcn::Label *mHairStyleLabel;
        gcn::Button *mCreateButton;
        gcn::Button *mCancelButton;

        Player *mPlayer;
        PlayerBox *mPlayerBox;

        int mSlot;
};

#endif
