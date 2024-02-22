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

#ifndef SOCIALWINDOW_H
#define SOCIALWINDOW_H

#include "gui/widgets/window.h"

#include <guichan/actionevent.hpp>
#include <guichan/actionlistener.hpp>

#include <string>
#include <map>

class Button;
class ConfirmDialog;
class CreatePopup;
class Guild;
class Party;
class SocialTab;
class Tab;
class TabbedArea;
class TextDialog;

/**
 * Party window.
 *
 * \ingroup Interface
 */
class SocialWindow : public Window, gcn::ActionListener
{
public:
    SocialWindow();

    ~SocialWindow() override;

    bool addTab(Guild *guild);

    bool removeTab(Guild *guild);

    bool addTab(Party *party);

    bool removeTab(Party *party);

    /**
     * Handle events.
     */
    void action(const gcn::ActionEvent &event) override;

    void showGuildInvite(const std::string &guildName, const int guildId,
                         const std::string &inviterName);

    void showGuildCreate();

    void showPartyInvite(const std::string &inviter,
                         const std::string &partyName = std::string());

    void showPartyCreate();

protected:
    friend class SocialTab;

    void updateButtons();

    int mGuildInvited;
    ConfirmDialog *mGuildAcceptDialog;
    TextDialog *mGuildCreateDialog;

    std::string mPartyInviter;
    ConfirmDialog *mPartyAcceptDialog;
    TextDialog *mPartyCreateDialog;

    using GuildMap = std::map<Guild *, SocialTab *>;
    GuildMap mGuilds;

    using PartyMap = std::map<Party *, SocialTab *>;
    PartyMap mParties;

    CreatePopup *mCreatePopup;

    Button *mCreateButton;
    Button *mInviteButton;
    Button *mLeaveButton;
    TabbedArea *mTabs;
};

extern SocialWindow *socialWindow;

#endif // SOCIALWINDOW_H
