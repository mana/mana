/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "buyselldialog.h"

#include "playerinfo.h"

#include "gui/widgets/button.h"

#include "net/net.h"
#include "net/npchandler.h"

#include "utils/gettext.h"

BuySellDialog::DialogList BuySellDialog::instances;

BuySellDialog::BuySellDialog(int npcId):
    Window(_("Shop")),
    mNpcId(npcId),
    mBuyButton(nullptr)
{
    setWindowName("BuySell");
    //setupWindow->registerWindowForReset(this);
    setCloseButton(true);

    static const char *buttonNames[] = {
        N_("Buy"), N_("Sell"), N_("Cancel"), nullptr
    };
    int x = 10, y = 10;

    for (const char **curBtn = buttonNames; *curBtn; curBtn++)
    {
        auto *btn = new Button(gettext(*curBtn), *curBtn, this);
        if (!mBuyButton)
            mBuyButton = btn; // For focus request
        btn->setPosition(x, y);
        add(btn);
        x += btn->getWidth() + 10;
    }
    mBuyButton->requestFocus();

    setContentSize(x, 2 * y + mBuyButton->getHeight());

    center();
    setDefaultSize();
    loadWindowState();

    instances.push_back(this);
    setVisible(true);

    PlayerInfo::setBuySellState(BUYSELL_CHOOSING);
}

BuySellDialog::~BuySellDialog()
{
    instances.remove(this);

    if (PlayerInfo::getBuySellState() == BUYSELL_CHOOSING)
        PlayerInfo::setBuySellState(BUYSELL_NONE);
}

void BuySellDialog::setVisible(bool visible)
{
    Window::setVisible(visible);

    if (visible)
    {
        mBuyButton->requestFocus();
    }
    else
    {
        scheduleDelete();
    }
}

void BuySellDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "Buy")
    {
        Net::getNpcHandler()->buy(mNpcId);
    }
    else if (event.getId() == "Sell")
    {
        Net::getNpcHandler()->sell(mNpcId);
    }

    close();
}

void BuySellDialog::closeAll()
{
    for (auto &instance : instances)
        instance->close();
}
