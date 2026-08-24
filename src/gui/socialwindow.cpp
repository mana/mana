/*
 *  The Mana Client
 *  Copyright (C) 2010-2026  The Mana Developers
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

#include "client.h"
#include "event.h"
#include "guild.h"
#include "localplayer.h"
#include "party.h"
#include "playerrelations.h"

#include "gui/chatwindow.h"
#include "gui/confirmdialog.h"
#include "gui/popupmenu.h"
#include "gui/okdialog.h"
#include "gui/setup.h"
#include "gui/textdialog.h"

#include "gui/widgets/avatarlistbox.h"
#include "gui/widgets/button.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/menu.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/tabbedarea.h"

#include "net/net.h"
#include "net/chathandler.h"
#include "net/guildhandler.h"
#include "net/partyhandler.h"

#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <memory>

class SocialTab : public Tab
{
protected:
    friend class SocialWindow;

    SocialTab() = default;

    ~SocialTab() override
    {
        // Cleanup dialogs
        if (mInviteDialog)
        {
            mInviteDialog->close();
            mInviteDialog->scheduleDelete();
            mInviteDialog = nullptr;
        }

        if (mConfirmDialog)
        {
            mConfirmDialog->close();
            mConfirmDialog->scheduleDelete();
            mConfirmDialog = nullptr;
        }
    }

    virtual void invite() = 0;

    virtual void leave() = 0;

    /**
     * Shows the context menu for \a avatar at the given screen position.
     * Which actions apply depends on the player, not on the list they were
     * clicked in.
     */
    void showPopup(Avatar *avatar, int x, int y)
    {
        const std::string &name = avatar->getName();

        Menu *menu = socialWindow->getMenu();
        menu->clear();

        if (name != local_player->getName())
        {
            menu->addItem(strprintf(_("Whisper %s"), name.c_str()), [name] {
                chatWindow->addInputText("/w \"" + name + "\" ");
            });

            menu->addSeparator();

            addPlayerRelationItems(*menu, name);

            if (local_player->getNumberOfGuilds())
            {
                menu->addItem(strprintf(_("Invite %s to join your guild"),
                                        name.c_str()), [name] {
                    local_player->inviteToGuild(name);
                });
            }

            Party *party = local_player->getParty();
            const PartyMember *member = party ? party->getMember(name) : nullptr;

            if (!member && (local_player->isInParty() ||
                            Net::getNetworkType() == ServerType::ManaServ))
            {
                menu->addItem(strprintf(_("Invite %s to join your party"),
                                        name.c_str()), [name] {
                    Net::getPartyHandler()->invite(name);
                });
            }
            else if (member)
            {
                const PartyMember *self =
                        party->getMember(local_player->getName());

                if (self && self->getLeader())
                {
                    menu->addItem(strprintf(_("Kick %s from party"),
                                            name.c_str()), [name] {
                        Net::getPartyHandler()->kick(name);
                    });
                }
            }
        }

        menu->addSeparator();
        menu->addItem(_("Add name to chat"), [name] {
            chatWindow->addInputText(name);
        });
        menu->showAt(x, y);
    }

    /**
     * Creates the avatar list and the scroll area around it.
     */
    void setupList(AvatarListModel *model)
    {
        mList = std::make_unique<AvatarListBox>(model);
        mList->setContextMenuHandler([this] (Avatar *avatar, int x, int y) {
            showPopup(avatar, x, y);
        });

        mScroll = std::make_unique<ScrollArea>(mList.get());
        mScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
        mScroll->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_AUTO);
    }

    TextDialog *mInviteDialog = nullptr;
    ConfirmDialog *mConfirmDialog = nullptr;
    std::unique_ptr<ScrollArea> mScroll;
    std::unique_ptr<AvatarListBox> mList;
};

class GuildTab : public SocialTab, public gcn::ActionListener
{
public:
    GuildTab(Guild *guild):
        mGuild(guild)
    {
        setCaption(guild->getName());

        setTabColor(&Theme::getThemeColor(Theme::GUILD));

        setupList(guild);
    }

    void action(const gcn::ActionEvent &event) override
    {
        if (event.getId() == "do invite")
        {
            std::string name = mInviteDialog->getText();

            if (!name.empty())
            {
                Net::getGuildHandler()->invite(mGuild->getId(), name);
                serverNotice(strprintf(_("Invited user %s to guild %s."),
                                       name.c_str(),
                                       mGuild->getName().c_str()));
            }
            mInviteDialog = nullptr;
        }
        else if (event.getId() == "~do invite")
        {
            mInviteDialog = nullptr;
        }
        else if (event.getId() == "yes")
        {
            Net::getGuildHandler()->leave(mGuild->getId());
            serverNotice(strprintf(_("Guild %s quit requested."),
                                   mGuild->getName().c_str()));
            mConfirmDialog = nullptr;
        }
        else if (event.getId() == "no")
        {
            mConfirmDialog = nullptr;
        }
    }

protected:
    void invite() override
    {
        // TODO - Give feedback on whether the invite succeeded
        mInviteDialog = new TextDialog(_("Member Invite to Guild"),
                     strprintf(_("Who would you like to invite to guild %s?"),
                               mGuild->getName().c_str()),
                     socialWindow, true);
        mInviteDialog->setActionEventId("do invite");
        mInviteDialog->addActionListener(this);
    }

