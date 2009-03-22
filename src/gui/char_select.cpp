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

#include <string>

#include <guichan/font.hpp>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "char_select.h"
#include "confirm_dialog.h"
#include "ok_dialog.h"
#include "playerbox.h"
#include "textfield.h"

#ifdef TMWSERV_SUPPORT
#include "radiobutton.h"
#include "slider.h"

#include "unregisterdialog.h"
#include "changepassworddialog.h"
#include "changeemaildialog.h"

#include "../logindata.h"

#include "../net/accountserver/account.h"
#endif

#include "widgets/layout.h"

#include "../game.h"
#include "../localplayer.h"
#include "../main.h"
#include "../units.h"

#include "../net/charserverhandler.h"
#include "../net/messageout.h"

#include "../resources/colordb.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/stringutils.h"

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
    if (event.getId() == "yes")
    {
        master->attemptCharDelete();
    }
    ConfirmDialog::action(event);
}

#ifdef TMWSERV_SUPPORT
CharSelectDialog::CharSelectDialog(LockedArray<LocalPlayer*> *charInfo,
                                   LoginData *loginData):
    Window(_("Account and Character Management")),
    mCharInfo(charInfo), mCharSelected(false), mLoginData(loginData)
#else
CharSelectDialog::CharSelectDialog(Network *network,
                                   LockedArray<LocalPlayer*> *charInfo,
                                   Gender gender):
    Window(_("Select Character")), mNetwork(network),
    mCharInfo(charInfo),
    mCharSelected(false),
    mGender(gender)
#endif
{
    mSelectButton = new Button(_("Ok"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mPreviousButton = new Button(_("Previous"), "previous", this);
    mNextButton = new Button(_("Next"), "next", this);
    mNameLabel = new gcn::Label(strprintf(_("Name: %s"), ""));
    mLevelLabel = new gcn::Label(strprintf(_("Level: %d"), 0));
#ifdef TMWSERV_SUPPORT
    mNewCharButton = new Button(_("New"), "new", this);
    mDelCharButton = new Button(_("Delete"), "delete", this);
    mUnRegisterButton = new Button(_("Unregister"), "unregister", this);
    mChangePasswordButton = new Button(_("Change Password"), "change_password", this);
    mChangeEmailButton = new Button(_("Change Email Address"), "change_email", this);

    mAccountNameLabel = new gcn::Label(strprintf(_("Account: %s"), mLoginData->username.c_str()));
    mNameLabel = new gcn::Label(strprintf(_("Name: %s"), ""));
    mLevelLabel = new gcn::Label(strprintf(_("Level: %d"), 0));
    mMoneyLabel = new gcn::Label(strprintf(_("Money: %d"), 0));

    // Control that shows the Player
    mPlayerBox = new PlayerBox;
    mPlayerBox->setWidth(74);

    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, mAccountNameLabel);
    place(0, 1, mUnRegisterButton);
    place(0, 2, mChangePasswordButton);
    place(1, 2, mChangeEmailButton);
    place = getPlacer(0, 1);
    place(0, 0, mPlayerBox, 1, 5).setPadding(3);
    place(1, 0, mNameLabel, 3);
    place(1, 1, mLevelLabel, 3);
    place(1, 2, mMoneyLabel, 3);
    place(1, 3, mPreviousButton);
    place(2, 3, mNextButton);
    place(1, 4, mNewCharButton);
    place(2, 4, mDelCharButton);
    place.getCell().matchColWidth(1, 2);
    place = getPlacer(0, 2);
    place(0, 0, mSelectButton);
    place(1, 0, mCancelButton);
    reflowLayout(265, 0);
#else
    mCharInfo->select(0);
    LocalPlayer *pi = mCharInfo->getEntry();
    if (pi)
        mMoney = Units::formatCurrency(pi->getMoney());
    // Control that shows the Player
    mPlayerBox = new PlayerBox;
    mPlayerBox->setWidth(74);

    mJobLevelLabel = new gcn::Label(strprintf(_("Job Level: %d"), 0));
    mMoneyLabel = new gcn::Label(strprintf(_("Money: %s"), mMoney.c_str()));

    const std::string tempString = getFont()->getWidth(_("New")) <
                                   getFont()->getWidth(_("Delete")) ?
                                   _("Delete") : _("New");

    mNewDelCharButton = new Button(tempString, "newdel", this);

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mPlayerBox, 1, 6).setPadding(3);
    place(1, 0, mNewDelCharButton);
    place(1, 1, mNameLabel, 5);
    place(1, 2, mLevelLabel, 5);
    place(1, 3, mJobLevelLabel, 5);
    place(1, 4, mMoneyLabel, 5);
    place.getCell().matchColWidth(1, 4);
    place = getPlacer(0, 2);
    place(0, 0, mPreviousButton);
    place(1, 0, mNextButton);
    place(4, 0, mCancelButton);
    place(5, 0, mSelectButton);

    reflowLayout(250, 0);
#endif

    setLocationRelativeTo(getParent());
    setVisible(true);
    mSelectButton->requestFocus();
    updatePlayerInfo();
}

