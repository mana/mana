/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2013  The Mana Developers
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

#include "net/tmwa/generalhandler.h"

#include "client.h"
#include "configuration.h"
#include "log.h"

#include "gui/skilldialog.h"
#include "gui/socialwindow.h"
#include "gui/statuswindow.h"

#include "net/serverinfo.h"

#include "net/tmwa/adminhandler.h"
#include "net/tmwa/beinghandler.h"
#include "net/tmwa/buysellhandler.h"
#include "net/tmwa/chathandler.h"
#include "net/tmwa/charserverhandler.h"
#include "net/tmwa/gamehandler.h"
#include "net/tmwa/guildhandler.h"
#include "net/tmwa/inventoryhandler.h"
#include "net/tmwa/itemhandler.h"
#include "net/tmwa/loginhandler.h"
#include "net/tmwa/messagein.h"
#include "net/tmwa/network.h"
#include "net/tmwa/npchandler.h"
#include "net/tmwa/partyhandler.h"
#include "net/tmwa/playerhandler.h"
#include "net/tmwa/protocol.h"
#include "net/tmwa/tradehandler.h"
#include "net/tmwa/abilityhandler.h"

#include "net/tmwa/gui/guildtab.h"
#include "net/tmwa/gui/partytab.h"

#include "resources/itemdb.h"

#include "utils/gettext.h"

#include <list>

namespace TmwAthena {

ServerInfo charServer;
ServerInfo mapServer;

extern Guild *taGuild;
extern Party *taParty;

GeneralHandler::GeneralHandler():
    mAdminHandler(new AdminHandler),
    mBeingHandler(new BeingHandler(config.enableSync)),
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
    mAbilityHandler(new AbilityHandler),
    mTradeHandler(new TradeHandler)
{
    static const Uint16 _messages[] = {
        SMSG_CONNECTION_PROBLEM,
        0
    };
    handledMessages = _messages;

    std::list<ItemStat> stats;
    stats.emplace_back("str", _("Strength %+d"));
    stats.emplace_back("agi", _("Agility %+d"));
    stats.emplace_back("vit", _("Vitality %+d"));
    stats.emplace_back("int", _("Intelligence %+d"));
    stats.emplace_back("dex", _("Dexterity %+d"));
    stats.emplace_back("luck", _("Luck %+d"));

    setStatsList(std::move(stats));

    listen(Event::ClientChannel);
    listen(Event::GameChannel);
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
            Log::info("Connection problem: %i", code);

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
    // This sets mNetwork to the created Network instance
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
    mNetwork->registerHandler(mAbilityHandler.get());
    mNetwork->registerHandler(mTradeHandler.get());
    mNetwork->registerHandler(mPartyHandler.get());
}

void GeneralHandler::reload()
{
    if (mNetwork)
        mNetwork->disconnect();

    static_cast<LoginHandler*>(mLoginHandler.get())->clearWorlds();
    static_cast<CharServerHandler*>(mCharHandler.get())->setCharCreateDialog(nullptr);
    static_cast<CharServerHandler*>(mCharHandler.get())->setCharSelectDialog(nullptr);
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

void GeneralHandler::event(Event::Channel channel,
                           const Event &event)
{
    if (channel == Event::GameChannel)
    {
        if (event.getType() == Event::GuiWindowsLoaded)
        {
            skillDialog->loadSkills();

            statusWindow->addAttribute(STRENGTH, _("Strength"), true, "");
            statusWindow->addAttribute(AGILITY, _("Agility"), true, "");
            statusWindow->addAttribute(VITALITY, _("Vitality"), true, "");
            statusWindow->addAttribute(INTELLIGENCE, _("Intelligence"),
                                       true, "");
            statusWindow->addAttribute(DEXTERITY, _("Dexterity"), true, "");
            statusWindow->addAttribute(LUCK, _("Luck"), true, "");

            statusWindow->addAttribute(ATK, _("Attack"), false, "");
            statusWindow->addAttribute(DEF, _("Defense"), false, "");
            statusWindow->addAttribute(MATK, _("M.Attack"), false, "");
            statusWindow->addAttribute(MDEF, _("M.Defense"), false, "");
            // NOTE: Don't remove the gettext comments as they are used
            // by the xgettext invocation.
            //xgettext:no-c-format
            statusWindow->addAttribute(HIT, _("% Accuracy"), false, "");
            //xgettext:no-c-format
            statusWindow->addAttribute(FLEE, _("% Evade"), false, "");
            //xgettext:no-c-format
            statusWindow->addAttribute(CRIT, _("% Critical"), false, "");
        }
        else if (event.getType() == Event::GuiWindowsUnloading)
        {
            socialWindow->removeTab(taGuild);
            socialWindow->removeTab(taParty);

            delete guildTab;
            guildTab = nullptr;

            delete partyTab;
            partyTab = nullptr;
        }
    }
}

} // namespace TmwAthena
