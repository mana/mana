/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "net/manaserv/playerhandler.h"

#include "client.h"
#include "effectmanager.h"
#include "event.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "particle.h"
#include "playerinfo.h"

#include "gui/viewport.h"

#include "net/net.h"

#include "net/abilityhandler.h"
#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

#include "resources/abilitydb.h"
#include "resources/attributes.h"

/**
 * Max. distance in tiles we are willing to scroll after a teleport;
 * everything beyond will reset the port hard.
 * @todo: Make this parameter read from config.
 */
const int MAP_TELEPORT_SCROLL_DISTANCE = 256;

extern Net::PlayerHandler *playerHandler;
extern Net::AbilityHandler *abilityHandler;

namespace ManaServ {

extern Connection *gameServerConnection;
extern std::string netToken;
extern ServerInfo gameServer;

void RespawnRequestListener::action(const gcn::ActionEvent &event)
{
    Net::getPlayerHandler()->respawn();

    Event::trigger(Event::NpcChannel, Event::CloseAll);
}

PlayerHandler::PlayerHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_PLAYER_MAP_CHANGE,
        GPMSG_PLAYER_SERVER_CHANGE,
        GPMSG_PLAYER_ATTRIBUTE_CHANGE,
        GPMSG_ATTRIBUTE_POINTS_STATUS,
        GPMSG_RAISE_ATTRIBUTE_RESPONSE,
        GPMSG_LOWER_ATTRIBUTE_RESPONSE,
        GPMSG_ABILITY_STATUS,
        GPMSG_ABILITY_REMOVED,
        0
    };
    handledMessages = _messages;
    playerHandler = this;
}

void PlayerHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_PLAYER_MAP_CHANGE:
            handleMapChangeMessage(msg);
            break;

        case GPMSG_PLAYER_SERVER_CHANGE:
        {   // TODO: Fix the servers to test this
            netToken = msg.readString(32);
            std::string address = msg.readString();
            int port = msg.readInt16();
            Log::info("Changing server to %s:%d", address.c_str(), port);

            gameServer.hostname = address;
            gameServer.port = port;

            gameServerConnection->disconnect();
            Client::setState(STATE_CHANGE_MAP);
            local_player->setMap(nullptr);
        } break;

        case GPMSG_PLAYER_ATTRIBUTE_CHANGE:
        {
            while (msg.getUnreadLength())
            {
                int attrId = msg.readInt16();
                double base = msg.readInt32() / 256.0;
                double value = msg.readInt32() / 256.0;

                // Set the core player attribute the stat
                // depending on attribute link.
                int playerInfoId =
                                  Attributes::getPlayerInfoIdFromAttrId(attrId);
                if (playerInfoId > -1)
                {
                    PlayerInfo::setAttribute(playerInfoId, value);
                }
                else
                {
                    PlayerInfo::setStatBase(attrId, base);
                    PlayerInfo::setStatMod(attrId, value - base);
                }
            }
        } break;

        case GPMSG_ATTRIBUTE_POINTS_STATUS:
            PlayerInfo::setAttribute(CHAR_POINTS, msg.readInt16());
            PlayerInfo::setAttribute(CORR_POINTS, msg.readInt16());
            break;

        case GPMSG_RAISE_ATTRIBUTE_RESPONSE:
        {
            int errCode = msg.readInt8();
            int attrNum = msg.readInt16();
            switch (errCode)
            {
                case ATTRIBMOD_OK:
                {
                    // feel(acknowledgment);
                } break;
                case ATTRIBMOD_INVALID_ATTRIBUTE:
                {
                    Log::warn("Server denied increase of attribute %d (unknown attribute) ", attrNum);
                } break;
                case ATTRIBMOD_NO_POINTS_LEFT:
                {
                    // when the server says "you got no points" it
                    // has to be correct. The server is always right!
                    // undo attribute change and set points to 0
                    Log::warn("Server denied increase of attribute %d (no points left) ", attrNum);
                    int attrValue = PlayerInfo::getStatBase(attrNum) - 1;
                    PlayerInfo::setAttribute(CHAR_POINTS, 0);
                    PlayerInfo::setStatBase(attrNum, attrValue);
                } break;
                case ATTRIBMOD_DENIED:
                {
                    // undo attribute change
                    Log::warn("Server denied increase of attribute %d (reason unknown) ", attrNum);
                    int points = PlayerInfo::getAttribute(CHAR_POINTS) - 1;
                    PlayerInfo::setAttribute(CHAR_POINTS, points);

                    int attrValue = PlayerInfo::getStatBase(attrNum) - 1;
                    PlayerInfo::setStatBase(attrNum, attrValue);
                } break;
            }
        } break;

        case GPMSG_LOWER_ATTRIBUTE_RESPONSE:
        {
            int errCode = msg.readInt8();
            int attrNum = msg.readInt16();
            switch (errCode)
            {
                case ATTRIBMOD_OK:
                {
                    // feel(acknowledgment);
                } break;
                case ATTRIBMOD_INVALID_ATTRIBUTE:
                {
                    Log::warn("Server denied reduction of attribute %d (unknown attribute) ", attrNum);
                } break;
                case ATTRIBMOD_NO_POINTS_LEFT:
                {
                    // when the server says "you got no points" it
                    // has to be correct. The server is always right!
                    // undo attribute change and set points to 0
                    Log::warn("Server denied reduction of attribute %d (no points left) ", attrNum);
                    int attrValue = PlayerInfo::getStatBase(attrNum) + 1;
                    // TODO are these right?
                    PlayerInfo::setAttribute(CHAR_POINTS, 0);
                    PlayerInfo::setAttribute(CORR_POINTS, 0);
                    PlayerInfo::setStatBase(attrNum, attrValue);
                } break;
                case ATTRIBMOD_DENIED:
                {
                    // undo attribute change
                    Log::warn("Server denied reduction of attribute %d (reason unknown) ", attrNum);
                    int charaPoints = PlayerInfo::getAttribute(CHAR_POINTS) - 1;
                    PlayerInfo::setAttribute(CHAR_POINTS, charaPoints);

                    int correctPoints = PlayerInfo::getAttribute(CORR_POINTS) + 1;
                    PlayerInfo::setAttribute(CORR_POINTS, correctPoints);

                    int attrValue = PlayerInfo::getStatBase(attrNum) + 1;
                    PlayerInfo::setStatBase(attrNum, attrValue);
                } break;
            }

        } break;

        case GPMSG_ABILITY_STATUS:
        {
            while (msg.getUnreadLength())
            {
                int id = msg.readInt8();
                int current = msg.readInt32();
                int max = msg.readInt32();
                int recharge = msg.readInt32();
                PlayerInfo::setAbilityStatus(id, current, max, recharge);
            }
        } break;

        case GPMSG_ABILITY_REMOVED:
        {
            int id = msg.readInt8();
            PlayerInfo::clearAbilityStatus(id);
        } break;

        /*
        case SMSG_PLAYER_ARROW_MESSAGE:
            {
                Sint16 type = msg.readInt16();

                switch (type)
                {
                    case 0:
                        localChatTab->chatLog(_("Equip arrows first."),
                                             BY_SERVER);
                        break;
                    default:
                        Log::info("0x013b: Unhandled message %i", type);
                        break;
                }
            }
            break;
        */
    }
}

void PlayerHandler::handleMapChangeMessage(MessageIn &msg)
{
    const std::string mapName = msg.readString();
    const unsigned short x = msg.readInt16();
    const unsigned short y = msg.readInt16();

    Game *game = Game::instance();
    const bool sameMap = (game->getCurrentMapName() == mapName);

    Log::info("Changing map to %s (%d, %d)", mapName.c_str(), x, y);

    // Switch the actual map, deleting the previous one
    game->changeMap(mapName);

    const Vector &playerPos = local_player->getPosition();
    float scrollOffsetX = 0.0f;
    float scrollOffsetY = 0.0f;

    /* Scroll if neccessary */
    if (!sameMap
            || (abs(x - (int) playerPos.x) > MAP_TELEPORT_SCROLL_DISTANCE)
            || (abs(y - (int) playerPos.y) > MAP_TELEPORT_SCROLL_DISTANCE))
    {
        scrollOffsetX = x - (int) playerPos.x;
        scrollOffsetY = y - (int) playerPos.y;
    }

    local_player->setAction(Being::STAND);
    local_player->setPosition(x, y);
    local_player->setDestination(x, y);

    Log::info("Adjust scrolling by %d,%d", (int) scrollOffsetX,
                                           (int) scrollOffsetY);
    viewport->scrollBy(scrollOffsetX, scrollOffsetY);
}

