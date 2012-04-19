/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#include "gui/socialwindow.h"

#include "event.h"
#include "guild.h"
#include "localplayer.h"
#include "party.h"

#include "gui/confirmdialog.h"
#include "gui/okdialog.h"
#include "gui/setup.h"
#include "gui/textdialog.h"

#include "gui/widgets/avatarlistbox.h"
#include "gui/widgets/browserbox.h"
#include "gui/widgets/button.h"
#include "gui/widgets/container.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/linkhandler.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/tabbedarea.h"

#include "net/net.h"
#include "net/guildhandler.h"
#include "net/partyhandler.h"

#include "resources/theme.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

class SocialTab : public Tab
{
protected:
    friend class SocialWindow;

    SocialTab():
            mInviteDialog(NULL),
            mConfirmDialog(NULL)
    {}

    virtual ~SocialTab()
    {
        // Cleanup dialogs
        if (mInviteDialog)
        {
            mInviteDialog->close();
            mInviteDialog->scheduleDelete();
            mInviteDialog = NULL;
        }

        if (mConfirmDialog)
        {
            mConfirmDialog->close();
            mConfirmDialog->scheduleDelete();
            mConfirmDialog = NULL;
        }
    }

    virtual void invite() = 0;

    virtual void leave() = 0;

    TextDialog *mInviteDialog;
    ConfirmDialog *mConfirmDialog;
    ScrollArea *mScroll;
    AvatarListBox *mList;
};

class GuildTab : public SocialTab, public gcn::ActionListener
{
public:
    GuildTab(Guild *guild):
            mGuild(guild)
    {
        setCaption(guild->getName());

        setTabColor(&Theme::getThemeColor(Theme::GUILD));

        mList = new AvatarListBox(guild);
        mScroll = new ScrollArea(mList);

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_ALWAYS);
    }

    ~GuildTab()
    {
        delete mList;
        mList = 0;
        delete mScroll;
        mScroll = 0;
    }

    void action(const gcn::ActionEvent &event)
    {
        if (event.getId() == "do invite")
        {
            std::string name = mInviteDialog->getText();

            if (!name.empty())
            {
                Net::getGuildHandler()->invite(mGuild->getId(), name);
                SERVER_NOTICE(strprintf(_("Invited user %s to guild %s."),
                              name.c_str(),
                              mGuild->getName().c_str()))
            }
            mInviteDialog = NULL;
        }
        else if (event.getId() == "~do invite")
        {
            mInviteDialog = NULL;
        }
        else if (event.getId() == "yes")
        {
            Net::getGuildHandler()->leave(mGuild->getId());
            SERVER_NOTICE(strprintf(_("Guild %s quit requested."),
                                        mGuild->getName().c_str()))
            mConfirmDialog = NULL;
        }
        else if (event.getId() == "no")
        {
            mConfirmDialog = NULL;
        }
    }

protected:
    void invite()
    {
        // TODO - Give feedback on whether the invite succeeded
        mInviteDialog = new TextDialog(_("Member Invite to Guild"),
                     strprintf(_("Who would you like to invite to guild %s?"),
                               mGuild->getName().c_str()),
                     socialWindow, true);
        mInviteDialog->setActionEventId("do invite");
        mInviteDialog->addActionListener(this);
    }

    void leave()
    {
        mConfirmDialog = new ConfirmDialog(_("Leave Guild?"),
                       strprintf(_("Are you sure you want to leave guild %s?"),
                                 mGuild->getName().c_str()),
                       socialWindow);

        mConfirmDialog->addActionListener(this);
    }

private:
    Guild *mGuild;
};

class PartyTab : public SocialTab, public gcn::ActionListener
{
public:
    PartyTab(Party *party):
            mParty(party)
    {
        setCaption(party->getName());

        setTabColor(&Theme::getThemeColor(Theme::PARTY_SOCIAL_TAB));

        mList = new AvatarListBox(party);
        mScroll = new ScrollArea(mList);

        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
    }

