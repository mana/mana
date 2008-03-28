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
 *  $$
 */

#include "guildwindow.h"

#include "button.h"
#include "chat.h"
#include "confirm_dialog.h"
#include "gccontainer.h"
#include "guildlistbox.h"
#include "scrollarea.h"
#include "tabbedcontainer.h"
#include "textdialog.h"
#include "windowcontainer.h"

#include "widgets/layout.h"

#include "../guild.h"
#include "../log.h"
#include "../localplayer.h"

#include "../net/chatserver/guild.h"
#include "../utils/dtor.h"
#include "../utils/gettext.h"

#include <algorithm>

GuildWindow::GuildWindow(LocalPlayer *player):
    Window(player->getName()),
    mPlayer(player),
    mFocus(false)
{
    setCaption("Guild");
    setResizable(true);
    setCloseButton(true);
    setMinWidth(200);
    setMinHeight(280);
    setDefaultSize(124, 41, 288, 330);

    // Set button events Id
    mGuildButton[0] = new Button(_("Create Guild"), "CREATE_GUILD", this);
    mGuildButton[1] = new Button(_("Invite User"), "INVITE_USER", this);
    mGuildButton[2] = new Button(_("Quit Guild"), "QUIT_GUILD", this);
    mGuildButton[1]->setEnabled(false);
    mGuildButton[2]->setEnabled(false);

    mGuildsContainer  = new TabbedContainer();

    mGuildsContainer->setOpaque(false);

    place(0, 0, mGuildButton[0]);
    place(1, 0, mGuildButton[1]);
    place(2, 0, mGuildButton[2]);
    place(0, 1, mGuildsContainer);
    Layout &layout = getLayout();
    layout.setColWidth(0, 48);
    layout.setColWidth(1, 65);

    loadWindowState(player->getName());
}

GuildWindow::~GuildWindow()
{
    for_each(mTabs.begin(), mTabs.end(), make_dtor(mTabs));
}

void GuildWindow::update()
{

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
        if (mGuildsContainer->getNumberOfTabs() > 1)
        {
            // This is just to limit the number of guild tabs that are created
            // TODO: Either limit this server side, or fix the interface issue
            chatWindow->chatLog("Current maximum number of guilds ownable is 2", BY_SERVER);
            return;
        }
        // Set focus so that guild name to be created can be typed.
        mFocus = true;
        guildDialog = new TextDialog("Guild Name", "Choose your guild's name", this);
        guildDialog->setOKButtonActionId("CREATE_GUILD_OK");
        guildDialog->addActionListener(this);
    }
    else if (eventId == "INVITE_USER")
    {
        // TODO - Process Invite User button clicked
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
            chatWindow->chatLog("Guild " + mGuildsContainer->getActiveWidget() + " quit", BY_SERVER);
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
        chatWindow->chatLog("Creating Guild called " + name, BY_SERVER);
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
        chatWindow->chatLog("Invited user " + name, BY_SERVER);
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
    GCContainer *tab = new GCContainer();
    tab->setWidth(getWidth() - 2 * tab->getBorderSize());
    tab->setHeight(getHeight() - 2 * tab->getBorderSize());
    tab->setOpaque(false);
    ListBox *list = new ListBox();
    list->setListModel(mPlayer->getGuild(guildName));
    ScrollArea *sa = new ScrollArea(list);
    sa->setDimension(gcn::Rectangle(5, 5, 135, 250));
    tab->add(sa);

    mGuildsContainer->addTab(tab, guildName);
    mGuildsContainer->setDimension(gcn::Rectangle(28,35,280,250));

    mTabs.push_back(tab);

    updateTab();
}

void GuildWindow::updateTab()
{
    setTab(mGuildsContainer->getActiveWidget());
}

void GuildWindow::setTab(const std::string &guildName)
{
    // Only enable invite button if user has rights
    if(mPlayer->checkInviteRights(guildName))
    {
        mGuildButton[1]->setEnabled(true);
    }
    else
    {
        mGuildButton[1]->setEnabled(false);
    }

    mGuildButton[2]->setEnabled(true);
}

bool GuildWindow::isFocused()
{
    return mFocus;
}

short GuildWindow::getSelectedGuild()
{
    Guild *guild = mPlayer->getGuild(mGuildsContainer->getActiveWidget());
    if (guild)
    {
        return guild->getId();
    }

    return 0;
}

void GuildWindow::openAcceptDialog(const std::string &inviterName, const std::string &guildName)
{
    std::string msg = inviterName + " has invited you to join the guild " + guildName;
    chatWindow->chatLog(msg, BY_SERVER);

    acceptDialog = new ConfirmDialog("Accept Guild Invite", msg, this);
    acceptDialog->addActionListener(this);

    invitedGuild = guildName;
}

void GuildWindow::requestMemberList(short guildId)
{
    // Get the list of members for displaying in the guild window.
    Net::ChatServer::Guild::getGuildMembers(guildId);
}

void GuildWindow::removeTab()
{
    mGuildsContainer->removeTab(mGuildsContainer->getActiveWidget());
    mGuildsContainer->logic();
}
