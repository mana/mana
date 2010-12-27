/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "gui/charselectdialog.h"

#include "client.h"
#include "game.h"
#include "localplayer.h"
#include "units.h"
#include "log.h"

#include "gui/changeemaildialog.h"
#include "gui/changepassworddialog.h"
#include "gui/charcreatedialog.h"
#include "gui/confirmdialog.h"
#include "gui/okdialog.h"
#include "gui/sdlinput.h"
#include "gui/unregisterdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/container.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/playerbox.h"
#include "gui/widgets/textfield.h"

#include "net/charhandler.h"
#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/messageout.h"
#include "net/net.h"

#include "resources/colordb.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>

#include <string>
#include <cassert>

// Character slots per row in the dialog
static const int SLOTS_PER_ROW = 5;

/**
 * Listener for confirming character deletion.
 */
class CharDeleteConfirm : public ConfirmDialog
{
    public:
        CharDeleteConfirm(CharSelectDialog *m, int index):
            ConfirmDialog(_("Confirm Character Delete"),
                    _("Are you sure you want to delete this character?"), m),
            mMaster(m),
            mIndex(index)
        {
        }

        void action(const gcn::ActionEvent &event)
        {
            if (event.getId() == "yes")
                mMaster->attemptCharacterDelete(mIndex);

            ConfirmDialog::action(event);
        }

    private:
        CharSelectDialog *mMaster;
        int mIndex;
};

class CharacterDisplay : public Container
{
    public:
        CharacterDisplay(CharSelectDialog *charSelectDialog);

        void setCharacter(Net::Character *character);

        Net::Character *getCharacter() const
        { return mCharacter; }

        void requestFocus();

        void setActive(bool active);

    private:
        void update();

        Net::Character *mCharacter;

        PlayerBox *mPlayerBox;
        Label *mName;
        Label *mLevel;
        Label *mMoney;
        Button *mButton;
        Button *mDelete;
};

CharSelectDialog::CharSelectDialog(LoginData *loginData):
    Window(_("Account and Character Management")),
    mLocked(false),
    mUnregisterButton(0),
    mChangeEmailButton(0),
    mCharacterEntries(0),
    mLoginData(loginData),
    mCharHandler(Net::getCharHandler())
{
    setCloseButton(false);

    mAccountNameLabel = new Label(loginData->username);
    mSwitchLoginButton = new Button(_("Switch Login"), "switch", this);
    mChangePasswordButton = new Button(_("Change Password"), "change_password",
                                       this);

    int optionalActions = Net::getLoginHandler()->supportedOptionalActions();

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mAccountNameLabel, 2);
    place(0, 1, mSwitchLoginButton);

    if (optionalActions & Net::LoginHandler::Unregister)
    {
        mUnregisterButton = new Button(_("Unregister"),
                                       "unregister", this);
        place(3, 1, mUnregisterButton);
    }

    place(0, 2, mChangePasswordButton);

    if (optionalActions & Net::LoginHandler::ChangeEmail)
    {
        mChangeEmailButton = new Button(_("Change Email"),
                                        "change_email", this);
        place(3, 2, mChangeEmailButton);
    }

    place = getPlacer(0, 1);

    for (int i = 0; i < (int)mLoginData->characterSlots; i++)
    {
        mCharacterEntries.push_back(new CharacterDisplay(this));
        place(i % SLOTS_PER_ROW, (int)i / SLOTS_PER_ROW, mCharacterEntries[i]);
    }

    reflowLayout();

    addKeyListener(this);

    center();
    setVisible(true);

    Net::getCharHandler()->setCharSelectDialog(this);
    mCharacterEntries[0]->requestFocus();
}

CharSelectDialog::~CharSelectDialog()
{
}

void CharSelectDialog::action(const gcn::ActionEvent &event)
{
    // Check if a button of a character was pressed
    const gcn::Widget *sourceParent = event.getSource()->getParent();
    int selected = -1;
    for (int i = 0; i < (int)mCharacterEntries.size(); ++i)
    {
        if (mCharacterEntries[i] == sourceParent)
        {
            selected = i;
            break;
        }
    }

    const std::string &eventId = event.getId();

    if (selected != -1)
    {
        if (eventId == "use")
        {
            attemptCharacterSelect(selected);
        }
        else if (eventId == "new"
                 && !mCharacterEntries[selected]->getCharacter())
        {
            // Start new character dialog
            CharCreateDialog *charCreateDialog =
                    new CharCreateDialog(this, selected);
            mCharHandler->setCharCreateDialog(charCreateDialog);
        }
        else if (eventId == "delete"
                 && mCharacterEntries[selected]->getCharacter())
        {
            new CharDeleteConfirm(this, selected);
        }
    }
    else if (eventId == "switch")
    {
        Client::setState(STATE_SWITCH_LOGIN);
    }
    else if (eventId == "change_password")
    {
        Client::setState(STATE_CHANGEPASSWORD);
    }
    else if (eventId == "change_email")
    {
        Client::setState(STATE_CHANGEEMAIL);
    }
    else if (eventId == "unregister")
    {
        Client::setState(STATE_UNREGISTER);
    }
}

void CharSelectDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ESCAPE)
    {
        action(gcn::ActionEvent(mSwitchLoginButton,
                                mSwitchLoginButton->getActionEventId()));
    }
}

/**
 * Communicate character deletion to the server.
 */
void CharSelectDialog::attemptCharacterDelete(int index)
{
    if (mLocked)
        return;

    mCharHandler->deleteCharacter(mCharacterEntries[index]->getCharacter());
    lock();
}

/**
 * Communicate character selection to the server.
 */
void CharSelectDialog::attemptCharacterSelect(int index)
{
    if (mLocked)
        return;

    setVisible(false);
    mCharHandler->chooseCharacter(mCharacterEntries[index]->getCharacter());
    lock();
}

void CharSelectDialog::setCharacters(const Net::Characters &characters)
{
    // Reset previous characters
    std::vector<CharacterDisplay*>::iterator iter, iter_end;
    for (iter = mCharacterEntries.begin(), iter_end = mCharacterEntries.end();
         iter != iter_end; ++iter)
        (*iter)->setCharacter(0);

    Net::Characters::const_iterator i, i_end = characters.end();
    for (i = characters.begin(); i != i_end; ++i)
    {
        Net::Character *character = *i;

        // Slots Number start at 1 for Manaserv, so we offset them by one.
        int characterSlot = character->slot;
        if (Net::getNetworkType() == ServerInfo::MANASERV
            && characterSlot > 0)
            --characterSlot;

        if (characterSlot >= (int)mCharacterEntries.size())
        {
            logger->log("Warning: slot out of range: %d", character->slot);
            continue;
        }

        mCharacterEntries[characterSlot]->setCharacter(character);
    }
}

void CharSelectDialog::lock()
{
    assert(!mLocked);
    setLocked(true);
}

void CharSelectDialog::unlock()
{
    setLocked(false);
}

void CharSelectDialog::setLocked(bool locked)
{
    mLocked = locked;

    mSwitchLoginButton->setEnabled(!locked);
    mChangePasswordButton->setEnabled(!locked);
    if (mUnregisterButton)
        mUnregisterButton->setEnabled(!locked);
    if (mChangeEmailButton)
        mChangeEmailButton->setEnabled(!locked);

    for (int i = 0; i < (int)mCharacterEntries.size(); ++i)
        mCharacterEntries[i]->setActive(!mLocked);
}

bool CharSelectDialog::selectByName(const std::string &name,
                                    SelectAction action)
{
    if (mLocked)
        return false;

    for (int i = 0; i < (int)mCharacterEntries.size(); ++i)
    {
        if (Net::Character *character = mCharacterEntries[i]->getCharacter())
        {
            if (character->dummy->getName() == name)
            {
                mCharacterEntries[i]->requestFocus();
                if (action == Choose)
                    attemptCharacterSelect(i);
                return true;
            }
        }
    }

    return false;
}


CharacterDisplay::CharacterDisplay(CharSelectDialog *charSelectDialog):
    mCharacter(0),
    mPlayerBox(new PlayerBox)
{
    mButton = new Button("", "go", charSelectDialog);
    mName = new Label("");
    mLevel = new Label("");
    mMoney = new Label("");

    mDelete = new Button(_("Delete"), "delete", charSelectDialog);

    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mPlayerBox, 3, 5);
    place(0, 5, mName, 3);
    place(0, 6, mLevel, 3);
    place(0, 7, mMoney, 3);
    place(0, 8, mButton, 3);
    place(0, 9, mDelete, 3);

    update();

    h.reflowLayout(80, 112 + mName->getHeight() + mLevel->getHeight() +
            mMoney->getHeight() + mButton->getHeight() + mDelete->getHeight());
}

void CharacterDisplay::setCharacter(Net::Character *character)
{
    if (mCharacter == character)
        return;

    mCharacter = character;
    mPlayerBox->setPlayer(character ? character->dummy : 0);
    update();
}

void CharacterDisplay::requestFocus()
{
    mButton->requestFocus();
}

void CharacterDisplay::setActive(bool active)
{
    mButton->setEnabled(active);
    mDelete->setEnabled(active);
}

void CharacterDisplay::update()
{
    if (mCharacter)
    {
        const LocalPlayer *character = mCharacter->dummy;
        mButton->setCaption(_("Choose"));
        mButton->setActionEventId("use");
        mName->setCaption(strprintf("%s", character->getName().c_str()));
        mLevel->setCaption(strprintf("Level %d",
                           mCharacter->data.mAttributes[LEVEL]));
        mMoney->setCaption(Units::formatCurrency(
                           mCharacter->data.mAttributes[MONEY]));

        mDelete->setVisible(true);
    }
    else
    {
        mButton->setCaption(_("Create"));
        mButton->setActionEventId("new");
        mName->setCaption(_("(empty)"));
        mLevel->setCaption(_("(empty)"));
        mMoney->setCaption(Units::formatCurrency(0));

        mDelete->setVisible(false);
    }

    // Recompute layout
    distributeResizedEvent();
}
