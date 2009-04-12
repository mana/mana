/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef CHAR_CREATE_H
#define CHAR_CREATE_H

#include "being.h"
#include "guichanfwd.h"
#include "lockedarray.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

#include <string>
#include <vector>

class LocalPlayer;
class Player;
class PlayerBox;

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

        /**
         * Unlocks the dialog, enabling the create character button again.
         */
        void unlock();

        void setAttributes(std::vector<std::string> labels, int available,
                           int min, int max);

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

        Player *mPlayer;
        PlayerBox *mPlayerBox;

        int mSlot;
};

#endif // CHAR_CREATE_H