void CharSelectDialog::action(const gcn::ActionEvent &event)
{
#ifdef TMWSERV_SUPPORT
    // The pointers are set to NULL if there is no character stored
    if (event.getId() == "ok" && (mCharInfo->getEntry()))
#else
    if (event.getId() == "ok" && n_character > 0)
#endif
    {
        // Start game
#ifdef TMWSERV_SUPPORT
        mNewCharButton->setEnabled(false);
        mDelCharButton->setEnabled(false);
        mUnRegisterButton->setEnabled(false);
#else
        mNewDelCharButton->setEnabled(false);
#endif
        mSelectButton->setEnabled(false);
        mPreviousButton->setEnabled(false);
        mNextButton->setEnabled(false);
        mCharSelected = true;
        attemptCharSelect();
    }
    else if (event.getId() == "cancel")
    {
#ifdef TMWSERV_SUPPORT
        mCharInfo->clear();
        state = STATE_SWITCH_ACCOUNTSERVER_ATTEMPT;
#else
        state = STATE_EXIT;
#endif
    }
#ifdef TMWSERV_SUPPORT
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
#else
    else if (event.getId() == "newdel")
    {
        // Check for a character
        if (mCharInfo->getEntry() && n_character <= MAX_SLOT + 1)
        {
            new CharDeleteConfirm(this);
        }
        else
        {
            // Start new character dialog
            CharCreateDialog *charCreateDialog =
                new CharCreateDialog(this, mCharInfo->getPos(),
                                     mNetwork, mGender);
            charServerHandler.setCharCreateDialog(charCreateDialog);
        }
    }
#endif
    else if (event.getId() == "previous")
    {
        mCharInfo->prev();
        LocalPlayer *pi = mCharInfo->getEntry();
        if (pi)
            mMoney = Units::formatCurrency(pi->getMoney());
    }
    else if (event.getId() == "next")
    {
        mCharInfo->next();
        LocalPlayer *pi = mCharInfo->getEntry();
        if (pi)
            mMoney = Units::formatCurrency(pi->getMoney());
    }
#ifdef TMWSERV_SUPPORT
    else if (event.getId() == "unregister")
    {
        new UnRegisterDialog(this, mLoginData);
    }
    else if (event.getId() == "change_password")
    {
        new ChangePasswordDialog(this, mLoginData);
    }
    else if (event.getId() == "change_email")
    {
        new ChangeEmailDialog(this, mLoginData);
    }
#endif
}

void CharSelectDialog::updatePlayerInfo()
{
    LocalPlayer *pi = mCharInfo->getEntry();

    if (pi)
    {
        mNameLabel->setCaption(strprintf(_("Name: %s"),
                                          pi->getName().c_str()));
        mLevelLabel->setCaption(strprintf(_("Level: %d"), pi->getLevel()));
#ifndef TMWSERV_SUPPORT
        mJobLevelLabel->setCaption(strprintf(_("Job Level: %d"),
                                              pi->mJobLevel));
#endif
        mMoneyLabel->setCaption(strprintf(_("Money: %s"), mMoney.c_str()));
        if (!mCharSelected)
        {
#ifdef TMWSERV_SUPPORT
            mNewCharButton->setEnabled(false);
            mDelCharButton->setEnabled(true);
#else
            mNewDelCharButton->setCaption(_("Delete"));
#endif
            mSelectButton->setEnabled(true);
        }
    }
    else
    {
        mNameLabel->setCaption(strprintf(_("Name: %s"), ""));
        mLevelLabel->setCaption(strprintf(_("Level: %d"), 0));
#ifndef TMWSERV_SUPPORT
        mJobLevelLabel->setCaption(strprintf(_("Job Level: %d"), 0));
#endif
        mMoneyLabel->setCaption(strprintf(_("Money: %s"), ""));
#ifdef TMWSERV_SUPPORT
        mNewCharButton->setEnabled(true);
        mDelCharButton->setEnabled(false);
#else
        mNewDelCharButton->setCaption(_("New"));
#endif
        mSelectButton->setEnabled(false);
    }

    mPlayerBox->setPlayer(pi);
}

