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
 *  $Id$
 */

#ifndef _CHAR_SELECT_H
#define _CHAR_SELECT_H

#include "window.h"

#include "../guichanfwd.h"
#include "../lockedarray.h"

#include <guichan/actionlistener.hpp>

#include "../logindata.h"

class Player;
class LocalPlayer;
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

        /**
         * Returns name of selected player
         */
        std::string getName();

    private:
        LockedArray<LocalPlayer*> *mCharInfo;

        gcn::Button *mSelectButton;
        gcn::Button *mCancelButton;
        gcn::Button *mNewCharButton;
        gcn::Button *mDelCharButton;
        gcn::Button *mPreviousButton;
        gcn::Button *mNextButton;
        gcn::Button *mUnRegisterButton;

        gcn::Label *mNameLabel;
        gcn::Label *mLevelLabel;
        gcn::Label *mMoneyLabel;

        PlayerBox *mPlayerBox;

        bool mCharSelected;

        LoginData *mLoginData;
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
        CharCreateDialog(Window *parent, int slot);

        /**
         * Destructor.
         */
        ~CharCreateDialog();

        void action(const gcn::ActionEvent &event);

        std::string getName();

    private:
        int getDistributedPoints();
        void UpdateSliders();

        gcn::TextField *mNameField;
        gcn::Label *mNameLabel;
        gcn::Button *mNextHairColorButton;
        gcn::Button *mPrevHairColorButton;
        gcn::Label *mHairColorLabel;
        gcn::Button *mNextHairStyleButton;
        gcn::Button *mPrevHairStyleButton;
        gcn::Label *mHairStyleLabel;

        gcn::Slider *mAttributeSlider[7];
        gcn::Label *mAttributeLabel[7];
        gcn::Label *mAttributeValue[7];
        gcn::Label *mAttributesLeft;

        gcn::Button *mCreateButton;
        gcn::Button *mCancelButton;

        Player *mPlayer;
        PlayerBox *mPlayerBox;

        int mSlot;

        static const int mMaxPoints = 70;
        int mUsedPoints;


        /**
         * Communicate character creation to the server.
         */
        void attemptCharCreate();
};

#endif
