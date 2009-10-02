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

#include "gui/charselectdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/container.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/textfield.h"

#include "gui/changeemaildialog.h"
#include "gui/changepassworddialog.h"
#include "gui/charcreatedialog.h"
#include "gui/confirmdialog.h"
#include "gui/okdialog.h"
#include "gui/playerbox.h"
#include "gui/unregisterdialog.h"

#include "game.h"
#include "localplayer.h"
#include "main.h"
#include "units.h"

#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/accountserver/account.h"
#else
#include "net/ea/protocol.h"
#endif

#include "net/charhandler.h"
#include "net/logindata.h"
#include "net/messageout.h"
#include "net/net.h"

#include "resources/colordb.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>

#include <string>

/**
 * Listener for confirming character deletion.
 */
class CharDeleteConfirm : public ConfirmDialog
{
    public:
        CharDeleteConfirm(CharSelectDialog *m):
            ConfirmDialog(_("Confirm Character Delete"),
                    _("Are you sure you want to delete this character?"), m),
            master(m)
        {
        }
        void action(const gcn::ActionEvent &event)
        {
            //ConfirmDialog::action(event);
            if (event.getId() == "yes")
            {
                master->attemptCharDelete();
            }
            ConfirmDialog::action(event);
        }
    private:
        CharSelectDialog *master;
};

class CharEntry : public Container
{
    public:
        CharEntry(CharSelectDialog *m, char slot, LocalPlayer *chr);

        char getSlot()
        { return mSlot; }

        LocalPlayer *getChar()
        { return mChr; }

        void requestFocus();

        void update();

    protected:
        friend class CharSelectDialog;
        char mSlot;
        LocalPlayer *mChr;

        PlayerBox *mPlayerBox;
        Label *mName;
        Label *mMoney;
        Button *mButton;
};

bool CharSelectDialog::doAllowUnregister = true;
bool CharSelectDialog::doAllowChangeEmail = true;

CharSelectDialog::CharSelectDialog(LockedArray<LocalPlayer*> *charInfo,
                                   LoginData *loginData):
    Window(_("Account and Character Management")),
    mCharInfo(charInfo),
    mLoginData(loginData)
{
    setCloseButton(false);

    mAccountNameLabel = new Label(loginData->username);

    mSwitchLoginButton = new Button(_("Switch Login"), "switch", this);
    mUnregisterButton = new Button(_("Unregister"), "unregister", this);

    mChangePasswordButton = new Button(_("Change Password"), "change_password",
                                       this);
    mChangeEmailButton = new Button(_("Change Email"), "change_email", this);

    for (int i = 0; i < MAX_CHARACTER_COUNT; i++)
    {
        charInfo->select(i);
        mCharEntries[i] = new CharEntry(this, i, charInfo->getEntry());
    }

    place(0, 0, mAccountNameLabel, 6);

    place(0, 1, mSwitchLoginButton, 3);

    if (doAllowUnregister)
        place(3, 1, mUnregisterButton, 3);

    place(0, 2, mChangePasswordButton, 3);

    if (doAllowChangeEmail)
        place(3, 2, mChangeEmailButton, 3);

    place(0, 3, mCharEntries[0], 2, 3);
    place(2, 3, mCharEntries[1], 2, 3);
    place(4, 3, mCharEntries[2], 2, 3);

    int width = mAccountNameLabel->getWidth();
    width = std::max(width, mSwitchLoginButton->getWidth() +
                     mUnregisterButton->getWidth());
    width = std::max(width, mChangePasswordButton->getWidth() +
                     mChangeEmailButton->getWidth());
    width = std::max(width, 3 * mCharEntries[0]->getWidth());

    reflowLayout(width + 10, mAccountNameLabel->getHeight() +
                 mSwitchLoginButton->getHeight() +
                 mChangePasswordButton->getHeight() +
                 mCharEntries[0]->getHeight() + 20);

    center();
    mCharEntries[0]->requestFocus();
    setVisible(true);
}

void CharSelectDialog::action(const gcn::ActionEvent &event)
{
    CharEntry *entry = dynamic_cast<CharEntry*>(event.getSource()->getParent());

    // Update the locked array
    if (entry)
        mCharInfo->select(entry->getSlot());

    if (event.getId() == "use")
    {
        setVisible(false);
        attemptCharSelect();
    }
    else if (event.getId() == "switch")
    {
        mCharInfo->clear();
        state = STATE_SWITCH_SERVER_ATTEMPT;
    }
    else if (event.getId() == "new")
    {
        if (!(mCharInfo->getEntry()))
        {
            // Start new character dialog
            CharCreateDialog *charCreateDialog =
                new CharCreateDialog(this, mCharInfo->getPos());
            Net::getCharHandler()->setCharCreateDialog(charCreateDialog);
        }
    }
    else if (event.getId() == "delete")
    {
        if (mCharInfo->getEntry())
        {
            new CharDeleteConfirm(this);
        }
    }
    else if (event.getId() == "change_password")
    {
        new ChangePasswordDialog(this, mLoginData);
    }
    else if (event.getId() == "change_email")
    {
        new ChangeEmailDialog(this, mLoginData);
    }
    else if (event.getId() == "unregister")
    {
        new UnRegisterDialog(this, mLoginData);
    }
}

void CharSelectDialog::attemptCharDelete()
{
    Net::getCharHandler()->deleteCharacter(mCharInfo->getPos(), mCharInfo->getEntry());
    mCharInfo->lock();
}

void CharSelectDialog::attemptCharSelect()
{
    Net::getCharHandler()->chooseCharacter(mCharInfo->getPos(), mCharInfo->getEntry());
    mCharInfo->lock();
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
	{
            mCharEntries[mCharInfo->getPos()]->requestFocus();
	}

        mCharInfo->next();
    } while (mCharInfo->getPos());

    mCharInfo->select(oldPos);

    return false;
}

void CharSelectDialog::setNetworkOptions(bool allowUnregister,
                              bool allowChangeEmail)
{
    printf("Network options: %d, %d\n", allowUnregister, allowChangeEmail);
    doAllowUnregister = allowUnregister;
    doAllowChangeEmail = allowChangeEmail;
}

CharEntry::CharEntry(CharSelectDialog *m, char slot, LocalPlayer *chr):
        mSlot(slot),
        mChr(chr),
        mPlayerBox(new PlayerBox(chr))
{
    mButton = new Button("wwwwwwwww", "go", m);
    mName = new Label("wwwwwwwwwwwwwwwwwwwwwwww (888)");
    mMoney = new Label("wwwwwwwww");

    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mPlayerBox, 3, 5);
    place(0, 5, mName, 3);
    place(0, 6, mMoney, 3);
    place(0, 7, mButton, 3);

    h.reflowLayout(65, 120 + mName->getHeight() + mMoney->getHeight() +
                   mButton->getHeight());

    update();
}

void CharEntry::requestFocus()
{
    mButton->requestFocus();
}

void CharEntry::update()
{
    if (mChr)
    {
        mButton->setCaption(_("Choose"));
        mButton->setActionEventId("use");
        mName->setCaption(strprintf("%s (%d)", mChr->getName().c_str(),
                                    mChr->getLevel()));
        mMoney->setCaption(Units::formatCurrency(mChr->getMoney()));
    }
    else
    {
        mButton->setCaption(_("Create"));
        mButton->setActionEventId("new");
        mName->setCaption(_("(empty)"));
        mMoney->setCaption(Units::formatCurrency(0));
    }

    // Recompute layout
    distributeResizedEvent();
}
