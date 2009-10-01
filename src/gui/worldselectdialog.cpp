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

#include "gui/widgets/button.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"

#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/net.h"
#include "net/worldinfo.h"

#include "main.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

extern WorldInfo **server_info;

/**
 * The list model for the server list.
 */
class ServerListModel : public gcn::ListModel
{
    public:
        ServerListModel(Worlds worlds):
                mWorlds(worlds)
        {
        }

        virtual ~ServerListModel() {}

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
    mServerListModel = new ServerListModel(worlds);
    mServerList = new ListBox(mServerListModel);
    ScrollArea *mScrollArea = new ScrollArea(mServerList);
    mOkButton = new Button(_("OK"), "ok", this);
    Button *mCancelButton = new Button(_("Cancel"), "cancel", this);

    setContentSize(200, 100);

    mCancelButton->setPosition(
            200 - mCancelButton->getWidth() - 5,
            100 - mCancelButton->getHeight() - 5);
    mOkButton->setPosition(
            mCancelButton->getX() - mOkButton->getWidth() - 5,
            100 - mOkButton->getHeight() - 5);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setDimension(gcn::Rectangle(
                5, 5, 200 - 2 * 5,
                100 - 3 * 5 - mCancelButton->getHeight() -
                mScrollArea->getFrameSize()));

    mServerList->setActionEventId("ok");

    //mServerList->addActionListener(this);

    add(mScrollArea);
    add(mOkButton);
    add(mCancelButton);

    if (worlds.size() == 0)
        // Disable Ok button
        mOkButton->setEnabled(false);
    else
        // Select first server
        mServerList->setSelected(0);

    center();
    setVisible(true);
    mOkButton->requestFocus();
}

WorldSelectDialog::~WorldSelectDialog()
{
    delete mServerListModel;
}

void WorldSelectDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        mOkButton->setEnabled(false);
        Net::getLoginHandler()->chooseServer(mServerList->getSelected());
        state = STATE_UPDATE;
    }
    else if (event.getId() == "cancel")
        state = STATE_LOGIN;
}
