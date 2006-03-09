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

#include <sstream>
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

/**
 * Listener for confirming character deletion.
 */
class CharDeleteConfirm : public ConfirmDialog
{
    public:
        CharDeleteConfirm(CharSelectDialog *master);
        void action(const std::string &eventId);
    private:
        CharSelectDialog *master;
};

CharDeleteConfirm::CharDeleteConfirm(CharSelectDialog *m):
    ConfirmDialog("Confirm", "Are you sure you want to delete this character?",
            m),
    master(m)
{
}

void CharDeleteConfirm::action(const std::string &eventId)
{
    //ConfirmDialog::action(eventId);
    if (eventId == "yes") {
        master->attemptCharDelete();
    }
    ConfirmDialog::action(eventId);
}

CharSelectDialog::CharSelectDialog(Network *network, LockedArray<LocalPlayer*> *charInfo):
    Window("Select Character"), mNetwork(network), mCharInfo(charInfo), mCharSelected(false)
{
    selectButton = new Button("Ok", "ok", this);
    cancelButton = new Button("Cancel", "cancel", this);
    newCharButton = new Button("New", "new", this);
    delCharButton = new Button("Delete", "delete", this);
    previousButton = new Button("Previous", "previous", this);
    nextButton = new Button("Next", "next", this);

    nameLabel = new gcn::Label("Name");
    levelLabel = new gcn::Label("Level");
    jobLevelLabel = new gcn::Label("Job Level");
    moneyLabel = new gcn::Label("Money");
    playerBox = new PlayerBox();

    int w = 195;
    int h = 220;
    setContentSize(w, h);
    playerBox->setDimension(gcn::Rectangle(5, 5, w - 10, 90));
    nameLabel->setDimension(gcn::Rectangle(10, 100, 128, 16));
    levelLabel->setDimension(gcn::Rectangle(10, 116, 128, 16));
    jobLevelLabel->setDimension(gcn::Rectangle(10, 132, 128, 16));
    moneyLabel->setDimension(gcn::Rectangle(10, 148, 128, 16));
    previousButton->setPosition(5, 170);
    nextButton->setPosition(previousButton->getWidth() + 10, 170);
    newCharButton->setPosition(5, h - 5 - newCharButton->getHeight());
    delCharButton->setPosition(
            5 + newCharButton->getWidth() + 5,
            newCharButton->getY());
    cancelButton->setPosition(
            w - 5 - cancelButton->getWidth(),
            newCharButton->getY());
    selectButton->setPosition(
            cancelButton->getX() - 5 - selectButton->getWidth(),
            newCharButton->getY());

    add(playerBox);
    add(selectButton);
    add(cancelButton);
    add(newCharButton);
    add(delCharButton);
    add(previousButton);
    add(nextButton);
    add(nameLabel);
    add(levelLabel);
    add(jobLevelLabel);
    add(moneyLabel);

    selectButton->requestFocus();
    setLocationRelativeTo(getParent());
    updatePlayerInfo();
}

