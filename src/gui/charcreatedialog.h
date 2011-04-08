/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef CHAR_CREATE_DIALOG_H
#define CHAR_CREATE_DIALOG_H

#include "being.h"
#include "guichanfwd.h"

#include "gui/charselectdialog.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

#include <string>
#include <vector>

class LocalPlayer;
class PlayerBox;

/**
 * Character creation dialog.
 *
 * \ingroup Interface
 */
class CharCreateDialog : public Window, public gcn::ActionListener
{
    public:
        CharCreateDialog(CharSelectDialog *parent, int slot);

        ~CharCreateDialog();

        void action(const gcn::ActionEvent &event);

        /**
         * Unlocks the dialog, enabling the create character button again.
         */
        void unlock();

        void setAttributes(const std::vector<std::string> &labels,
                           unsigned int available,
                           unsigned int min, unsigned int max);

        void setFixedGender(bool fixed, Gender gender = GENDER_FEMALE);

    private:
        int getDistributedPoints() const;

        void updateSliders();

        /**
         * Returns the name of the character to create.
         */
        std::string getName() const;

        /**
         * Communicate character creation to the server.
         */
        void attemptCharCreate();

        void updateHair();

        CharSelectDialog *mCharSelectDialog;

        gcn::TextField *mNameField;
        gcn::Label *mNameLabel;
        gcn::Button *mNextHairColorButton;
        gcn::Button *mPrevHairColorButton;
        gcn::Label *mHairColorLabel;
        gcn::Button *mNextHairStyleButton;
        gcn::Button *mPrevHairStyleButton;
        gcn::Label *mHairStyleLabel;

        gcn::RadioButton *mMale;
        gcn::RadioButton *mFemale;

        std::vector<gcn::Slider*> mAttributeSlider;
        std::vector<gcn::Label*> mAttributeLabel;
        std::vector<gcn::Label*> mAttributeValue;
        gcn::Label *mAttributesLeft;

        int mMaxPoints;
        int mUsedPoints;

        gcn::Button *mCreateButton;
        gcn::Button *mCancelButton;

        Being *mPlayer;
        PlayerBox *mPlayerBox;

        int mHairStyle;
        int mHairColor;

        int mSlot;
};

#endif // CHAR_CREATE_DIALOG_H