    ~PartyTab()
    {
        delete mList;
        mList = 0;
        delete mScroll;
        mScroll = 0;
    }

    void action(const gcn::ActionEvent &event)
    {
        if (event.getId() == "do invite")
        {
            std::string name = mInviteDialog->getText();

            if (!name.empty())
                SERVER_NOTICE(strprintf(_("Invited user %s to party."),
                              name.c_str()))
            mInviteDialog = NULL;
        }
        else if (event.getId() == "~do invite")
        {
            mInviteDialog = NULL;
        }
        else if (event.getId() == "yes")
        {
            Net::getPartyHandler()->leave();
            SERVER_NOTICE(strprintf(_("Party %s quit requested."),
                                        mParty->getName().c_str()))
            mConfirmDialog = NULL;
        }
        else if (event.getId() == "no")
        {
            mConfirmDialog = NULL;
        }
    }

protected:
    void invite()
    {
        // TODO - Give feedback on whether the invite succeeded
        mInviteDialog = new TextDialog(_("Member Invite to Party"),
                      strprintf(_("Who would you like to invite to party %s?"),
                                mParty->getName().c_str()),
                      socialWindow, true);
        mInviteDialog->setActionEventId("do invite");
        mInviteDialog->addActionListener(this);
    }

    void leave()
    {
        mConfirmDialog = new ConfirmDialog(_("Leave Party?"),
                       strprintf(_("Are you sure you want to leave party %s?"),
                                 mParty->getName().c_str()),
                       socialWindow);

        mConfirmDialog->addActionListener(this);
    }

private:
    Party *mParty;
};

/*class BuddyTab : public SocialTab
{
    // TODO?
};*/

class CreatePopup : public Popup, public LinkHandler
{
public:
    CreatePopup():
            Popup("SocialCreatePopup")
    {
        mBrowserBox = new BrowserBox;
        mBrowserBox->setPosition(4, 4);
        mBrowserBox->setHighlightMode(BrowserBox::BACKGROUND);
        mBrowserBox->setOpaque(false);
        mBrowserBox->setLinkHandler(this);

        if (Net::getGuildHandler()->isSupported())
            mBrowserBox->addRow(strprintf("@@guild|%s@@", _("Create Guild")));
        mBrowserBox->addRow(strprintf("@@party|%s@@", _("Create Party")));
        mBrowserBox->addRow("##3---");
        mBrowserBox->addRow(strprintf("@@cancel|%s@@", _("Cancel")));

        add(mBrowserBox);

        setContentSize(mBrowserBox->getWidth() + 8,
                       mBrowserBox->getHeight() + 8);
    }

    void handleLink(const std::string &link)
    {
        if (link == "guild")
        {
            socialWindow->showGuildCreate();
        }
        else if (link == "party")
        {
            socialWindow->showPartyCreate();
        }

        setVisible(false);
    }

    void show(gcn::Widget *parent)
    {
        int x, y;
        parent->getAbsolutePosition(x, y);
        y += parent->getHeight();
        setPosition(x, y);
        setVisible(true);
        requestMoveToTop();
    }

private:
    BrowserBox* mBrowserBox;
};

SocialWindow::SocialWindow() :
    Window(_("Social")),
    mGuildInvited(0),
    mGuildAcceptDialog(NULL),
    mPartyAcceptDialog(NULL)
{
    setWindowName("Social");
    setVisible(false);
    setSaveVisible(true);
    setResizable(true);
    setSaveVisible(true);
    setCloseButton(true);
    setMinWidth(120);
    setMinHeight(55);
    setDefaultSize(590, 200, 150, 124);
    setupWindow->registerWindowForReset(this);

    loadWindowState();

    mCreateButton = new Button(_("Create"), "create", this);
    mInviteButton = new Button(_("Invite"), "invite", this);
    mLeaveButton = new Button(_("Leave"), "leave", this);
    mTabs = new TabbedArea;

    place(0, 0, mCreateButton);
    place(1, 0, mInviteButton);
    place(2, 0, mLeaveButton);
    place(0, 1, mTabs, 4, 4);

    widgetResized(NULL);

    mCreatePopup = new CreatePopup();

    if (local_player->getParty())
    {
        addTab(local_player->getParty());
    }
    else
        updateButtons();
}

