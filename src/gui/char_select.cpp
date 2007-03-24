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
#include "slider.h"
#include "textfield.h"

#include "unregisterdialog.h"

#include "../game.h"
#include "../localplayer.h"
#include "../main.h"
#include "../logindata.h"

#include "../net/accountserver/account.h"
#include "../net/charserverhandler.h"
#include "../net/messageout.h"

#include "../utils/tostring.h"

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
    ConfirmDialog("Confirm",
                  "Are you sure you want to delete this character?", m),
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

CharSelectDialog::CharSelectDialog(LockedArray<LocalPlayer*> *charInfo,
                                   LoginData *loginData):
    Window("Select Character"),
    mCharInfo(charInfo), mCharSelected(false), mLoginData(loginData)
{

    mSelectButton = new Button("Ok", "ok", this);
    mCancelButton = new Button("Cancel", "cancel", this);
    mNewCharButton = new Button("New", "new", this);
    mDelCharButton = new Button("Delete", "delete", this);
    mPreviousButton = new Button("Previous", "previous", this);
    mNextButton = new Button("Next", "next", this);
    mUnRegisterButton = new Button("Unregister", "unregister", this);

    mNameLabel = new gcn::Label("Name");
    mLevelLabel = new gcn::Label("Level");
    mMoneyLabel = new gcn::Label("Money");
    mPlayerBox = new PlayerBox();

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
    mUnRegisterButton->setPosition(
            w - 5 - mUnRegisterButton->getWidth(),
            mCancelButton->getY() - 5 - mUnRegisterButton->getHeight());

    add(mPlayerBox);
    add(mSelectButton);
    add(mCancelButton);
    add(mUnRegisterButton);
    add(mNewCharButton);
    add(mDelCharButton);
    add(mPreviousButton);
    add(mNextButton);
    add(mNameLabel);
    add(mLevelLabel);
    add(mMoneyLabel);

    setLocationRelativeTo(getParent());
    setVisible(true);
    mSelectButton->requestFocus();
    updatePlayerInfo();
}

