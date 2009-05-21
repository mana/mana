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

#include "net/tmwserv/playerhandler.h"

#include "net/tmwserv/connection.h"
#include "net/tmwserv/protocol.h"

#include "net/tmwserv/gameserver/internal.h"
#include "net/tmwserv/gameserver/player.h"

#include "net/messagein.h"
#include "net/messageout.h"
#include "net/net.h"

#include "effectmanager.h"
#include "engine.h"
#include "localplayer.h"
#include "log.h"
#include "particle.h"
#include "npc.h"

#include "gui/buy.h"
#include "gui/chat.h"
#include "gui/gui.h"
#include "gui/okdialog.h"
#include "gui/sell.h"
#include "gui/skill.h"
#include "gui/viewport.h"

// TODO Move somewhere else
OkDialog *weightNotice = NULL;
OkDialog *deathNotice = NULL;

extern BuyDialog *buyDialog;
extern SellDialog *sellDialog;
extern Window *buySellDialog;

/* Max. distance we are willing to scroll after a teleport;
 * everything beyond will reset the port hard.
 */
static const int MAP_TELEPORT_SCROLL_DISTANCE = 8 * 32;

/**
 * Listener used for handling the overweigth message.
 */
// TODO Move somewhere else
namespace {
    struct WeightListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            weightNotice = NULL;
        }
    } weightListener;
}

/**
 * Listener used for handling death message.
 */
// TODO Move somewhere else
namespace {
    struct DeathListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            Net::getPlayerHandler()->respawn();
            deathNotice = NULL;
            buyDialog->setVisible(false);
            sellDialog->setVisible(false);
            current_npc = 0;
        }
    } deathListener;
}

Net::PlayerHandler *playerHandler;

namespace TmwServ {

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
                int stat = msg.readInt8();
                int base = msg.readInt16();
                int value = msg.readInt16();
                logger->log("%d set to %d %d", stat, base, value);

                if (stat == BASE_ATTR_HP)
                {
                    player_node->setMaxHp(base);
                    player_node->setHp(value);
                }
                else if (stat < NB_CHARACTER_ATTRIBUTES)
                {
                    if (stat >= CHAR_SKILL_BEGIN && stat < CHAR_SKILL_END
                        && player_node->getAttributeBase(stat) < base
                        && player_node->getAttributeBase(stat) > -1)
                    {
                        Particle* effect = particleEngine->addEffect("graphics/particles/skillup.particle.xml", 0, 0);
                        player_node->controlParticle(effect);
                    }

                    player_node->setAttributeBase(stat, base);
                    player_node->setAttributeEffective(stat, value);
                }
                else
                {
                    logger->log("Warning: server wants to update unknown "
                                "attribute %d to %d", stat, value);
                }
            }
        } break;

        case GPMSG_PLAYER_EXP_CHANGE:
        {
            logger->log("EXP Update");
            while (msg.getUnreadLength())
            {
                int skill = msg.readInt8();
                int current = msg.readInt32();
                int next = msg.readInt32();

                if (skill < CHAR_SKILL_NB)
                {
                    player_node->setExperience(skill, current, next);
                }
                else
                {
                    logger->log("Warning: server wants to update experience of unknown "
                                "skill  %d to %d / %d", skill, current, next);
                }
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
            logger->log("Level Progress Update");
            player_node->setLevelProgress(msg.readInt8());
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
        /*
        case SMSG_PLAYER_ARROW_MESSAGE:
            {
                Sint16 type = msg.readInt16();

                switch (type) {
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

void PlayerHandler::handleMapChangeMessage(MessageIn &msg)
{
    const std::string mapName = msg.readString();
    const unsigned short x = msg.readInt16();
    const unsigned short y = msg.readInt16();
    const bool nearby = (engine->getCurrentMapName() == mapName);

    logger->log("Changing map to %s (%d, %d)", mapName.c_str(), x, y);

    // Switch the actual map, deleting the previous one
    engine->changeMap(mapName);

    current_npc = 0;

    const Vector &playerPos = player_node->getPosition();
    float scrollOffsetX = 0.0f;
    float scrollOffsetY = 0.0f;

    /* Scroll if neccessary */
    if (!nearby
            || (abs(x - (int) playerPos.x) > MAP_TELEPORT_SCROLL_DISTANCE)
            || (abs(y - (int) playerPos.y) > MAP_TELEPORT_SCROLL_DISTANCE)) {
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

void PlayerHandler::attack(Being *being)
{
    // TODO
}

void PlayerHandler::emote(int emoteId)
{
    // TODO
}

void PlayerHandler::increaseStat(LocalPlayer::Attribute attr)
{
    // TODO
}

void PlayerHandler::decreaseStat(LocalPlayer::Attribute attr)
{
    // TODO
}

void PlayerHandler::pickUp(FloorItem *floorItem)
{
    int id = floorItem->getId();
    Net::GameServer::Player::pickUp(id >> 16, id & 0xFFFF);
}

void PlayerHandler::setDirection(char direction)
{
    MessageOut msg(PGMSG_DIRECTION_CHANGE);
    msg.writeInt8(direction);
    Net::GameServer::connection->send(msg);
}

void PlayerHandler::setDestination(int x, int y, int /* direction */)
{
    MessageOut msg(PGMSG_WALK);
    msg.writeInt16(x);
    msg.writeInt16(y);
    Net::GameServer::connection->send(msg);

    // Debugging fire burst
    effectManager->trigger(15, x, y);
}

void PlayerHandler::changeAction(Being::Action action)
{
    if (action == Being::SIT)
        player_node->setAction(action);

    MessageOut msg(PGMSG_ACTION_CHANGE);
    msg.writeInt8(action);
    Net::GameServer::connection->send(msg);
}

void PlayerHandler::respawn()
{
    // TODO
}

void PlayerHandler::ignorePlayer(const std::string &player, bool ignore)
{
    // TODO
}

void PlayerHandler::ignoreAll(bool ignore)
{
    // TODO
}

} // namespace TmwServ
