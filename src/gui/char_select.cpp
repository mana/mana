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
 *  $Id: char_select.cpp 3760 2007-11-21 19:43:11Z b_lindeijer $
 */

#include "char_select.h"

#include <string>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "confirm_dialog.h"
#include "ok_dialog.h"
#include "playerbox.h"
#include "textfield.h"

#include "../game.h"
#include "../localplayer.h"
#include "../main.h"

#include "../net/charserverhandler.h"
#include "../net/messageout.h"

#include "../utils/tostring.h"
#include "../utils/trim.h"

// Defined in main.cpp, used here for setting the char create dialog
extern CharServerHandler charServerHandler;

/**
 * Listener for confirming character deletion.
 */
class CharDeleteConfirm : public ConfirmDialog
{
    public:
        CharDeleteConfirm(CharSelectDialog *master);
        void action(const gcn::ActionEvent &event);
    private:
        CharSelectDialog *master;
};

CharDeleteConfirm::CharDeleteConfirm(CharSelectDialog *m):
    ConfirmDialog("Confirm", "Are you sure you want to delete this character?",
            m),
    master(m)
{
}

void CharDeleteConfirm::action(const gcn::ActionEvent &event)
{
    //ConfirmDialog::action(event);
    if (event.getId() == "yes") {
        master->attemptCharDelete();
    }
    ConfirmDialog::action(event);
}

CharSelectDialog::CharSelectDialog(Network *network,
                                   LockedArray<LocalPlayer*> *charInfo,
                                   unsigned char gender):
    Window("Select Character"), mNetwork(network),
    mCharInfo(charInfo), mGender(gender), mCharSelected(false)
{
    mSelectButton = new Button("Ok", "ok", this);
    mCancelButton = new Button("Cancel", "cancel", this);
    mNewCharButton = new Button("New", "new", this);
    mDelCharButton = new Button("Delete", "delete", this);
    mPreviousButton = new Button("Previous", "previous", this);
    mNextButton = new Button("Next", "next", this);

    mNameLabel = new gcn::Label("Name");
    mLevelLabel = new gcn::Label("Level");
    mJobLevelLabel = new gcn::Label("Job Level");
    mMoneyLabel = new gcn::Label("Money");
    mPlayerBox = new PlayerBox();

    int w = 195;
    int h = 220;
    setContentSize(w, h);
    mPlayerBox->setDimension(gcn::Rectangle(5, 5, w - 10, 90));
    mNameLabel->setDimension(gcn::Rectangle(10, 100, 128, 16));
    mLevelLabel->setDimension(gcn::Rectangle(10, 116, 128, 16));
    mJobLevelLabel->setDimension(gcn::Rectangle(10, 132, 128, 16));
    mMoneyLabel->setDimension(gcn::Rectangle(10, 148, 128, 16));
    mPreviousButton->setPosition(5, 170);
    mNextButton->setPosition(mPreviousButton->getWidth() + 10, 170);
    mNewCharButton->setPosition(5, h - 5 - mNewCharButton->getHeight());
    mDelCharButton->setPosition(
            5 + mNewCharButton->getWidth() + 5,
            mNewCharButton->getY());
    mCancelButton->setPosition(
            w - 5 - mCancelButton->getWidth(),
            mNewCharButton->getY());
    mSelectButton->setPosition(
            mCancelButton->getX() - 5 - mSelectButton->getWidth(),
            mNewCharButton->getY());

    add(mPlayerBox);
    add(mSelectButton);
    add(mCancelButton);
    add(mNewCharButton);
    add(mDelCharButton);
    add(mPreviousButton);
    add(mNextButton);
    add(mNameLabel);
    add(mLevelLabel);
    add(mJobLevelLabel);
    add(mMoneyLabel);

    setLocationRelativeTo(getParent());
    setVisible(true);
    mSelectButton->requestFocus();
    updatePlayerInfo();
}

void CharSelectDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok" && n_character > 0)
    {
        // Start game
        mNewCharButton->setEnabled(false);
        mDelCharButton->setEnabled(false);
        mSelectButton->setEnabled(false);
        mPreviousButton->setEnabled(false);
        mNextButton->setEnabled(false);
        mCharSelected = true;
        attemptCharSelect();
    }
    else if (event.getId() == "cancel")
    {
        state = EXIT_STATE;
    }
    else if (event.getId() == "new" && n_character <= MAX_SLOT)
    {
        // Start new character dialog
        CharCreateDialog *charCreateDialog =
            new CharCreateDialog(this, mCharInfo->getPos(), mNetwork, mGender);
        charServerHandler.setCharCreateDialog(charCreateDialog);
    }
    else if (event.getId() == "delete")
    {
        // Delete character
        if (mCharInfo->getEntry())
        {
            new CharDeleteConfirm(this);
        }
    }
    else if (event.getId() == "previous")
    {
        mCharInfo->prev();
    }
    else if (event.getId() == "next")
    {
        mCharInfo->next();
    }
}

void CharSelectDialog::updatePlayerInfo()
{
    LocalPlayer *pi = mCharInfo->getEntry();

    if (pi)
    {
        mNameLabel->setCaption(pi->getName());
        mLevelLabel->setCaption("Lvl: " + toString(pi->mLevel));
        mJobLevelLabel->setCaption("Job Lvl: " + toString(pi->mJobLevel));
        mMoneyLabel->setCaption("Gold: " + toString(pi->mGp));
        if (!mCharSelected)
        {
            mNewCharButton->setEnabled(false);
            mDelCharButton->setEnabled(true);
            mSelectButton->setEnabled(true);
        }
    }
    else {
        mNameLabel->setCaption("Name");
        mLevelLabel->setCaption("Level");
        mJobLevelLabel->setCaption("Job Level");
        mMoneyLabel->setCaption("Money");
        mNewCharButton->setEnabled(true);
        mDelCharButton->setEnabled(false);
        mSelectButton->setEnabled(false);
    }

    mPlayerBox->setPlayer(pi);
}

void CharSelectDialog::attemptCharDelete()
{
    // Request character deletion
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x0068);
    outMsg.writeInt32(mCharInfo->getEntry()->mCharId);
    outMsg.writeString("a@a.com", 40);
    mCharInfo->lock();
}

void CharSelectDialog::attemptCharSelect()
{
    // Request character selection
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x0066);
    outMsg.writeInt8(mCharInfo->getPos());
    mCharInfo->lock();
}

void CharSelectDialog::logic()
{
    updatePlayerInfo();
}

bool CharSelectDialog::selectByName(const std::string &name)
{
    if (mCharInfo->isLocked())
        return false;

    unsigned int oldPos = mCharInfo->getPos();

    mCharInfo->select(0);
    do {
        LocalPlayer *player = mCharInfo->getEntry();

        if (player && player->getName() == name)
            return true;

        mCharInfo->next();
    } while (mCharInfo->getPos());

    mCharInfo->select(oldPos);

    return false;
}

