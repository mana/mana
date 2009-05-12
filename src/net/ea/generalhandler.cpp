/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "gui/inventorywindow.h"

#include "net/ea/generalhandler.h"

#include "net/ea/network.h"
#include "net/ea/protocol.h"

#include "net/ea/adminhandler.h"
#include "net/ea/beinghandler.h"
#include "net/ea/buysellhandler.h"
#include "net/ea/chathandler.h"
#include "net/ea/charserverhandler.h"
#include "net/ea/equipmenthandler.h"
#include "net/ea/inventoryhandler.h"
#include "net/ea/itemhandler.h"
#include "net/ea/loginhandler.h"
#include "net/ea/logouthandler.h"
#include "net/ea/maphandler.h"
#include "net/ea/npchandler.h"
#include "net/ea/playerhandler.h"
#include "net/ea/partyhandler.h"
#include "net/ea/tradehandler.h"
#include "net/ea/skillhandler.h"

#include "net/ea/gui/partytab.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "resources/itemdb.h"

#include "configuration.h"
#include "log.h"
#include "main.h"

#include "utils/gettext.h"

#include <list>

Net::GeneralHandler *generalHandler;

namespace EAthena {

GeneralHandler::GeneralHandler():
    mAdminHandler(new AdminHandler),
    mBeingHandler(new BeingHandler(config.getValue("EnableSync", 0) == 1)),
    mBuySellHandler(new BuySellHandler),
    mCharHandler(new CharServerHandler),
    mChatHandler(new ChatHandler),
    mEquipmentHandler(new EquipmentHandler),
    mInventoryHandler(new InventoryHandler),
    mItemHandler(new ItemHandler),
    mLoginHandler(new LoginHandler),
    mLogoutHandler(new LogoutHandler),
    mMapHandler(new MapHandler),
    mNpcHandler(new NpcHandler),
    mPartyHandler(new PartyHandler),
    mPlayerHandler(new PlayerHandler),
    mSkillHandler(new SkillHandler),
    mTradeHandler(new TradeHandler)
{
    static const Uint16 _messages[] = {
        SMSG_CONNECTION_PROBLEM,
        0
    };
    handledMessages = _messages;
    generalHandler = this;

    std::list<ItemDB::Stat> stats;
    stats.push_back(ItemDB::Stat("str", N_("Strength %+d")));
    stats.push_back(ItemDB::Stat("agi", N_("Agility %+d")));
    stats.push_back(ItemDB::Stat("vit", N_("Vitality %+d")));
    stats.push_back(ItemDB::Stat("int", N_("Intelligence %+d")));
    stats.push_back(ItemDB::Stat("dex", N_("Dexterity %+d")));
    stats.push_back(ItemDB::Stat("luck", N_("Luck %+d")));

    ItemDB::setStatsList(stats);
}

GeneralHandler::~GeneralHandler()
{
    delete mNetwork;
}

void GeneralHandler::handleMessage(MessageIn &msg)
{
    int code;

    switch (msg.getId())
    {
        case SMSG_CONNECTION_PROBLEM:
            code = msg.readInt8();
            logger->log("Connection problem: %i", code);

            switch (code) {
                case 0:
                    errorMessage = _("Authentication failed");
                    break;
                case 1:
                    errorMessage = _("No servers available");
                    break;
                case 2:
                    if (state == STATE_GAME)
                        errorMessage = _("Someone else is trying to use this "
                                         "account");
                    else
                        errorMessage = _("This account is already logged in");
                    break;
                case 3:
                    errorMessage = _("Speed hack detected");
                    break;
                case 8:
                    errorMessage = _("Duplicated login");
                    break;
                default:
                    errorMessage = _("Unknown connection error");
                    break;
            }
            state = STATE_ERROR;
            break;
    }
}

void GeneralHandler::load()
{
    mNetwork->registerHandler(mAdminHandler.get());
    mNetwork->registerHandler(mBeingHandler.get());
    mNetwork->registerHandler(mBuySellHandler.get());
    mNetwork->registerHandler(mChatHandler.get());
    mNetwork->registerHandler(mCharHandler.get());
    mNetwork->registerHandler(mEquipmentHandler.get());
    mNetwork->registerHandler(mInventoryHandler.get());
    mNetwork->registerHandler(mItemHandler.get());
    mNetwork->registerHandler(mLoginHandler.get());
    mNetwork->registerHandler(mLogoutHandler.get());
    mNetwork->registerHandler(mMapHandler.get());
    mNetwork->registerHandler(mNpcHandler.get());
    mNetwork->registerHandler(mPlayerHandler.get());
    mNetwork->registerHandler(mSkillHandler.get());
    mNetwork->registerHandler(mTradeHandler.get());
    mNetwork->registerHandler(mPartyHandler.get());
}

void GeneralHandler::unload()
{
    mNetwork->clearHandlers();
}

void GeneralHandler::flushNetwork()
{
    if (!mNetwork)
        return;

    mNetwork->flush();
    mNetwork->dispatchMessages();
}

bool GeneralHandler::isNetworkConnected()
{
    return mNetwork->isConnected();
}

void GeneralHandler::tick()
{
    if (!mNetwork)
        return;

    if (mNetwork->getState() == Network::NET_ERROR)
    {
        if (!mNetwork->getError().empty())
            errorMessage = mNetwork->getError();
        else
            errorMessage = _("Got disconnected from server!");

        state = STATE_ERROR;
    }
}

void GeneralHandler::guiWindowsLoaded()
{
    partyTab = new PartyTab;
    inventoryWindow->setSplitAllowed(false);
}

void GeneralHandler::guiWindowsUnloaded()
{
    delete partyTab;
}

} // namespace EAthena