void PlayerHandler::attack(int id)
{
    auto ability = AbilityDB::find("Strike");
    if (!ability)
    {
        Log::info("PlayerHandler::attack: 'Strike' ability not found.");
        return;
    }

    switch (ability->targetMode) {
    case AbilityInfo::TARGET_BEING:
        abilityHandler->useOn(ability->id, id);
        break;
    case AbilityInfo::TARGET_POINT:
        Log::info("PlayerHandler::attack: Unsupported target mode 'point' for 'Strike' ability.");
        break;
    case AbilityInfo::TARGET_DIRECTION:
        abilityHandler->useInDirection(ability->id, local_player->getDirection());
        break;
    }
}

void PlayerHandler::emote(int emoteId)
{
    MessageOut msg(PGMSG_BEING_EMOTE);
    msg.writeInt8(emoteId);
    gameServerConnection->send(msg);
}

void PlayerHandler::increaseAttribute(int attr)
{
    MessageOut msg(PGMSG_RAISE_ATTRIBUTE);
    msg.writeInt16(attr);
    gameServerConnection->send(msg);
}

void PlayerHandler::decreaseAttribute(int attr)
{
    MessageOut msg(PGMSG_LOWER_ATTRIBUTE);
    msg.writeInt16(attr);
    gameServerConnection->send(msg);
}

void PlayerHandler::increaseSkill(int skillId)
{
    // Not used atm
}

void PlayerHandler::pickUp(FloorItem *floorItem)
{
    if (floorItem)
    {
        MessageOut msg(PGMSG_PICKUP);
        msg.writeInt16(floorItem->getPixelX());
        msg.writeInt16(floorItem->getPixelY());
        gameServerConnection->send(msg);
    }
}

void PlayerHandler::setDirection(char direction)
{
    MessageOut msg(PGMSG_DIRECTION_CHANGE);
    msg.writeInt8(direction);
    gameServerConnection->send(msg);
}

void PlayerHandler::setDestination(int x, int y, int /* direction */)
{
    MessageOut msg(PGMSG_WALK);
    msg.writeInt16(x);
    msg.writeInt16(y);
    gameServerConnection->send(msg);
}

void PlayerHandler::changeAction(Being::Action action)
{
    local_player->setAction(action);

    MessageOut msg(PGMSG_ACTION_CHANGE);
    msg.writeInt8(action);
    gameServerConnection->send(msg);
}

void PlayerHandler::respawn()
{
    MessageOut msg(PGMSG_RESPAWN);
    gameServerConnection->send(msg);
}

void PlayerHandler::ignorePlayer(const std::string &player, bool ignore)
{
    // TODO
}

void PlayerHandler::ignoreAll(bool ignore)
{
    // TODO
}

bool PlayerHandler::canUseMagic()
{
    return true;
}

bool PlayerHandler::canCorrectAttributes()
{
    return true;
}

int PlayerHandler::getJobLocation()
{
    return -1;
}

Vector PlayerHandler::getDefaultMoveSpeed() const
{
    // Return default speed at 6 tiles per second.
    return Vector(6.0f, 6.0f, 0.0f);
}

Vector PlayerHandler::getPixelsPerSecondMoveSpeed(const Vector &speed, Map *map)
{
    Vector speedInPixels;

    Game *game = Game::instance();
    if (game && !map)
        map = game->getCurrentMap();

    if (!map)
    {
        Log::info("Manaserv::PlayerHandler: Speed wasn't given back"
                  " because Map not initialized.");
        return speedInPixels;
    }

    speedInPixels.x = speed.x * map->getTileWidth();
    speedInPixels.y = speed.y * map->getTileHeight();

    return speedInPixels;
}

} // namespace ManaServ