CharCreateDialog::CharCreateDialog(Window *parent, int slot, Network *network,
                                   unsigned char gender):
    Window("Create Character", true, parent), mNetwork(network), mSlot(slot)
{
    mPlayer = new Player(0, 0, NULL);
    mPlayer->setGender(gender);

    int numberOfHairColors = ColorDB::size();

    printf("%d\n", numberOfHairColors);

    mPlayer->setHairStyle(rand() % mPlayer->getNumOfHairstyles(), rand() % numberOfHairColors);

    mNameField = new TextField("");
    mNameLabel = new gcn::Label("Name:");
    mNextHairColorButton = new Button(">", "nextcolor", this);
    mPrevHairColorButton = new Button("<", "prevcolor", this);
    mHairColorLabel = new gcn::Label("Hair Color:");
    mNextHairStyleButton = new Button(">", "nextstyle", this);
    mPrevHairStyleButton = new Button("<", "prevstyle", this);
    mHairStyleLabel = new gcn::Label("Hair Style:");
    mCreateButton = new Button("Create", "create", this);
    mCancelButton = new Button("Cancel", "cancel", this);
    mPlayerBox = new PlayerBox(mPlayer);

    mNameField->setActionEventId("create");

    int w = 200;
    int h = 150;
    setContentSize(w, h);
    mPlayerBox->setDimension(gcn::Rectangle(80, 30, 110, 85));
    mNameLabel->setPosition(5, 5);
    mNameField->setDimension(
            gcn::Rectangle(45, 5, w - 45 - 7, mNameField->getHeight()));
    mPrevHairColorButton->setPosition(90, 35);
    mNextHairColorButton->setPosition(165, 35);
    mHairColorLabel->setPosition(5, 40);
    mPrevHairStyleButton->setPosition(90, 64);
    mNextHairStyleButton->setPosition(165, 64);
    mHairStyleLabel->setPosition(5, 70);
    mCancelButton->setPosition(
            w - 5 - mCancelButton->getWidth(),
            h - 5 - mCancelButton->getHeight());
    mCreateButton->setPosition(
            mCancelButton->getX() - 5 - mCreateButton->getWidth(),
            h - 5 - mCancelButton->getHeight());

    mNameField->addActionListener(this);

    add(mPlayerBox);
    add(mNameField);
    add(mNameLabel);
    add(mNextHairColorButton);
    add(mPrevHairColorButton);
    add(mHairColorLabel);
    add(mNextHairStyleButton);
    add(mPrevHairStyleButton);
    add(mHairStyleLabel);
    add(mCreateButton);
    add(mCancelButton);

    setLocationRelativeTo(getParent());
    setVisible(true);
    mNameField->requestFocus();
}

CharCreateDialog::~CharCreateDialog()
{
    delete mPlayer;

    // Make sure the char server handler knows that we're gone
    charServerHandler.setCharCreateDialog(0);
}

void
CharCreateDialog::action(const gcn::ActionEvent &event)
{
    int numberOfColors = ColorDB::size();
    if (event.getId() == "create") {
        if (getName().length() >= 4) {
            // Attempt to create the character
            mCreateButton->setEnabled(false);
            attemptCharCreate();
        }
        else {
            new OkDialog("Error",
                    "Your name needs to be at least 4 characters.", this);
        }
    }
    else if (event.getId() == "cancel") {
        scheduleDelete();
    }
    else if (event.getId() == "nextcolor") {
        mPlayer->setHairStyle(mPlayer->getHairStyle(), (mPlayer->getHairColor() + 1) % numberOfColors);
    }
    else if (event.getId() == "prevcolor") {
        mPlayer->setHairStyle(mPlayer->getHairStyle(), (mPlayer->getHairColor() + numberOfColors - 1) % numberOfColors);
    }
    else if (event.getId() == "nextstyle") {
        mPlayer->setHairStyle(mPlayer->getHairStyle() + 1, mPlayer->getHairColor());
    }
    else if (event.getId() == "prevstyle") {
        mPlayer->setHairStyle(mPlayer->getHairStyle() + mPlayer->getNumOfHairstyles() - 1, mPlayer->getHairColor());
    }
}

std::string
CharCreateDialog::getName()
{
    std::string name = mNameField->getText();
    trim(name);
    return name;
}

void
CharCreateDialog::unlock()
{
    mCreateButton->setEnabled(true);
}

void
CharCreateDialog::attemptCharCreate()
{
    // Send character infos
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x0067);
    outMsg.writeString(getName(), 24);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(mSlot);
    outMsg.writeInt16(mPlayer->getHairColor());
    outMsg.writeInt16(mPlayer->getHairStyle());
}