    void leave() override
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

        setTabColor(&Theme::getThemeColor(Theme::PARTY_TAB));

        setupList(party);
    }

    void action(const gcn::ActionEvent &event) override
    {
        if (event.getId() == "do invite")
        {
            std::string name = mInviteDialog->getText();

            if (!name.empty())
                serverNotice(strprintf(_("Invited user %s to party."),
                                       name.c_str()));
            mInviteDialog = nullptr;
        }
        else if (event.getId() == "~do invite")
        {
            mInviteDialog = nullptr;
        }
        else if (event.getId() == "yes")
        {
            Net::getPartyHandler()->leave();
            serverNotice(strprintf(_("Party %s quit requested."),
                                   mParty->getName().c_str()));
            mConfirmDialog = nullptr;
        }
        else if (event.getId() == "no")
        {
            mConfirmDialog = nullptr;
        }
    }

protected:
    void invite() override
    {
        // TODO - Give feedback on whether the invite succeeded
        mInviteDialog = new TextDialog(_("Member Invite to Party"),
                      strprintf(_("Who would you like to invite to party %s?"),
                                mParty->getName().c_str()),
                      socialWindow, true);
        mInviteDialog->setActionEventId("do invite");
        mInviteDialog->addActionListener(this);
    }

    void leave() override
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

class PlayerList : public AvatarListModel
{
public:
    ~PlayerList() override
    {
        delete_all(mPlayers);
    }

    void setPlayers(const std::vector<Avatar*> &players)
    {
        delete_all(mPlayers);
        mPlayers = players;
    }

    /**
     * Returns the number of players in the list.
     */
    int getNumberOfElements() override
    {
        return mPlayers.size();
    }

    Avatar *getAvatarAt(int index) override
    {
        return mPlayers[index];
    }

private:
    std::vector<Avatar*> mPlayers;
};

class PlayerListTab : public SocialTab
{
public:
    PlayerListTab()
    {
        mPlayerList = new PlayerList;

        setupList(mPlayerList);
    }

    ~PlayerListTab() override
    {
        delete mPlayerList;
    }

    void setPlayers(const std::vector<Avatar*> &players)
    {
        mPlayerList->setPlayers(players);
    }

protected:
    void invite() override {}
    void leave() override {}

private:
    PlayerList *mPlayerList;
};

/*class BuddyTab : public SocialTab
{
    // TODO?
};*/

SocialWindow::SocialWindow() :
    Window(_("Social"))
{
    setWindowName("Social");
    setVisible(false);
    setSaveVisible(true);
    setResizable(true);
    setSaveVisible(true);
    setCloseButton(true);
    setupWindow->registerWindowForReset(this);

    mCreateButton = new Button(_("Create"), "create", this);
    mInviteButton = new Button(_("Invite"), "invite", this);
    mLeaveButton = new Button(_("Leave"), "leave", this);
    mTabs = new TabbedArea;

    place(0, 0, mCreateButton);
    place(1, 0, mInviteButton);
    place(2, 0, mLeaveButton);
    place(0, 1, mTabs, 4, 4);

    // Determine minimum size
    int width = 0, height = 0;
    getLayout().reflow(width, height);
    setMinimumContentSize(width, height);

    setDefaultSize(590, 200, 150, 124);
    loadWindowState();

    mMenu = new Menu("SocialMenu");

    mPlayerListTab = new PlayerListTab;
    mPlayerListTab->setCaption(strprintf(_("Online (%u)"), 0u));

    mTabs->addTab(mPlayerListTab, mPlayerListTab->mScroll.get());

    if (local_player->getParty())
        addTab(local_player->getParty());
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
        mGuildAcceptDialog = nullptr;

        mGuildInvited = 0;
    }

    if (mPartyAcceptDialog)
    {
        mPartyAcceptDialog->close();
        mPartyAcceptDialog->scheduleDelete();
        mPartyAcceptDialog = nullptr;

        mPartyInviter.clear();
    }
    delete mMenu;
    delete mPlayerListTab;
}

bool SocialWindow::addTab(Guild *guild)
{
    if (mGuilds.find(guild) != mGuilds.end())
        return false;

    auto *tab = new GuildTab(guild);
    mGuilds[guild] = tab;

    mTabs->addTab(tab, tab->mScroll.get());

    updateButtons();

    return true;
}

