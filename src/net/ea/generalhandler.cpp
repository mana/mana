/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "net/ea/generalhandler.h"

#include "client.h"
#include "configuration.h"
#include "log.h"

#include "gui/charselectdialog.h"
#include "gui/inventorywindow.h"
#include "gui/register.h"
#include "gui/skilldialog.h"
#include "gui/socialwindow.h"
#include "gui/statuswindow.h"

#include "net/ea/adminhandler.h"
#include "net/ea/beinghandler.h"
#include "net/ea/buysellhandler.h"
#include "net/ea/chathandler.h"
#include "net/ea/charserverhandler.h"
#include "net/ea/gamehandler.h"
#include "net/ea/guildhandler.h"
#include "net/ea/inventoryhandler.h"
#include "net/ea/itemhandler.h"
#include "net/ea/loginhandler.h"
#include "net/ea/network.h"
#include "net/ea/npchandler.h"
#include "net/ea/partyhandler.h"
#include "net/ea/playerhandler.h"
#include "net/ea/protocol.h"
#include "net/ea/tradehandler.h"
#include "net/ea/specialhandler.h"

#include "net/ea/gui/guildtab.h"
#include "net/ea/gui/partytab.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "resources/itemdb.h"

#include "utils/gettext.h"

#include <assert.h>
#include <list>

extern Net::GeneralHandler *generalHandler;

namespace EAthena {

ServerInfo charServer;
ServerInfo mapServer;

extern Guild *eaGuild;
extern Party *eaParty;

GeneralHandler::GeneralHandler():
    mAdminHandler(new AdminHandler),
    mBeingHandler(new BeingHandler(config.getValue("EnableSync", 0) == 1)),
    mBuySellHandler(new BuySellHandler),
    mCharHandler(new CharServerHandler),
    mChatHandler(new ChatHandler),
    mGameHandler(new GameHandler),
    mGuildHandler(new GuildHandler),
    mInventoryHandler(new InventoryHandler),
    mItemHandler(new ItemHandler),
    mLoginHandler(new LoginHandler),
    mNpcHandler(new NpcHandler),
    mPartyHandler(new PartyHandler),
    mPlayerHandler(new PlayerHandler),
    mSpecialHandler(new SpecialHandler),
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

void GeneralHandler::handleMessage(Net::MessageIn &msg)
{
    int code;

    switch (msg.getId())
    {
        case SMSG_CONNECTION_PROBLEM:
            code = msg.readInt8();
            logger->log("Connection problem: %i", code);

            switch (code)
            {
                case 0:
                    errorMessage = _("Authentication failed.");
                    break;
                case 1:
                    errorMessage = _("No servers available.");
                    break;
                case 2:
                    if (Client::getState() == STATE_GAME)
                        errorMessage = _("Someone else is trying to use this "
                                         "account.");
                    else
                        errorMessage = _("This account is already logged in.");
                    break;
                case 3:
                    errorMessage = _("Speed hack detected.");
                    break;
                case 8:
                    errorMessage = _("Duplicated login.");
                    break;
                default:
                    errorMessage = _("Unknown connection error.");
                    break;
            }
            Client::setState(STATE_ERROR);
            break;
    }
}

void GeneralHandler::load()
{
    (new Network)->registerHandler(this);

    mNetwork->registerHandler(mAdminHandler.get());
    mNetwork->registerHandler(mBeingHandler.get());
    mNetwork->registerHandler(mBuySellHandler.get());
    mNetwork->registerHandler(mChatHandler.get());
    mNetwork->registerHandler(mCharHandler.get());
    mNetwork->registerHandler(mGameHandler.get());
    mNetwork->registerHandler(mGuildHandler.get());
    mNetwork->registerHandler(mInventoryHandler.get());
    mNetwork->registerHandler(mItemHandler.get());
    mNetwork->registerHandler(mLoginHandler.get());
    mNetwork->registerHandler(mNpcHandler.get());
    mNetwork->registerHandler(mPlayerHandler.get());
    mNetwork->registerHandler(mSpecialHandler.get());
    mNetwork->registerHandler(mTradeHandler.get());
    mNetwork->registerHandler(mPartyHandler.get());
}

void GeneralHandler::reload()
{
    if (mNetwork)
        mNetwork->disconnect();

    static_cast<LoginHandler*>(mLoginHandler.get())->clearWorlds();
    static_cast<CharServerHandler*>(mCharHandler.get())->setCharCreateDialog(0);
    static_cast<CharServerHandler*>(mCharHandler.get())->setCharSelectDialog(0);
}

void GeneralHandler::unload()
{
    if (mNetwork)
        mNetwork->clearHandlers();
}

void GeneralHandler::flushNetwork()
{
    if (!mNetwork)
        return;

    mNetwork->flush();
    mNetwork->dispatchMessages();

    if (mNetwork->getState() == Network::NET_ERROR)
    {
        if (!mNetwork->getError().empty())
            errorMessage = mNetwork->getError();
        else
            errorMessage = _("Got disconnected from server!");

        Client::setState(STATE_ERROR);
    }
}

void GeneralHandler::guiWindowsLoaded()
{
    inventoryWindow->setSplitAllowed(false);
    skillDialog->loadSkills("ea-skills.xml");

    statusWindow->addAttribute(STR, _("Strength"), true);
    statusWindow->addAttribute(AGI, _("Agility"), true);
    statusWindow->addAttribute(VIT, _("Vitality"), true);
    statusWindow->addAttribute(INT, _("Intelligence"), true);
    statusWindow->addAttribute(DEX, _("Dexterity"), true);
    statusWindow->addAttribute(LUK, _("Luck"), true);

    statusWindow->addAttribute(ATK, _("Attack"), false);
    statusWindow->addAttribute(DEF, _("Defense"), false);
    statusWindow->addAttribute(MATK, _("M.Attack"), false);
    statusWindow->addAttribute(MDEF, _("M.Defense"), false);
    statusWindow->addAttribute(HIT, _("% Accuracy"), false);
    statusWindow->addAttribute(FLEE, _("% Evade"), false);
    statusWindow->addAttribute(CRIT, _("% Critical"), false);
}

void GeneralHandler::guiWindowsUnloaded()
{
    socialWindow->removeTab(eaGuild);
    socialWindow->removeTab(eaParty);

    delete guildTab;
    guildTab = 0;

    delete partyTab;
    partyTab = 0;
}

void GeneralHandler::clearHandlers()
{
    mNetwork->clearHandlers();
}

} // namespace EAthena