void CharSelectDialog::attemptCharDelete()
{
#ifdef TMWSERV_SUPPORT
    Net::AccountServer::Account::deleteCharacter(mCharInfo->getPos());
#else
    // Request character deletion
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x0068);
    outMsg.writeInt32(mCharInfo->getEntry()->mCharId);
    outMsg.writeString("a@a.com", 40);
#endif
    mCharInfo->lock();
}

void CharSelectDialog::attemptCharSelect()
{
#ifdef TMWSERV_SUPPORT
    Net::AccountServer::Account::selectCharacter(mCharInfo->getPos());
#else
    // Request character selection
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x0066);
    outMsg.writeInt8(mCharInfo->getPos());
#endif
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
    do
    {
        LocalPlayer *player = mCharInfo->getEntry();

        if (player && player->getName() == name)
            return true;

        mCharInfo->next();
    } while (mCharInfo->getPos());

    mCharInfo->select(oldPos);

    return false;
}

#ifdef TMWSERV_SUPPORT
CharCreateDialog::CharCreateDialog(Window *parent, int slot):
#else
CharCreateDialog::CharCreateDialog(Window *parent, int slot, Network *network,
                                   Gender gender):
#endif
    Window(_("Create Character"), true, parent),
#ifndef TMWSERV_SUPPORT
    mNetwork(network),
#endif
    mSlot(slot)
{
    mPlayer = new Player(0, 0, NULL);
#ifdef TMWSERV_SUPPORT
    mPlayer->setGender(GENDER_MALE);
#else
    mPlayer->setGender(gender);
#endif

    int numberOfHairColors = ColorDB::size();

    mPlayer->setHairStyle(rand() % mPlayer->getNumOfHairstyles(),
                          rand() % numberOfHairColors);

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
#ifdef TMWSERV_SUPPORT
    mMale = new RadioButton(_("Male"), "gender");
    mFemale = new RadioButton(_("Female"), "gender");

    // Default to a Male character
    mMale->setSelected(true);

    mMale->setActionEventId("gender");
    mFemale->setActionEventId("gender");

    mMale->addActionListener(this);
    mFemale->addActionListener(this);
#endif
    mPlayerBox = new PlayerBox(mPlayer);

    mPlayerBox->setWidth(74);

    mNameField->setActionEventId("create");
    mNameField->addActionListener(this);

#ifdef TMWSERV_SUPPORT
    mAttributeLabel[0] = new gcn::Label(_("Strength:"));
    mAttributeLabel[1] = new gcn::Label(_("Agility:"));
    mAttributeLabel[2] = new gcn::Label(_("Dexterity:"));
    mAttributeLabel[3] = new gcn::Label(_("Vitality:"));
    mAttributeLabel[4] = new gcn::Label(_("Intelligence:"));
    mAttributeLabel[5] = new gcn::Label(_("Willpower:"));
    for (int i = 0; i < 6; i++)
    {
        mAttributeLabel[i]->setWidth(70);
        mAttributeSlider[i] = new Slider(1, 20);
        mAttributeValue[i] = new gcn::Label("1");
    };

    mAttributesLeft = new gcn::Label(strprintf(_("Please distribute %d points"), 99));

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
    for (int i=0; i<6; i++)
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

    mMale->setPosition(30, 120);
    mFemale->setPosition(100, 120);

    add(mPlayerBox);
    add(mNameField);
    add(mNameLabel);
    add(mNextHairColorButton);
    add(mPrevHairColorButton);
    add(mHairColorLabel);
    add(mNextHairStyleButton);
    add(mPrevHairStyleButton);
    add(mHairStyleLabel);
    for (int i = 0; i < 6; i++)
    {
        add(mAttributeSlider[i]);
        add(mAttributeValue[i]);
        add(mAttributeLabel[i]);
    };
    add(mAttributesLeft);
    add(mCreateButton);
    add(mCancelButton);

    add(mMale);
    add(mFemale);

#else

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mNameLabel, 1);
    place(1, 0, mNameField, 6);
    place(0, 1, mHairStyleLabel, 1);
    place(1, 1, mPrevHairStyleButton);
    place(2, 1, mPlayerBox, 1, 8).setPadding(3);
    place(3, 1, mNextHairStyleButton);
    place(0, 2, mHairColorLabel, 1);
    place(1, 2, mPrevHairColorButton);
    place(3, 2, mNextHairColorButton);
    place.getCell().matchColWidth(0, 2);
    place = getPlacer(0, 2);
    place(4, 0, mCancelButton);
    place(5, 0, mCreateButton);

    reflowLayout(225, 0);