bool SocialWindow::removeTab(Guild *guild)
{
    auto it = mGuilds.find(guild);
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

    auto *tab = new PartyTab(party);
    mParties[party] = tab;

    mTabs->addTab(tab, tab->mScroll.get());

    updateButtons();

    return true;
}

bool SocialWindow::removeTab(Party *party)
{
    auto it = mParties.find(party);
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
            serverNotice(strprintf(_("Accepted party invite from %s."),
                                   mPartyInviter.c_str()));
            Net::getPartyHandler()->inviteResponse(mPartyInviter, true);
        }
        else if (eventId == "no")
        {
            serverNotice(strprintf(_("Rejected party invite from %s."),
                                   mPartyInviter.c_str()));
            Net::getPartyHandler()->inviteResponse(mPartyInviter, false);
        }

        mPartyInviter.clear();
        mPartyAcceptDialog = nullptr;
    }
    else if (event.getSource() == mGuildAcceptDialog)
    {
        // check if they accepted the invite
        if (eventId == "yes")
        {
            serverNotice(_("Accepted guild invite"));
            Net::getGuildHandler()->inviteResponse(mGuildInvited, true);
        }
        else if (eventId == "no")
        {
            serverNotice(_("Rejected guild invite."));
            Net::getGuildHandler()->inviteResponse(mGuildInvited, false);
        }

        mGuildInvited = 0;
        mGuildAcceptDialog = nullptr;
    }
    else if (event.getId() == "create")
    {
        if (Net::getGuildHandler()->isSupported())
        {
            mMenu->clear();
            mMenu->addItem(_("Create Guild"), [this] { showGuildCreate(); });
            mMenu->addItem(_("Create Party"), [this] { showPartyCreate(); });
            mMenu->showBelow(mCreateButton);
        }
        else
        {
            showPartyCreate();
        }
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
            serverNotice(_("Creating guild failed, please choose a "
                           "shorter name."));
            return;
        }

        if (!name.empty())
        {
            Net::getGuildHandler()->create(name);
            serverNotice(strprintf(_("Creating guild called %s."),
                                   name.c_str()));
        }

        mGuildCreateDialog = nullptr;
    }
    else if (event.getId() == "~create guild")
    {
        mGuildCreateDialog = nullptr;
    }
    else if (event.getId() == "create party")
    {
        std::string name = mPartyCreateDialog->getText();

        if (name.size() > 16)
        {
            serverNotice(_("Creating party failed, please choose a "
                           "shorter name."));
            return;
        }

        if (!name.empty())
        {
            Net::getPartyHandler()->create(name);
            serverNotice(strprintf(_("Creating party called %s."),
                                   name.c_str()));
        }

        mPartyCreateDialog = nullptr;
    }
    else if (event.getId() == "~create party")
    {
        mPartyCreateDialog = nullptr;
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
        serverNotice(_("Received guild request, but one already exists."));
        return;
    }

    std::string msg = strprintf(_("%s has invited you to join the guild %s."),
                                inviterName.c_str(), guildName.c_str());
    serverNotice(msg);

    // show invite
    mGuildAcceptDialog = new ConfirmDialog(_("Accept Guild Invite"), msg, this);
    mGuildAcceptDialog->addActionListener(this);

    mGuildInvited = guildId;
}

void SocialWindow::showPartyInvite(const std::string &inviter,
                                   const std::string &partyName)
{
    // check there isnt already an invite showing
    if (!mPartyInviter.empty())
    {
        serverNotice(_("Received party request, but one already exists."));
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

    serverNotice(msg);

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
                     _("Cannot create party. You are already in a party."),
                     true, this);
        return;
    }

    mPartyCreateDialog = new TextDialog(_("Party Name"),
                                        _("Choose your party's name."), this);
    mPartyCreateDialog->setActionEventId("create party");
    mPartyCreateDialog->addActionListener(this);
}

void SocialWindow::setPlayersOnline(const std::vector<Avatar*> &players)
{
    mPlayerListTab->setPlayers(players);

    unsigned playerCount = static_cast<unsigned>(players.size());
    mPlayerListTab->setCaption(strprintf(_("Online (%u)"), playerCount));
}

void SocialWindow::logic()
{
    if (mOnlineListUpdateTimer.passed())
    {
        Net::getChatHandler()->requestOnlineList();
        mOnlineListUpdateTimer.set(18000);
    }

    Window::logic();
}

void SocialWindow::updateButtons()
{
    bool hasTabs = mTabs->getNumberOfTabs() > 0;
    mInviteButton->setEnabled(hasTabs);
    mLeaveButton->setEnabled(hasTabs);
}
