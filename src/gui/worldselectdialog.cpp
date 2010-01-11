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

#include "gui/worldselectdialog.h"

#include "main.h"

#include "gui/sdlinput.h"

#include "gui/widgets/button.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"

#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/net.h"
#include "net/worldinfo.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

extern WorldInfo **server_info;

/**
 * The list model for the server list.
 */
class WorldListModel : public gcn::ListModel
{
    public:
        WorldListModel(Worlds worlds):
                mWorlds(worlds)
        {
        }

        virtual ~WorldListModel() {}

        int getNumberOfElements()
        {
            return mWorlds.size();
        }

        std::string getElementAt(int i)
        {
            const WorldInfo *si = mWorlds[i];
            return si->name + " (" + toString(si->online_users) + ")";
        }
    private:
        Worlds mWorlds;
};

WorldSelectDialog::WorldSelectDialog(Worlds worlds):
    Window(_("Select World"))
{
    mWorldListModel = new WorldListModel(worlds);
    mWorldList = new ListBox(mWorldListModel);
    ScrollArea *worldsScroll = new ScrollArea(mWorldList);
    mChangeLoginButton = new Button(_("Change Login"), "login", this);
    mChooseWorld = new Button(_("Choose World"), "world", this);

    worldsScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    place(0, 0, worldsScroll, 3, 5).setPadding(2);
    place(1, 5, mChangeLoginButton);
    place(2, 5, mChooseWorld);

    // Make sure the list has enough height
    getLayout().setRowHeight(0, 60);

    reflowLayout(0, 0);

    if (worlds.size() == 0)
        // Disable Ok button
        mChooseWorld->setEnabled(false);
    else
        // Select first server
        mWorldList->setSelected(0);

    addKeyListener(this);

    center();
    setVisible(true);
    mChooseWorld->requestFocus();
}

WorldSelectDialog::~WorldSelectDialog()
{
    delete mWorldListModel;
}

void WorldSelectDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "world")
    {
        mChangeLoginButton->setEnabled(false);
        mChooseWorld->setEnabled(false);
        Net::getLoginHandler()->chooseServer(mWorldList->getSelected());

        // Check in case netcode moves us forward
        if (state == STATE_WORLD_SELECT)
            state = STATE_WORLD_SELECT_ATTEMPT;
    }
    else if (event.getId() == "login")
    {
        state = STATE_LOGIN;
    }
}

void WorldSelectDialog::keyPressed(gcn::KeyEvent &keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ESCAPE)
    {
        action(gcn::ActionEvent(NULL, mChangeLoginButton->getActionEventId()));
    }
    else if (key.getValue() == Key::ENTER)
    {
        action(gcn::ActionEvent(NULL, mChooseWorld->getActionEventId()));
    }
}
