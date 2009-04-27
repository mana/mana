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
 *  $$
 */

#include "gui/guildwindow.h"

#include "gui/confirmdialog.h"
#include "gui/guildlistbox.h"
#include "gui/textdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/chattab.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/tabbedarea.h"
#include "gui/widgets/windowcontainer.h"

#include "guild.h"
#include "log.h"
#include "localplayer.h"

#include "net/tmwserv/chatserver/guild.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

#include <algorithm>

#include <guichan/widgets/tab.hpp>

GuildWindow::GuildWindow():
    Window(_("Guild")),
    mFocus(false)
{
    setWindowName("Guild");
    setCaption(_("Guild"));
    setResizable(false);
    setCloseButton(true);
    setSaveVisible(true);
    setMinWidth(200);
    setMinHeight(280);
    setDefaultSize(124, 41, 288, 330);

    // Set button events Id
    mGuildButton[0] = new Button(_("Create Guild"), "CREATE_GUILD", this);
    mGuildButton[1] = new Button(_("Invite User"), "INVITE_USER", this);
    mGuildButton[2] = new Button(_("Quit Guild"), "QUIT_GUILD", this);
    mGuildButton[1]->setEnabled(false);
    mGuildButton[2]->setEnabled(false);

    mGuildTabs = new TabbedArea;

    place(0, 0, mGuildButton[0]);
    place(1, 0, mGuildButton[1]);
    place(2, 0, mGuildButton[2]);
    place(0, 1, mGuildTabs);
    Layout &layout = getLayout();
    layout.setColWidth(0, 48);
    layout.setColWidth(1, 65);

    loadWindowState();
}

GuildWindow::~GuildWindow()
{
}

void GuildWindow::update()
{
    updateTab();

    if (mGuildButton[2]->isEnabled() && mGuildTabs->getNumberOfTabs() <= 0)
    {
        mGuildButton[2]->setEnabled(false);
        mGuildButton[1]->setEnabled(false);
    }
}

void GuildWindow::draw(gcn::Graphics *g)
{
    update();

    Window::draw(g);
}

void GuildWindow::action(const gcn::ActionEvent &event)
{
    const std::string &eventId = event.getId();

    // Stats Part
    if (eventId == "CREATE_GUILD")
    {
        // Set focus so that guild name to be created can be typed.
        mFocus = true;
        guildDialog = new TextDialog("Guild Name", "Choose your guild's name", this);
        guildDialog->setOKButtonActionId("CREATE_GUILD_OK");
        guildDialog->addActionListener(this);
    }
    else if (eventId == "INVITE_USER")
    {
        // TODO - Give feedback on whether the invite succeeded
        mFocus = true;
        inviteDialog = new TextDialog("Member Invite", "Who would you like to invite?", this);
        inviteDialog->setOKButtonActionId("INVITE_USER_OK");
        inviteDialog->addActionListener(this);
    }
    else if (eventId == "QUIT_GUILD")
    {
        short guild = getSelectedGuild();
        if (guild)
        {
            Net::ChatServer::Guild::quitGuild(guild);
            localChatTab->chatLog("Guild " + mGuildTabs->getSelectedTab()->getCaption() + " quit", BY_SERVER);
        }
    }
    else if (eventId == "CREATE_GUILD_OK")
    {
        std::string name = guildDialog->getText();
        if(name.size() > 16)
        {
            // TODO : State too many characters in input.
            return;
        }
        // Process guild name to be created, and unfocus.
        Net::ChatServer::Guild::createGuild(name);

        // Defocus dialog
        mFocus = false;
        localChatTab->chatLog("Creating Guild called " + name, BY_SERVER);
        guildDialog->scheduleDelete();
    }
    else if (eventId == "INVITE_USER_OK")
    {
        std::string name = inviteDialog->getText();
        short selectedGuild = getSelectedGuild();

        // Process invited user to be created and unfocus.
        Net::ChatServer::Guild::invitePlayer(name, selectedGuild);

        // Defocus dialog
        mFocus = false;
        localChatTab->chatLog("Invited user " + name, BY_SERVER);
        inviteDialog->scheduleDelete();
    }
    else if (eventId == "yes")
    {
        logger->log("Sending invitation acceptance.");
        Net::ChatServer::Guild::acceptInvite(invitedGuild);
    }
}

void GuildWindow::newGuildTab(const std::string &guildName)
{
    // Create new tab
    GuildListBox *list = new GuildListBox;
    list->setListModel(player_node->getGuild(guildName));
    ScrollArea *sa = new ScrollArea(list);
    sa->setDimension(gcn::Rectangle(5, 5, 135, 250));

    // Add the listbox to the map
    mGuildLists.insert(std::pair<std::string, GuildListBox*>(guildName, list));

    mGuildTabs->addTab(guildName, sa);
    mGuildTabs->setDimension(gcn::Rectangle(28,35,140,250));

    updateTab();
}

void GuildWindow::updateTab()
{
    gcn::Tab *tab = mGuildTabs->getSelectedTab();
    if (tab)
    {
        setTab(tab->getCaption());
    }
    mGuildTabs->logic();
}

void GuildWindow::setTab(const std::string &guildName)
{
    // Only enable invite button if user has rights
    if(player_node->checkInviteRights(guildName))
    {
        mGuildButton[1]->setEnabled(true);
    }
    else
    {
        mGuildButton[1]->setEnabled(false);
    }

    mGuildButton[2]->setEnabled(true);
}

bool GuildWindow::isWindowFocused()
{
    return mFocus;
}

short GuildWindow::getSelectedGuild()
{
    if (mGuildTabs->getNumberOfTabs() > 0)
    {

        Guild *guild = player_node->getGuild(mGuildTabs->getSelectedTab()->getCaption());

        if (guild)
        {
            return guild->getId();
        }
    }

    return 0;
}

void GuildWindow::openAcceptDialog(const std::string &inviterName,
                                   const std::string &guildName)
{
    std::string msg = inviterName + " has invited you to join the guild " + guildName;
    localChatTab->chatLog(msg, BY_SERVER);

    acceptDialog = new ConfirmDialog("Accept Guild Invite", msg, this);
    acceptDialog->addActionListener(this);

    invitedGuild = guildName;
}

void GuildWindow::requestMemberList(short guildId)
{
    // Get the list of members for displaying in the guild window.
    Net::ChatServer::Guild::getGuildMembers(guildId);
}

void GuildWindow::removeTab(int guildId)
{
    Guild* guild = player_node->getGuild(guildId);
    if (guild)
    {
        Tab *tab = mGuildTabs->getTab(guild->getName());
        if (tab)
        {
            mGuildTabs->removeTab(tab);
        }
        updateTab();
    }
    mGuildTabs->logic();
}

void GuildWindow::setOnline(const std::string &guildName, const std::string &member,
                            bool online)
{
    GuildListMap::iterator itr = mGuildLists.find(guildName);
    if (itr != mGuildLists.end())
    {
        itr->second->setOnlineStatus(member, online);
    }
}