#endif

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

void CharCreateDialog::action(const gcn::ActionEvent &event)
{
    int numberOfColors = ColorDB::size();
    if (event.getId() == "create")
    {
        if (getName().length() >= 4)
        {
            // Attempt to create the character
            mCreateButton->setEnabled(false);
#ifdef TMWSERV_SUPPORT
            unsigned int genderSelected;
            if (mMale->isSelected()) {
                genderSelected = GENDER_MALE;
            } else {
                genderSelected = GENDER_FEMALE;
            }

            Net::AccountServer::Account::createCharacter(
                    getName(),
                    mPlayer->getHairStyle(),
                    mPlayer->getHairColor(),
                    genderSelected,   // gender
                    (int) mAttributeSlider[0]->getValue(),  // STR
                    (int) mAttributeSlider[1]->getValue(),  // AGI
                    (int) mAttributeSlider[2]->getValue(),  // DEX
                    (int) mAttributeSlider[3]->getValue(),  // VIT
                    (int) mAttributeSlider[4]->getValue(),  // INT
                    (int) mAttributeSlider[5]->getValue()  // WILL
            );
#else
            attemptCharCreate();
#endif
        }
        else
        {
            new OkDialog(_("Error"),
                         _("Your name needs to be at least 4 characters."),
                         this);
        }
    }
    else if (event.getId() == "cancel")
        scheduleDelete();
    else if (event.getId() == "nextcolor")
        mPlayer->setHairStyle(mPlayer->getHairStyle(),
                             (mPlayer->getHairColor() + 1) % numberOfColors);
    else if (event.getId() == "prevcolor")
        mPlayer->setHairStyle(mPlayer->getHairStyle(),
                             (mPlayer->getHairColor() + numberOfColors - 1) %
                              numberOfColors);
    else if (event.getId() == "nextstyle")
        mPlayer->setHairStyle(mPlayer->getHairStyle() + 1,
                              mPlayer->getHairColor());
    else if (event.getId() == "prevstyle")
        mPlayer->setHairStyle(mPlayer->getHairStyle() +
                              mPlayer->getNumOfHairstyles() - 1,
                              mPlayer->getHairColor());
#ifdef TMWSERV_SUPPORT
    else if (event.getId() == "statslider") {
        UpdateSliders();
    }
    else if (event.getId() == "gender"){
        if (mMale->isSelected()) {
            mPlayer->setGender(GENDER_MALE);
        } else {
            mPlayer->setGender(GENDER_FEMALE);
        }
    }
#endif
}

std::string CharCreateDialog::getName()
{
    std::string name = mNameField->getText();
    trim(name);
    return name;
}

#ifdef TMWSERV_SUPPORT
void CharCreateDialog::UpdateSliders()
{
    for (int i = 0; i < 6; i++)
    {
        // Update captions
        mAttributeValue[i]->setCaption(
                toString((int) (mAttributeSlider[i]->getValue())));
        mAttributeValue[i]->adjustSize();
    }

    // Update distributed points
    int pointsLeft = 60 - getDistributedPoints();
    if (pointsLeft == 0)
    {
        mAttributesLeft->setCaption(_("Character stats OK"));
        mCreateButton->setEnabled(true);
    }
    else
    {
        mCreateButton->setEnabled(false);
        if (pointsLeft > 0)
        {
            mAttributesLeft->setCaption(strprintf(_("Please distribute %d points"), pointsLeft));
        }
        else
        {
            mAttributesLeft->setCaption(strprintf(_("Please remove %d points"), -pointsLeft));
        }
    }

    mAttributesLeft->adjustSize();
}
#endif

void CharCreateDialog::unlock()
{
    mCreateButton->setEnabled(true);
}

#ifdef TMWSERV_SUPPORT
int CharCreateDialog::getDistributedPoints()
{
    int points = 0;

    for (int i = 0; i < 6; i++)
    {
        points += (int) mAttributeSlider[i]->getValue();
    }
    return points;
}
#endif

#ifndef TMWSERV_SUPPORT
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
    outMsg.writeInt16(mPlayer->getHairColor());
    outMsg.writeInt16(mPlayer->getHairStyle());
}
#endif
