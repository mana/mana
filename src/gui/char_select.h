/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id: char_select.h 3752 2007-11-20 10:50:00Z b_lindeijer $
 */

#ifndef _CHAR_SELECT_H
#define _CHAR_SELECT_H

#include "window.h"

#include "../guichanfwd.h"
#include "../lockedarray.h"

#include <guichan/actionlistener.hpp>

class Player;
class LocalPlayer;
class Network;
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
                         unsigned char gender);

        void action(const gcn::ActionEvent &event);

        void updatePlayerInfo();

        void logic();

        bool selectByName(const std::string &name);

    private:
        Network *mNetwork;
        LockedArray<LocalPlayer*> *mCharInfo;

        gcn::Button *mSelectButton;
        gcn::Button *mCancelButton;
        gcn::Button *mNewCharButton;
        gcn::Button *mDelCharButton;
        gcn::Button *mPreviousButton;
        gcn::Button *mNextButton;

        gcn::Label *mNameLabel;
        gcn::Label *mLevelLabel;
        gcn::Label *mJobLevelLabel;
        gcn::Label *mMoneyLabel;

        PlayerBox *mPlayerBox;

        unsigned char mGender;
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
                         unsigned char gender);

        /**
         * Destructor.
         */
        ~CharCreateDialog();

        void
        action(const gcn::ActionEvent &event);

        /**
         * Unlocks the dialog, enabling the create character button again.
         */
        void
        unlock();

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
