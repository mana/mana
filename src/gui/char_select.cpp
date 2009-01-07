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
 */

#include <string>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "char_select.h"
#include "confirm_dialog.h"
#include "ok_dialog.h"
#include "playerbox.h"
#include "textfield.h"

#include "widgets/layout.h"

#include "../game.h"
#include "../localplayer.h"
#include "../main.h"

#include "../net/charserverhandler.h"
#include "../net/messageout.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
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
    ConfirmDialog(_("Confirm Character Delete"),
                  _("Are you sure you want to delete this character?"), m),
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
                                   Gender gender):
    Window(_("Select Character")), mNetwork(network),
    mCharInfo(charInfo), mGender(gender), mCharSelected(false)
{
    // Control that shows the Player
    mPlayerBox = new PlayerBox;

    mNameLabel = new gcn::Label(strprintf(_("Name: %s"), ""));
    mLevelLabel = new gcn::Label(strprintf(_("Level: %d"), 0));
    mJobLevelLabel = new gcn::Label(strprintf(_("Job Level: %d"), 0));
    mMoneyLabel = new gcn::Label(strprintf(_("Money: %d"), 0));

    mPreviousButton = new Button(_("Previous"), "previous", this);
    mNextButton = new Button(_("Next"), "next", this);
    mNewCharButton = new Button(_("New"), "new", this);
    mDelCharButton = new Button(_("Delete"), "delete", this);
    mSelectButton = new Button(_("Ok"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mPlayerBox, 6, 3);
    place(0, 3, mNameLabel, 2);
    place(0, 4, mLevelLabel, 2);
    place(0, 5, mJobLevelLabel, 2);
    place(0, 6, mMoneyLabel, 2);
    place(0, 7, mPreviousButton);
    place(1, 7, mNextButton);
    place(0, 8, mNewCharButton);
    place(1, 8, mDelCharButton);
    place(3, 8, mSelectButton);
    place(4, 8, mCancelButton);

    reflowLayout(195, 220);

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
        mNameLabel->setCaption(strprintf(_("Name: %s"), pi->getName().c_str()));
        mLevelLabel->setCaption(strprintf(_("Level: %d"), pi->mLevel));
        mJobLevelLabel->setCaption(strprintf(_("Job Level: %d"), pi->mJobLevel));
        mMoneyLabel->setCaption(strprintf(_("Gold: %d"), pi->mGp));
        if (!mCharSelected)
        {
            mNewCharButton->setEnabled(false);
            mDelCharButton->setEnabled(true);
            mSelectButton->setEnabled(true);
        }
    }
    else {
        mNameLabel->setCaption(strprintf(_("Name: %s"), ""));
        mLevelLabel->setCaption(strprintf(_("Level: %d"), 0));
        mJobLevelLabel->setCaption(strprintf(_("Job Level: %d"), 0));
        mMoneyLabel->setCaption(strprintf(_("Money: %d"), 0));
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
                                   Gender gender):
    Window(_("Create Character"), true, parent), mNetwork(network), mSlot(slot)
{
    mPlayer = new Player(0, 0, NULL);
    mPlayer->setGender(gender);

    int numberOfHairColors = ColorDB::size();

    mPlayer->setHairStyle(rand() % mPlayer->getNumOfHairstyles(), rand() % numberOfHairColors);

    mNameField = new TextField("");
    mNameLabel = new gcn::Label(_("Name:"));
    mNextHairColorButton = new Button(">", "nextcolor", this);
    mPrevHairColorButton = new Button("<", "prevcolor", this);
    mHairColorLabel = new gcn::Label(_("Hair Color:"));
    mNextHairStyleButton = new Button(">", "nextstyle", this);
    mPrevHairStyleButton = new Button("<", "prevstyle", this);
    mHairStyleLabel = new gcn::Label(_("Hair Style:"));
    mCreateButton = new Button(_("Create"), "create", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
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
