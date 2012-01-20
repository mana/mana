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

#include "net/manaserv/playerhandler.h"
#include "net/manaserv/beinghandler.h"

#include "client.h"
#include "effectmanager.h"
#include "event.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "particle.h"
#include "playerinfo.h"
#include "configuration.h"

#include "gui/gui.h"
#include "gui/okdialog.h"
#include "gui/viewport.h"

#include "net/net.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/defines.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"
#include "net/manaserv/attributes.h"

/**
 * Max. distance in tiles we are willing to scroll after a teleport;
 * everything beyond will reset the port hard.
 * @todo: Make this parameter read from config.
 */
const int MAP_TELEPORT_SCROLL_DISTANCE = 256;

extern Net::PlayerHandler *playerHandler;

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
        GPMSG_PLAYER_EXP_CHANGE,
        GPMSG_LEVELUP,
        GPMSG_LEVEL_PROGRESS,
        GPMSG_RAISE_ATTRIBUTE_RESPONSE,
        GPMSG_LOWER_ATTRIBUTE_RESPONSE,
        GPMSG_SPECIAL_STATUS,
        0
    };
    handledMessages = _messages;
    playerHandler = this;
}

void PlayerHandler::handleMessage(Net::MessageIn &msg)
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
            logger->log("Changing server to %s:%d", address.c_str(), port);

            gameServer.hostname = address;
            gameServer.port = port;

            gameServerConnection->disconnect();
            Client::setState(STATE_CHANGE_MAP);
            local_player->setMap(0);
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

        case GPMSG_PLAYER_EXP_CHANGE:
        {
            logger->log("EXP Update");
            while (msg.getUnreadLength())
            {
                int skill = msg.readInt16();
                int current = msg.readInt32();
                int next = msg.readInt32();

                PlayerInfo::setStatExperience(skill, current, next);
            }
        } break;

        case GPMSG_LEVELUP:
        {
            PlayerInfo::setAttribute(LEVEL, msg.readInt16());
            PlayerInfo::setAttribute(CHAR_POINTS, msg.readInt16());
            PlayerInfo::setAttribute(CORR_POINTS, msg.readInt16());
            Particle* effect = particleEngine->addEffect(
                                               paths.getStringValue("particles")
                                     + paths.getStringValue("levelUpEffectFile")
                                                         ,0, 0);
            local_player->controlParticle(effect);
        } break;


        case GPMSG_LEVEL_PROGRESS:
        {
            PlayerInfo::setAttribute(EXP, msg.readInt8());
        } break;


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
                    logger->log("Warning: Server denied increase of attribute %d (unknown attribute) ", attrNum);
                } break;
                case ATTRIBMOD_NO_POINTS_LEFT:
                {
                    // when the server says "you got no points" it
                    // has to be correct. The server is always right!
                    // undo attribute change and set points to 0
                    logger->log("Warning: Server denied increase of attribute %d (no points left) ", attrNum);
                    int attrValue = PlayerInfo::getStatBase(attrNum) - 1;
                    PlayerInfo::setAttribute(CHAR_POINTS, 0);
                    PlayerInfo::setStatBase(attrNum, attrValue);
                } break;
                case ATTRIBMOD_DENIED:
                {
                    // undo attribute change
                    logger->log("Warning: Server denied increase of attribute %d (reason unknown) ", attrNum);
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
                    logger->log("Warning: Server denied reduction of attribute %d (unknown attribute) ", attrNum);
                } break;
                case ATTRIBMOD_NO_POINTS_LEFT:
                {
                    // when the server says "you got no points" it
                    // has to be correct. The server is always right!
                    // undo attribute change and set points to 0
                    logger->log("Warning: Server denied reduction of attribute %d (no points left) ", attrNum);
                    int attrValue = PlayerInfo::getStatBase(attrNum) + 1;
                    // TODO are these right?
                    PlayerInfo::setAttribute(CHAR_POINTS, 0);
                    PlayerInfo::setAttribute(CORR_POINTS, 0);
                    PlayerInfo::setStatBase(attrNum, attrValue);
                } break;
                case ATTRIBMOD_DENIED:
                {
                    // undo attribute change
                    logger->log("Warning: Server denied reduction of attribute %d (reason unknown) ", attrNum);
                    int charaPoints = PlayerInfo::getAttribute(CHAR_POINTS) - 1;
                    PlayerInfo::setAttribute(CHAR_POINTS, charaPoints);

                    int correctPoints = PlayerInfo::getAttribute(CORR_POINTS) + 1;
                    PlayerInfo::setAttribute(CORR_POINTS, correctPoints);

                    int attrValue = PlayerInfo::getStatBase(attrNum) + 1;
                    PlayerInfo::setStatBase(attrNum, attrValue);
                } break;
            }

        } break;

        case GPMSG_SPECIAL_STATUS :
        {
            while (msg.getUnreadLength())
            {
                // { B specialID, L current, L max, L recharge }
                int id = msg.readInt8();
                int current = msg.readInt32();
                int max = msg.readInt32();
                int recharge = msg.readInt32();
                PlayerInfo::setSpecialStatus(id, current, max, recharge);
            }
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
                        logger->log("0x013b: Unhandled message %i", type);
                        break;
                }
            }
            break;
        */
    }
}

void PlayerHandler::handleMapChangeMessage(Net::MessageIn &msg)
{
    const std::string mapName = msg.readString();
    const unsigned short x = msg.readInt16();
    const unsigned short y = msg.readInt16();

    Game *game = Game::instance();
    const bool sameMap = (game->getCurrentMapName() == mapName);

    logger->log("Changing map to %s (%d, %d)", mapName.c_str(), x, y);

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

    logger->log("Adjust scrolling by %d,%d", (int) scrollOffsetX,
                                             (int) scrollOffsetY);
    viewport->scrollBy(scrollOffsetX, scrollOffsetY);
}

void PlayerHandler::attack(int id)
{
    MessageOut msg(PGMSG_ATTACK);
    msg.writeInt16(id);
    gameServerConnection->send(msg);
}

void PlayerHandler::emote(int emoteId)
{
    // TODO
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

Vector PlayerHandler::getPixelsPerTickMoveSpeed(const Vector &speed, Map *map)
{
    Vector speedInTicks;

    Game *game = Game::instance();
    if (game && !map)
        map = game->getCurrentMap();

    if (!map)
    {
        logger->log("Manaserv::PlayerHandler: Speed wasn't given back"
                    " because Map not initialized.");
        return speedInTicks;
    }

    speedInTicks.x = speed.x
        * (float)map->getTileWidth()
        / 1000 * (float) MILLISECONDS_IN_A_TICK;
    speedInTicks.y = speed.y
        * (float)map->getTileHeight()
        / 1000 * (float) MILLISECONDS_IN_A_TICK;

    return speedInTicks;
}

} // namespace ManaServ