void CharSelectDialog::action(const std::string& eventId)
{
    if (eventId == "ok" && n_character > 0)
    {
        // Start game
        newCharButton->setEnabled(false);
        delCharButton->setEnabled(false);
        selectButton->setEnabled(false);
        previousButton->setEnabled(false);
        nextButton->setEnabled(false);
        mCharSelected = true;
        attemptCharSelect();
    }
    else if (eventId == "cancel")
    {
        state = EXIT_STATE;
    }
    else if (eventId == "new")
    {
        if (n_character < MAX_SLOT + 1)
        {
            // Start new character dialog
            new CharCreateDialog(this, mCharInfo->getPos(), mNetwork);
            mCharInfo->lock();
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
        std::stringstream nameCaption, levelCaption, jobCaption, moneyCaption;

        nameCaption << pi->getName();
        levelCaption << "Lvl: " << pi->lvl;
        jobCaption << "Job Lvl: " << pi->jobLvl;
        moneyCaption << "Gold: " << pi->gp;

        nameLabel->setCaption(nameCaption.str());
        levelLabel->setCaption(levelCaption.str());
        jobLevelLabel->setCaption(jobCaption.str());
        moneyLabel->setCaption(moneyCaption.str());
        if (!mCharSelected)
        {
            newCharButton->setEnabled(false);
            delCharButton->setEnabled(true);
            selectButton->setEnabled(true);
        }
        playerBox->hairStyle = pi->getHairStyle() - 1;
        playerBox->hairColor = pi->getHairColor() - 1;
        playerBox->showPlayer = true;
    }
    else {
        nameLabel->setCaption("Name");
        levelLabel->setCaption("Level");
        jobLevelLabel->setCaption("Job Level");
        moneyLabel->setCaption("Money");
        newCharButton->setEnabled(true);
        delCharButton->setEnabled(false);
        selectButton->setEnabled(false);

        playerBox->hairStyle = 0;
        playerBox->hairColor = 0;
        playerBox->showPlayer = false;
    }
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

CharCreateDialog::CharCreateDialog(Window *parent, int slot, Network *network):
    Window("Create Character", true, parent), mNetwork(network), mSlot(slot)
{
    nameField = new TextField("");
    nameLabel = new gcn::Label("Name:");
    nextHairColorButton = new Button(">", "nextcolor", this);
    prevHairColorButton = new Button("<", "prevcolor", this);
    hairColorLabel = new gcn::Label("Hair Color:");
    nextHairStyleButton = new Button(">", "nextstyle", this);
    prevHairStyleButton = new Button("<", "prevstyle", this);
    hairStyleLabel = new gcn::Label("Hair Style:");
    createButton = new Button("Create", "create", this);
    cancelButton = new Button("Cancel", "cancel", this);
    playerBox = new PlayerBox();
    playerBox->showPlayer = true;

    nameField->setEventId("create");

    int w = 200;
    int h = 150;
    setContentSize(w, h);
    playerBox->setDimension(gcn::Rectangle(80, 30, 110, 85));
    nameLabel->setPosition(5, 5);
    nameField->setDimension(
            gcn::Rectangle(45, 5, w - 45 - 7, nameField->getHeight()));
    prevHairColorButton->setPosition(90, 35);
    nextHairColorButton->setPosition(165, 35);
    hairColorLabel->setPosition(5, 40);
    prevHairStyleButton->setPosition(90, 64);
    nextHairStyleButton->setPosition(165, 64);
    hairStyleLabel->setPosition(5, 70);
    cancelButton->setPosition(
            w - 5 - cancelButton->getWidth(),
            h - 5 - cancelButton->getHeight());
    createButton->setPosition(
            cancelButton->getX() - 5 - createButton->getWidth(),
            h - 5 - cancelButton->getHeight());

    nameField->addActionListener(this);

    add(playerBox);
    add(nameField);
    add(nameLabel);
    add(nextHairColorButton);
    add(prevHairColorButton);
    add(hairColorLabel);
    add(nextHairStyleButton);
    add(prevHairStyleButton);
    add(hairStyleLabel);
    add(createButton);
    add(cancelButton);

    setLocationRelativeTo(getParent());
}

void CharCreateDialog::action(const std::string& eventId)
{
    if (eventId == "create") {
        if (getName().length() >= 4) {
            // Attempt to create the character
            createButton->setEnabled(false);
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
        playerBox->hairColor++;
    }
    else if (eventId == "prevcolor") {
        playerBox->hairColor += NR_HAIR_COLORS - 1;
    }
    else if (eventId == "nextstyle") {
        playerBox->hairStyle++;
    }
    else if (eventId == "prevstyle") {
        playerBox->hairStyle += NR_HAIR_STYLES - 1;
    }

    playerBox->hairColor %= NR_HAIR_COLORS;
    playerBox->hairStyle %= NR_HAIR_STYLES;
}

std::string CharCreateDialog::getName()
{
    return nameField->getText();
}

void CharCreateDialog::attemptCharCreate()
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
    outMsg.writeInt16(playerBox->hairColor + 1);
    outMsg.writeInt16(playerBox->hairStyle + 1);
}