void CharSelectDialog::action(const gcn::ActionEvent &event)
{
    // The pointers are set to NULL if there is no character stored
    if (event.getId() == "ok" && (mCharInfo->getEntry()))
    {
        // Start game
        mNewCharButton->setEnabled(false);
        mDelCharButton->setEnabled(false);
        mSelectButton->setEnabled(false);
        mUnRegisterButton->setEnabled(false);
        mPreviousButton->setEnabled(false);
        mNextButton->setEnabled(false);
        mCharSelected = true;
        Net::AccountServer::Account::selectCharacter(mCharInfo->getPos());
        mCharInfo->lock();
    }
    else if (event.getId() == "cancel")
    {
        state = STATE_EXIT;
    }
    else if (event.getId() == "new")
    {
        // TODO: Search the first free slot, and start CharCreateDialog
        //       maybe add that search to the constructor.
        if (!(mCharInfo->getEntry()))
        {
            // Start new character dialog
            CharCreateDialog *charCreateDialog =
                new CharCreateDialog(this, mCharInfo->getPos());
            charServerHandler.setCharCreateDialog(charCreateDialog);
        }
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
    else if (event.getId() == "unregister")
    {
        new UnRegisterDialog(this, mLoginData);
    }
}

void CharSelectDialog::updatePlayerInfo()
{
    LocalPlayer *pi = mCharInfo->getEntry();

    if (pi)
    {
        mNameLabel->setCaption(pi->getName());
        mLevelLabel->setCaption("Lvl: " + toString(pi->getLevel()));
        mMoneyLabel->setCaption("Money: " + toString(pi->getMoney()));
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
        mMoneyLabel->setCaption("Money");
        mNewCharButton->setEnabled(true);
        mDelCharButton->setEnabled(false);
        mSelectButton->setEnabled(false);
    }

    mPlayerBox->setPlayer(pi);
}

void CharSelectDialog::attemptCharDelete()
{
    Net::AccountServer::Account::deleteCharacter(mCharInfo->getPos());
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

std::string CharSelectDialog::getName()
{
    return mNameLabel->getCaption();
}

CharCreateDialog::CharCreateDialog(Window *parent, int slot):
    Window("Create Character", true, parent), mSlot(slot)
{
    mPlayer = new Player(0, 0, NULL);
    mPlayer->setHairStyle(rand() % NR_HAIR_STYLES);
    mPlayer->setHairColor(rand() % NR_HAIR_COLORS);

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
    mAttributeLabel[0] = new gcn::Label("Strength:");
    mAttributeLabel[1] = new gcn::Label("Agility:");
    mAttributeLabel[2] = new gcn::Label("Dexterity:");
    mAttributeLabel[3] = new gcn::Label("Vitality:");
    mAttributeLabel[4] = new gcn::Label("Intelligence:");
    mAttributeLabel[5] = new gcn::Label("Willpower:");
    mAttributeLabel[6] = new gcn::Label("Charisma:");
    for (int i=0; i<7; i++)
    {
        mAttributeLabel[i]->setWidth(70);
        mAttributeSlider[i] = new Slider(1, 20);
        mAttributeValue[i] = new gcn::Label("1");
    };

    mAttributesLeft = new gcn::Label("Please distribute # points");

    mNameField->setActionEventId("create");

    int w = 200;
    int h = 330;
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
    for (int i=0; i<7; i++)
    {
        mAttributeSlider[i]->setValue(10);
        mAttributeSlider[i]->setDimension(gcn::Rectangle(   75, 140 + i*20,
                                                            100, 10));
        mAttributeSlider[i]->setActionEventId("statslider");
        mAttributeSlider[i]->addActionListener(this);
        mAttributeValue[i]->setPosition(180, 140 + i*20);
        mAttributeLabel[i]->setPosition(5, 140 + i*20);
    };
    mAttributesLeft->setPosition(15, 280);
    UpdateSliders();
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
    for (int i=0; i<7; i++)
    {
        add(mAttributeSlider[i]);
        add(mAttributeValue[i]);
        add(mAttributeLabel[i]);
    };
    add(mAttributesLeft);
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
    if (event.getId() == "create") {
        if (getName().length() >= 4) {
            // Attempt to create the character
            mCreateButton->setEnabled(false);
            Net::AccountServer::Account::createCharacter(
                    getName(),
                    mPlayer->getHairStyle(),
                    mPlayer->getHairColor(),
                    0,   // gender
                    (int) mAttributeSlider[0]->getValue(),  // STR
                    (int) mAttributeSlider[1]->getValue(),  // AGI
                    (int) mAttributeSlider[2]->getValue(),  // DEX
                    (int) mAttributeSlider[3]->getValue(),  // VIT
                    (int) mAttributeSlider[4]->getValue(),  // INT
                    (int) mAttributeSlider[5]->getValue(),  // WILL
                    (int) mAttributeSlider[6]->getValue()   // CHAR
            );
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
        mPlayer->setHairColor((mPlayer->getHairColor() + 1) % NR_HAIR_COLORS);
    }
    else if (event.getId() == "prevcolor") {
        int prevColor = mPlayer->getHairColor() + NR_HAIR_COLORS - 1;
        mPlayer->setHairColor(prevColor % NR_HAIR_COLORS);
    }
    else if (event.getId() == "nextstyle") {
        mPlayer->setHairStyle((mPlayer->getHairStyle() + 1) % NR_HAIR_STYLES);
    }
    else if (event.getId() == "prevstyle") {
        int prevStyle = mPlayer->getHairStyle() + NR_HAIR_STYLES - 1;
        mPlayer->setHairStyle(prevStyle % NR_HAIR_STYLES);
    }
    else if (event.getId() == "statslider") {
        UpdateSliders();
    }
}

const std::string&
CharCreateDialog::getName()
{
    return mNameField->getText();
}

void CharCreateDialog::UpdateSliders()
{
    for (int i = 0; i < 7; i++)
    {
        // Update captions
        mAttributeValue[i]->setCaption(
                toString((int) (mAttributeSlider[i]->getValue())));
        mAttributeValue[i]->adjustSize();
    }

    // Update distributed points
    int pointsLeft = 70 - getDistributedPoints();
    if (pointsLeft == 0)
    {
        mAttributesLeft->setCaption("Character stats OK");
        mCreateButton->setEnabled(true);
    }
    else
    {
        mCreateButton->setEnabled(false);
        if (pointsLeft > 0)
        {
            mAttributesLeft->setCaption(std::string("Please distribute " +
                                        toString(pointsLeft) + " points"));
        }
        else
        {
            mAttributesLeft->setCaption(std::string("Please remove " +
                                        toString(-pointsLeft) + " points"));
        }
    }

    mAttributesLeft->adjustSize();
}

void
CharCreateDialog::unlock()
{
    mCreateButton->setEnabled(true);
}

int CharCreateDialog::getDistributedPoints()
{
    int points = 0;

    for (int i = 0; i < 7; i++)
    {
        points += (int) mAttributeSlider[i]->getValue();
    }
    return points;
}
