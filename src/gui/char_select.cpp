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

#include "../net/messageout.h"
#include "../net/network.h"
#include "../net/protocol.h"

#include "../utils/tostring.h"

/**
 * Listener for confirming character deletion.
 */
class CharDeleteConfirm : public ConfirmDialog
{
    public:
        CharDeleteConfirm(CharSelectDialog *master);
        void action(const std::string &eventId, gcn::Widget *widget);
    private:
        CharSelectDialog *master;
};

CharDeleteConfirm::CharDeleteConfirm(CharSelectDialog *m):
    ConfirmDialog("Confirm", "Are you sure you want to delete this character?",
            m),
    master(m)
{
}

void CharDeleteConfirm::action(const std::string &eventId, gcn::Widget *widget)
{
    //ConfirmDialog::action(eventId);
    if (eventId == "yes") {
        master->attemptCharDelete();
    }
    ConfirmDialog::action(eventId, widget);
}

CharSelectDialog::CharSelectDialog(Network *network,
                                   LockedArray<LocalPlayer*> *charInfo):
    Window("Select Character"), mNetwork(network),
    mCharInfo(charInfo), mCharSelected(false)
{
    mSelectButton = new Button("Ok", "ok", this);
    mCancelButton = new Button("Cancel", "cancel", this);
    mNewCharButton = new Button("New", "new", this);
    mDelCharButton = new Button("Delete", "delete", this);
    mPreviousButton = new Button("Previous", "previous", this);
    mNextButton = new Button("Next", "next", this);

    mNameLabel = new gcn::Label("Name");
    mLevelLabel = new gcn::Label("Level");
    mMoneyLabel = new gcn::Label("Money");
    mPlayerBox = new PlayerBox(0);

    int w = 195;
    int h = 220;
    setContentSize(w, h);
    mPlayerBox->setDimension(gcn::Rectangle(5, 5, w - 10, 90));
    mNameLabel->setDimension(gcn::Rectangle(10, 100, 128, 16));
    mLevelLabel->setDimension(gcn::Rectangle(10, 116, 128, 16));
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
    add(mMoneyLabel);

    mSelectButton->requestFocus();
    setLocationRelativeTo(getParent());
    updatePlayerInfo();
}

void CharSelectDialog::action(const std::string &eventId, gcn::Widget *widget)
{
    if (eventId == "ok" && n_character > 0)
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
    else if (eventId == "cancel")
    {
        state = STATE_EXIT;
    }
    else if (eventId == "new")
    {
        if (n_character < MAX_SLOT + 1)
        {
            // Start new character dialog
            mCharInfo->lock();
            new CharCreateDialog(this, mCharInfo->getPos(), mNetwork);
            mCharInfo->unlock();
        }
    }
    else if (eventId == "delete")
    {
        // Delete character
        if (mCharInfo->getEntry())
        {
            new CharDeleteConfirm(this);
        }
    }
    else if (eventId == "previous")
    {
        mCharInfo->prev();
    }
    else if (eventId == "next")
    {
        mCharInfo->next();
    }
}

void CharSelectDialog::updatePlayerInfo()
{
    LocalPlayer *pi = mCharInfo->getEntry();

    if (pi) {
        mNameLabel->setCaption(pi->getName());
        mLevelLabel->setCaption("Lvl: " + toString(pi->mLevel));
        mMoneyLabel->setCaption("Money: " + toString(pi->mMoney));
        if (!mCharSelected)
        {
            mNewCharButton->setEnabled(false);
            mDelCharButton->setEnabled(true);
            mSelectButton->setEnabled(true);
        }
        mPlayerBox->mHairStyle = pi->getHairStyle() - 1;
        mPlayerBox->mHairColor = pi->getHairColor() - 1;
        mPlayerBox->mSex = pi->getSex();
        mPlayerBox->mShowPlayer = true;
    } else {
        mNameLabel->setCaption("Name");
        mLevelLabel->setCaption("Level");
        mMoneyLabel->setCaption("Money");
        mNewCharButton->setEnabled(true);
        mDelCharButton->setEnabled(false);
        mSelectButton->setEnabled(false);

        mPlayerBox->mHairStyle = 0;
        mPlayerBox->mHairColor = 0;
        mPlayerBox->mShowPlayer = false;
    }
}

void CharSelectDialog::attemptCharDelete()
{
    // Request character deletion
    MessageOut msg;
    msg.writeShort(PAMSG_CHAR_DELETE);
    // TODO: Send the selected slot
    msg.writeByte(0);
    network->send(Network::ACCOUNT, msg);
    mCharInfo->lock();
}

void CharSelectDialog::attemptCharSelect()
{
    // Request character selection
    MessageOut msg;
    msg.writeShort(PAMSG_CHAR_SELECT);
    msg.writeByte(mCharInfo->getPos());
    network->send(Network::ACCOUNT, msg);
    mCharInfo->lock();
}

void CharSelectDialog::logic()
{
    updatePlayerInfo();
}

CharCreateDialog::CharCreateDialog(Window *parent, int slot, Network *network):
    Window("Create Character", true, parent), mNetwork(network), mSlot(slot)
{
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
    mPlayerBox = new PlayerBox(0);
    mPlayerBox->mShowPlayer = true;

    mNameField->setEventId("create");

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
}

void CharCreateDialog::action(const std::string &eventId, gcn::Widget *widget)
{
    if (eventId == "create") {
        if (getName().length() >= 4) {
            // Attempt to create the character
            mCreateButton->setEnabled(false);
            attemptCharCreate();
            scheduleDelete();
        }
        else {
            new OkDialog("Error",
                    "Your name needs to be at least 4 characters.", this);
        }
    }
    else if (eventId == "cancel") {
        scheduleDelete();
    }
    else if (eventId == "nextcolor") {
        mPlayerBox->mHairColor++;
    }
    else if (eventId == "prevcolor") {
        mPlayerBox->mHairColor += NR_HAIR_COLORS - 1;
    }
    else if (eventId == "nextstyle") {
        mPlayerBox->mHairStyle++;
    }
    else if (eventId == "prevstyle") {
        mPlayerBox->mHairStyle += NR_HAIR_STYLES - 1;
    }

    mPlayerBox->mHairColor %= NR_HAIR_COLORS;
    mPlayerBox->mHairStyle %= NR_HAIR_STYLES;
}

std::string CharCreateDialog::getName()
{
    return mNameField->getText();
}

void CharCreateDialog::attemptCharCreate()
{
    // Send character infos
    MessageOut outMsg;
    outMsg.writeShort(PAMSG_CHAR_CREATE);
    outMsg.writeString(getName());
    outMsg.writeByte(mPlayerBox->mHairStyle + 1);
    outMsg.writeByte(mPlayerBox->mHairColor + 1);
    // TODO: send selected sex
    outMsg.writeByte(0); // Player sex
    outMsg.writeShort(10); // STR
    outMsg.writeShort(10); // AGI
    outMsg.writeShort(10); // VIT
    outMsg.writeShort(10); // INT
    outMsg.writeShort(10); // DEX
    outMsg.writeShort(10); // LUK
    network->send(Network::ACCOUNT, outMsg);
}
