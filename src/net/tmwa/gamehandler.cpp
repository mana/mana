/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "net/tmwa/gamehandler.h"

#include "client.h"
#include "event.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"

#include "gui/okdialog.h"

#include "net/tmwa/loginhandler.h"
#include "net/tmwa/messagein.h"
#include "net/tmwa/messageout.h"
#include "net/tmwa/network.h"
#include "net/tmwa/protocol.h"

#include "utils/gettext.h"

extern Net::GameHandler *gameHandler;

namespace TmwAthena {

extern ServerInfo mapServer;

GameHandler::GameHandler()
{
    static const Uint16 _messages[] = {
        SMSG_MAP_LOGIN_SUCCESS,
        SMSG_CHAR_SWITCH_RESPONSE,
        SMSG_MAP_QUIT_RESPONSE,
        0
    };
    handledMessages = _messages;
    gameHandler = this;

    listen(Event::GameChannel);
}

void GameHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_MAP_LOGIN_SUCCESS:
        {
            unsigned char direction;
            Uint16 x, y;
            msg.readInt32();   // server tick
            msg.readCoordinates(x, y, direction);
            msg.skip(2);      // unknown
            Log::info("Protocol: Player start position: (%d, %d), Direction: %d",
                      x, y, direction);
            // Switch now or we'll have problems
            Client::setState(STATE_GAME);
            // Stores the position until the map is loaded.
            mTileX = x; mTileY = y;
         }  break;

        case SMSG_CHAR_SWITCH_RESPONSE:
            if (msg.readInt8())
            {
                Client::setState(STATE_SWITCH_CHARACTER);
            }
            break;

        case SMSG_MAP_QUIT_RESPONSE:
            if (msg.readInt16())
            {
                new OkDialog(_("Game"), _("Request to quit denied!"), false,
                                                                          nullptr);
            }
            break;
    }
}

void GameHandler::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::GameChannel)
    {
        if (event.getType() == Event::EnginesInitialized)
        {
            Game *game = Game::instance();
            game->changeMap(mMap);

            // It could be that loading the map failed
            if (Map *map = game->getCurrentMap())
            {
                if (mTileX && mTileY)
                {
                    local_player->setPosition(map->getTileCenter(mTileX, mTileY));
                    mTileX = mTileY = 0;
                }
            }
        }
        else if (event.getType() == Event::MapLoaded)
        {
            MessageOut outMsg(CMSG_MAP_LOADED);
        }
    }
}

void GameHandler::connect()
{
    mNetwork->connect(mapServer);

    const Token &token =
            static_cast<LoginHandler*>(Net::getLoginHandler())->getToken();


    if (Client::getState() == STATE_CONNECT_GAME)
    {
        mCharID = local_player->getId();
        // Change the player's ID to the account ID to match what eAthena uses
        local_player->setId(token.account_ID);
    }

    // Send login infos
    MessageOut outMsg(CMSG_MAP_SERVER_CONNECT);
    outMsg.writeInt32(token.account_ID);
    outMsg.writeInt32(mCharID);
    outMsg.writeInt32(token.session_ID1);
    outMsg.writeInt32(token.session_ID2);
    outMsg.writeInt8(token.sex == Gender::Male ? 1 : 0);

    // We get 4 useless bytes before the real answer comes in (what are these?)
    mNetwork->skip(4);
}

bool GameHandler::isConnected()
{
    return mNetwork->isConnected();
}

void GameHandler::disconnect()
{
    mNetwork->disconnect();
}

void GameHandler::quit()
{
    MessageOut outMsg(CMSG_CLIENT_QUIT);
}

void GameHandler::setMap(const std::string &map)
{
    mMap = map.substr(0, map.rfind("."));
}

int GameHandler::getPickupRange() const
{
    if (Game *game = Game::instance())
        return game->getCurrentTileWidth();
    return DEFAULT_TILE_LENGTH;
}

} // namespace TmwAthena