SocialWindow::~SocialWindow()
{
    // Cleanup invites
    if (mGuildAcceptDialog)
    {
        mGuildAcceptDialog->close();
        mGuildAcceptDialog->scheduleDelete();
        mGuildAcceptDialog = NULL;

        mGuildInvited = 0;
    }

    if (mPartyAcceptDialog)
    {
        mPartyAcceptDialog->close();
        mPartyAcceptDialog->scheduleDelete();
        mPartyAcceptDialog = NULL;

        mPartyInviter = "";
    }
    delete mCreatePopup;
}

bool SocialWindow::addTab(Guild *guild)
{
    if (mGuilds.find(guild) != mGuilds.end())
        return false;

    GuildTab *tab = new GuildTab(guild);
    mGuilds[guild] = tab;

    mTabs->addTab(tab, tab->mScroll);

    updateButtons();

    return true;
}

bool SocialWindow::removeTab(Guild *guild)
{
    GuildMap::iterator it = mGuilds.find(guild);
    if (it == mGuilds.end())
        return false;

    mTabs->removeTab(it->second);
    delete it->second;
    mGuilds.erase(it);

    updateButtons();

    return true;
}

bool SocialWindow::addTab(Party *party)
{
    if (mParties.find(party) != mParties.end())
        return false;

    PartyTab *tab = new PartyTab(party);
    mParties[party] = tab;

    mTabs->addTab(tab, tab->mScroll);

    updateButtons();

    return true;
}

bool SocialWindow::removeTab(Party *party)
{
    PartyMap::iterator it = mParties.find(party);
    if (it == mParties.end())
        return false;

    mTabs->removeTab(it->second);
    delete it->second;
    mParties.erase(it);

    updateButtons();

    return true;
}

void SocialWindow::action(const gcn::ActionEvent &event)
{
    const std::string &eventId = event.getId();

    if (event.getSource() == mPartyAcceptDialog)
    {
        // check if they accepted the invite
        if (eventId == "yes")
        {
            SERVER_NOTICE(strprintf(_("Accepted party invite from %s."),
                    mPartyInviter.c_str()))
            Net::getPartyHandler()->inviteResponse(mPartyInviter, true);
        }
        else if (eventId == "no")
        {
            SERVER_NOTICE(strprintf(_("Rejected party invite from %s."),
                    mPartyInviter.c_str()))
            Net::getPartyHandler()->inviteResponse(mPartyInviter, false);
        }

        mPartyInviter = "";
        mPartyAcceptDialog = NULL;
    }
    else if (event.getSource() == mGuildAcceptDialog)
    {
        // check if they accepted the invite
        if (eventId == "yes")
        {
            SERVER_NOTICE(_("Accepted guild invite"))
            Net::getGuildHandler()->inviteResponse(mGuildInvited, true);
        }
        else if (eventId == "no")
        {
            SERVER_NOTICE(_("Rejected guild invite."))
            Net::getGuildHandler()->inviteResponse(mGuildInvited, false);
        }

        mGuildInvited = 0;
        mGuildAcceptDialog = NULL;
    }
    else if (event.getId() == "create")
    {
        if (Net::getGuildHandler()->isSupported())
            mCreatePopup->show(mCreateButton);
        else
            showPartyCreate();
    }
    else if (event.getId() == "invite" && mTabs->getSelectedTabIndex() > -1)
    {
        static_cast<SocialTab*>(mTabs->getSelectedTab())->invite();
    }
    else if (event.getId() == "leave" && mTabs->getSelectedTabIndex() > -1)
    {
        static_cast<SocialTab*>(mTabs->getSelectedTab())->leave();
    }
    else if (event.getId() == "create guild")
    {
        std::string name = mGuildCreateDialog->getText();

        if (name.size() > 16)
        {
            SERVER_NOTICE(_("Creating guild failed, please choose a "
                            "shorter name."));
            return;
        }
        else if (!name.empty())
        {
            Net::getGuildHandler()->create(name);
            SERVER_NOTICE(strprintf(_("Creating guild called %s."),
                                    name.c_str()));
        }

        mGuildCreateDialog = NULL;
    }
    else if (event.getId() == "~create guild")
    {
        mGuildCreateDialog = NULL;
    }
    else if (event.getId() == "create party")
    {
        std::string name = mPartyCreateDialog->getText();

        if (name.size() > 16)
        {
            SERVER_NOTICE(_("Creating party failed, please choose a "
                            "shorter name."));
            return;
        }
        else if (!name.empty())
        {
            Net::getPartyHandler()->create(name);
            SERVER_NOTICE(strprintf(_("Creating party called %s."),
                                    name.c_str()));
        }

        mPartyCreateDialog = NULL;
    }
    else if (event.getId() == "~create party")
    {
        mPartyCreateDialog = NULL;
    }
}

