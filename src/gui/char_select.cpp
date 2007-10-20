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

#include "widgets/layout.h"

#include "../game.h"
#include "../localplayer.h"
#include "../main.h"
#include "../logindata.h"

#include "../net/accountserver/account.h"
#include "../net/charserverhandler.h"
#include "../net/messageout.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
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
    ConfirmDialog(_("Confirm"),
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

CharSelectDialog::CharSelectDialog(LockedArray<LocalPlayer*> *charInfo,
                                   LoginData *loginData):
    Window(_("Select Character")),
    mCharInfo(charInfo), mCharSelected(false), mLoginData(loginData)
{

    mSelectButton = new Button(_("Ok"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mNewCharButton = new Button(_("New"), "new", this);
    mDelCharButton = new Button(_("Delete"), "delete", this);
    mPreviousButton = new Button(_("Previous"), "previous", this);
    mNextButton = new Button(_("Next"), "next", this);
    mUnRegisterButton = new Button(_("Unregister"), "unregister", this);

    mNameLabel = new gcn::Label(strprintf(_("Name: %s"), ""));
    mLevelLabel = new gcn::Label(strprintf(_("Level: %d"), 0));
    mMoneyLabel = new gcn::Label(strprintf(_("Money: %d"), 0));
    mPlayerBox = new PlayerBox();

    setPadding(8);
    Layout &layout = getLayout();
    place(0, 0, mPlayerBox, 1, 5).setPadding(3);
    place(1, 0, mNameLabel, 3);
    place(1, 1, mLevelLabel, 3);
    place(1, 2, mMoneyLabel, 3);
    place(1, 3, mPreviousButton);
    place(2, 3, mNextButton);
    place(1, 4, mNewCharButton);
    place(2, 4, mDelCharButton);
    layout.setWidth(230);
    layout.setColWidth(0, 80);
    layout.setColWidth(3, Layout::FILL);
    layout.matchColWidth(1, 2);
    layout.setRowHeight(5, 5);
    layout.flush();
    place(0, 0, mUnRegisterButton);
    place(2, 0, mSelectButton);
    place(3, 0, mCancelButton);
    layout.setColWidth(1, Layout::FILL);
    reflowLayout();
    forgetLayout();

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
        mNameLabel->setCaption(strprintf(_("Name: %s"), pi->getName().c_str()));
        mLevelLabel->setCaption(strprintf(_("Level: %d"), pi->getLevel()));
        mMoneyLabel->setCaption(strprintf(_("Money: %d"), pi->getMoney()));
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
        mMoneyLabel->setCaption(strprintf(_("Money: %d"), 0));
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

CharCreateDialog::CharCreateDialog(Window *parent, int slot):
    Window(_("Create Character"), true, parent), mSlot(slot)
{
    mPlayer = new Player(0, 0, NULL);
    mPlayer->setHairStyle(rand() % NR_HAIR_STYLES);
    mPlayer->setHairColor(rand() % NR_HAIR_COLORS);

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
    mAttributeLabel[0] = new gcn::Label(_("Strength:"));
    mAttributeLabel[1] = new gcn::Label(_("Agility:"));
    mAttributeLabel[2] = new gcn::Label(_("Dexterity:"));
    mAttributeLabel[3] = new gcn::Label(_("Vitality:"));
    mAttributeLabel[4] = new gcn::Label(_("Intelligence:"));
    mAttributeLabel[5] = new gcn::Label(_("Willpower:"));
    mAttributeLabel[6] = new gcn::Label(_("Charisma:"));
    for (int i=0; i<7; i++)
    {
        mAttributeLabel[i]->setWidth(70);
        mAttributeSlider[i] = new Slider(1, 20);
        mAttributeValue[i] = new gcn::Label("1");
    };

    mAttributesLeft = new gcn::Label(strprintf(_("Please distribute %d points"), 99));

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
            new OkDialog(_("Error"),
                    _("Your name needs to be at least 4 characters."), this);
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

std::string
CharCreateDialog::getName()
{
    std::string name = mNameField->getText();
    trim(name);
    return name;
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
