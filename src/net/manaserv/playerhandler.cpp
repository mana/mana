/*
 *  The Mana World
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "net/manaserv/playerhandler.h"
#include "net/manaserv/beinghandler.h"

#include "effectmanager.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "particle.h"
#include "npc.h"

#include "gui/chat.h"
#include "gui/gui.h"
#include "gui/okdialog.h"
#include "gui/viewport.h"

#include "net/net.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

/** @see in game.cpp */
extern const int MILLISECONDS_IN_A_TICK;

/**
 * Max. distance we are willing to scroll after a teleport;
 * everything beyond will reset the port hard.
 * 32 is the nominal tile width/height.
 * @todo: Make this parameter read from config.
 */
static const int MAP_TELEPORT_SCROLL_DISTANCE = 8 * 32;

extern Net::PlayerHandler *playerHandler;

namespace ManaServ {

void RespawnRequestListener::action(const gcn::ActionEvent &event)
{
    Net::getPlayerHandler()->respawn();
}

extern Connection *gameServerConnection;

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
        {   // TODO: Implement reconnecting to another game server
            std::string token = msg.readString(32);
            std::string address = msg.readString();
            int port = msg.readInt16();
            logger->log("Changing server to %s:%d", address.c_str(), port);
        } break;

        case GPMSG_PLAYER_ATTRIBUTE_CHANGE:
        {
            logger->log("ATTRIBUTE UPDATE:");
            while (msg.getUnreadLength())
            {
                int stat = msg.readInt16();
                int base = msg.readInt16();
                int value = msg.readInt16();
                logger->log("%d set to %d %d", stat, base, value);

                if (stat == BASE_ATTR_HP)
                {
                    player_node->setMaxHp(base);
                    player_node->setHp(value);
                }
                else
                {
                    player_node->setAttributeBase(stat, base);
                    player_node->setAttributeEffective(stat, value);
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

                player_node->setExperience(skill, current, next);
            }
        } break;

        case GPMSG_LEVELUP:
        {
            player_node->setLevel(msg.readInt16());
            player_node->setCharacterPoints(msg.readInt16());
            player_node->setCorrectionPoints(msg.readInt16());
            Particle* effect = particleEngine->addEffect("graphics/particles/levelup.particle.xml", 0, 0);
            player_node->controlParticle(effect);
        } break;


        case GPMSG_LEVEL_PROGRESS:
        {
            player_node->setExp(msg.readInt8(), false);
        } break;


        case GPMSG_RAISE_ATTRIBUTE_RESPONSE:
        {
            int errCode = msg.readInt8();
            int attrNum = msg.readInt8() - CHAR_ATTR_BEGIN;
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
                    int attrValue = player_node->getAttributeBase(attrNum) - 1;
                    player_node->setCharacterPoints(0);
                    player_node->setAttributeBase(attrNum, attrValue);
                } break;
                case ATTRIBMOD_DENIED:
                {
                    // undo attribute change
                    logger->log("Warning: Server denied increase of attribute %d (reason unknown) ", attrNum);
                    int points = player_node->getCharacterPoints() - 1;
                    player_node->setCharacterPoints(points);
                    int attrValue = player_node->getAttributeBase(attrNum) - 1;
                    player_node->setAttributeBase(attrNum, attrValue);
                } break;
            }
        } break;

        case GPMSG_LOWER_ATTRIBUTE_RESPONSE:
        {
            int errCode = msg.readInt8();
            int attrNum = msg.readInt8() - CHAR_ATTR_BEGIN;
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
                    int attrValue = player_node->getAttributeBase(attrNum) + 1;
                    player_node->setCorrectionPoints(0);
                    player_node->setAttributeBase(attrNum, attrValue);
                    break;
                } break;
                case ATTRIBMOD_DENIED:
                {
                    // undo attribute change
                    logger->log("Warning: Server denied reduction of attribute %d (reason unknown) ", attrNum);
                    int charaPoints = player_node->getCharacterPoints() - 1;
                    player_node->setCharacterPoints(charaPoints);
                    int correctPoints = player_node->getCorrectionPoints() + 1;
                    player_node->setCorrectionPoints(correctPoints);
                    int attrValue = player_node->getAttributeBase(attrNum) + 1;
                    player_node->setAttributeBase(attrNum, attrValue);
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
                player_node->setSpecialStatus(id, current, max, recharge);
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

    const Vector &playerPos = player_node->getPosition();
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

    player_node->setAction(Being::STAND);
    player_node->setPosition(x, y);
    player_node->setDestination(x, y);

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

void PlayerHandler::increaseAttribute(size_t attr)
{
    MessageOut msg(PGMSG_RAISE_ATTRIBUTE);
    msg.writeInt8(attr);
    gameServerConnection->send(msg);
}

void PlayerHandler::decreaseAttribute(size_t attr)
{
    MessageOut msg(PGMSG_LOWER_ATTRIBUTE);
    msg.writeInt8(attr);
    gameServerConnection->send(msg);
}

void PlayerHandler::increaseSkill(int skillId)
{
    // Not used atm
}

void PlayerHandler::pickUp(FloorItem *floorItem)
{
    int id = floorItem->getId();
    MessageOut msg(PGMSG_PICKUP);
    msg.writeInt16(id >> 16);
    msg.writeInt16(id & 0xFFFF);
    gameServerConnection->send(msg);
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
    player_node->setAction(action);

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

Vector PlayerHandler::getDefaultWalkSpeed()
{
    // Return translation in pixels per ticks.
    return ManaServ::BeingHandler::giveSpeedInPixelsPerTicks(6.0f);
}

} // namespace ManaServ