void SocialWindow::showGuildCreate()
{
    mGuildCreateDialog = new TextDialog(_("Guild Name"),
                                        _("Choose your guild's name."), this);
    mGuildCreateDialog->setActionEventId("create guild");
    mGuildCreateDialog->addActionListener(this);
}

void SocialWindow::showGuildInvite(const std::string &guildName,
                                   const int guildId,
                                   const std::string &inviterName)
{
    // check there isnt already an invite showing
    if (mGuildInvited != 0)
    {
        SERVER_NOTICE(_("Received guild request, but one already "
                "exists."))
        return;
    }

    std::string msg = strprintf(_("%s has invited you to join the guild %s."),
                                inviterName.c_str(), guildName.c_str());
    SERVER_NOTICE(msg)

    // show invite
    mGuildAcceptDialog = new ConfirmDialog(_("Accept Guild Invite"), msg, this);
    mGuildAcceptDialog->addActionListener(this);

    mGuildInvited = guildId;
}

void SocialWindow::showPartyInvite(const std::string &inviter,
                                   const std::string &partyName)
{
    // check there isnt already an invite showing
    if (mPartyInviter != "")
    {
        SERVER_NOTICE(_("Received party request, but one already exists."))
        return;
    }

    std::string msg;
    if (inviter.empty())
    {
        if (partyName.empty())
        {
            msg = _("You have been invited you to join a party.");
        }
        else
        {
            msg = strprintf(_("You have been invited to join the %s party."),
                                        partyName.c_str());
        }
    }
    else
    {
        if (partyName.empty())
        {
            msg = strprintf(_("%s has invited you to join their party."),
                                        inviter.c_str());
        }
        else
        {
            msg = strprintf(_("%s has invited you to join the %s party."),
                                        inviter.c_str(), partyName.c_str());
        }
    }

    SERVER_NOTICE(msg)

    // show invite
    mPartyAcceptDialog = new ConfirmDialog(_("Accept Party Invite"), msg, this);
    mPartyAcceptDialog->addActionListener(this);

    mPartyInviter = inviter;
}

void SocialWindow::showPartyCreate()
{
    if (local_player->getParty())
    {
        new OkDialog(_("Create Party"),
                     _("Cannot create party. You are already in a party"),
                     this);
        return;
    }

    mPartyCreateDialog = new TextDialog(_("Party Name"),
                                        _("Choose your party's name."), this);
    mPartyCreateDialog->setActionEventId("create party");
    mPartyCreateDialog->addActionListener(this);
}

void SocialWindow::updateButtons()
{
    bool hasTabs = mTabs->getNumberOfTabs() > 0;
    mInviteButton->setEnabled(hasTabs);
    mLeaveButton->setEnabled(hasTabs);
}
