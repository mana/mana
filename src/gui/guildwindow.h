/*
 *  guildwindow.h
 *  A file part of The Mana World
 *
 *  Created by David Athay on 06/03/2007.
 *  
 * Copyright (c) 2007, The Mana World Development Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * My name may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 * $Id$
 */

#ifndef _TMW_GUI_GUILDWINDOW_H
#define _TMW_GUI_GUILDWINDOW_H

#include <iosfwd>
#include <vector>

#include <guichan/actionlistener.hpp>
#include <guichan/widgets/listbox.hpp>

#include "window.h"

#include "../guichanfwd.h"

class LocalPlayer;
class TextDialog;
class ConfirmDialog;
class GuildListBox;
class TabbedContainer;
class ScrollArea;
class GCContainer;

class GuildWindow : public Window, public gcn::ActionListener
{
public:
    /**
     * Constructor.
     */
    GuildWindow(LocalPlayer *player);
    
    /**
     * Destructor.
     */
    ~GuildWindow();

    /**
     * Called when receiving actions from widget.
     */
    void action(const gcn::ActionEvent &event);

    /**
     * Draw this window.
     */
    void draw(gcn::Graphics *graphics);

    /**
     * Updates this dialog.
     */
    void update();
    
    /**
     * Create a new tab for a guild list.
     */
    void newGuildTab(const std::string &guildName);
    
    /**
     * Display guild's member list to active tab
     */
    void setTab(const std::string &guildName);
    
    /**
     * Update the contents of the active tab
     */
    void updateTab();
    
    /**
     * Check if the window is in focus
     */
    bool isFocused();
    
    /**
     * Create a dialog for accepting an invite
     */
    void openAcceptDialog(const std::string &inviterName, const std::string &guildName);
    
    /**
     * Request member list
     */
    void requestMemberList(short guildId);
    
protected:
    /**
     * Get selected guild tab
     * @return Returns selected guild
     */
    short getSelectedGuild();
    
private:
    LocalPlayer *mPlayer;
    gcn::Button *mGuildButton[2];
    TextDialog *guildDialog;
    TextDialog *inviteDialog;
    ConfirmDialog *acceptDialog;
    TabbedContainer *mGuildsContainer;
    GuildListBox *mGuildMembersList;
    ScrollArea *mScrollArea;
    std::vector<GCContainer*> mTabs;
    bool mFocus;
    std::string invitedGuild;
};

extern GuildWindow *guildWindow;

#endif
