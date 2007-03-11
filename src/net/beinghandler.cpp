/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "beinghandler.h"

#include <SDL_types.h>

#include "messagein.h"
#include "protocol.h"

#include "../being.h"
#include "../beingmanager.h"
#include "../game.h"
#include "../localplayer.h"
#include "../log.h"
#include "../main.h"
#include "../sound.h"

const int EMOTION_TIME = 150;    /**< Duration of emotion icon */

BeingHandler::BeingHandler()
{
    static const Uint16 _messages[] = {
        //SMSG_BEING_VISIBLE,
        //SMSG_BEING_MOVE,
        //SMSG_BEING_REMOVE,
        //SMSG_BEING_ACTION,
        //SMSG_BEING_LEVELUP,
        //SMSG_BEING_EMOTION,
        //SMSG_BEING_CHANGE_LOOKS,
        //SMSG_BEING_NAME_RESPONSE,
        //SMSG_PLAYER_UPDATE_1,
        //SMSG_PLAYER_UPDATE_2,
        //SMSG_PLAYER_MOVE,
        //0x0119,
        GPMSG_BEING_ATTACK,
        GPMSG_BEING_ENTER,
        GPMSG_BEING_LEAVE,
        GPMSG_BEINGS_MOVE,
        GPMSG_BEINGS_DAMAGE,
        GPMSG_BEING_ACTION_CHANGE,
        0
    };
    handledMessages = _messages;
}

void BeingHandler::handleMessage(MessageIn &msg)
{
    /*
    Uint32 id;
    Uint16 job, speed;
    Uint16 headBottom, headTop, headMid;
    Sint16 param1;
    Sint8 type;
    Being *srcBeing, *dstBeing;
    */

    switch (msg.getId())
    {
        case GPMSG_BEING_ENTER:
            handleBeingEnterMessage(msg);
            break;
        case GPMSG_BEING_LEAVE:
            handleBeingLeaveMessage(msg);
            break;
        case GPMSG_BEINGS_MOVE:
            handleBeingsMoveMessage(msg);
            break;
        case GPMSG_BEING_ATTACK:
            handleBeingAttackMessage(msg);
            break;
        case GPMSG_BEINGS_DAMAGE:
            handleBeingsDamageMessage(msg);
            break;
        case GPMSG_BEING_ACTION_CHANGE:
            handleBeingActionChangeMessage(msg);
            break;

        /*
        case SMSG_BEING_VISIBLE:
        case SMSG_BEING_MOVE:
            // Information about a being in range
            id = msg.readLong();
            speed = msg.readShort();
            msg.readShort();  // unknown
            msg.readShort();  // unknown
            msg.readShort();  // option
            job = msg.readShort();  // class

            dstBeing = beingManager->findBeing(id);

            if (dstBeing == NULL)
            {
                // Being with id >= 110000000 and job 0 are better
                // known as ghosts, so don't create those.
                if (job == 0 && id >= 110000000)
                {
                    break;
                }

                dstBeing = beingManager->createBeing(id, job);
            }
            else if (msg.getId() == 0x0078)
            {
                dstBeing->clearPath();
                dstBeing->mFrame = 0;
                dstBeing->mWalkTime = tick_time;
                dstBeing->setAction(Being::STAND);
            }

            // Prevent division by 0 when calculating frame
            if (speed == 0) { speed = 150; }

            dstBeing->setWalkSpeed(speed);
            dstBeing->mJob = job;
            dstBeing->setHairStyle(msg.readShort());
            dstBeing->setWeapon(msg.readShort());
            dstBeing->setVisibleEquipment(
                    Being::BOTTOMCLOTHES_SPRITE, msg.readShort());

            if (msg.getId() == SMSG_BEING_MOVE)
            {
                msg.readLong(); // server tick
            }

            msg.readShort();  // shield
            dstBeing->setVisibleEquipment(Being::HAIT_SPRITE, msg.readShort());
            dstBeing->setVisibleEquipment(
                    Being::TOPCLOTHES_SPRITE, msg.readShort());
            dstBeing->setHairColor(msg.readShort());
            msg.readShort();  // unknown
            msg.readShort();  // head dir
            msg.readShort();  // guild
            msg.readShort();  // unknown
            msg.readShort();  // unknown
            msg.readShort();  // manner
            msg.readShort();  // karma
            msg.readByte();   // unknown
            dstBeing->setSex(1 - msg.readByte());   // sex

            if (msg.getId() == SMSG_BEING_MOVE)
            {
                //Uint16 srcX, srcY, dstX, dstY;
                //msg.readCoordinatePair(srcX, srcY, dstX, dstY);
                //dstBeing->setAction(Being::STAND);
                //dstBeing->mX = srcX;
                //dstBeing->mY = srcY;
                //dstBeing->setDestination(dstX, dstY);
            }
            else
            {
                //Uint8 dir;
                //msg->readCoordinates(dstBeing->mX, dstBeing->mY, dir);
                //dstBeing->setDirection(dir);
            }

            msg.readByte();   // unknown
            msg.readByte();   // unknown
            msg.readByte();   // unknown / sit
            break;

        case SMSG_BEING_REMOVE:
            // A being should be removed or has died
            dstBeing = beingManager->findBeing(msg.readLong());

            if (!dstBeing)
                break;

            if (msg.readByte() == 1)
            {
                dstBeing->setAction(Being::DEAD);
            }
            else
            {
                beingManager->destroyBeing(dstBeing);
            }

            if (dstBeing == player_node->getTarget())
            {
                player_node->stopAttack();
            }
            break;

        case SMSG_BEING_ACTION:
            srcBeing = beingManager->findBeing(msg.readLong());
            dstBeing = beingManager->findBeing(msg.readLong());
            msg.readLong();   // server tick
            msg.readLong();   // src speed
            msg.readLong();   // dst speed
            param1 = msg.readShort();
            msg.readShort();  // param 2
            type = msg.readByte();
            msg.readShort();  // param 3

            switch (type)
            {
                case 0: // Damage
                    if (dstBeing) {
                        dstBeing->takeDamage(param1);
                    }
                    if (srcBeing) {
                        srcBeing->handleAttack(dstBeing, param1);
                    }
                    break;

                case 2: // Sit
                    if (srcBeing) {
                        srcBeing->mFrame = 0;
                        srcBeing->setAction(Being::SIT);
                    }
                    break;

                case 3: // Stand up
                    if (srcBeing) {
                        srcBeing->mFrame = 0;
                        srcBeing->setAction(Being::STAND);
                    }
                    break;
            }
            break;

        case SMSG_BEING_LEVELUP:
            if ((Uint32) msg.readLong() == player_node->getId()) {
                logger->log("Level up");
                sound.playSfx("sfx/levelup.ogg");
            } else {
                logger->log("Someone else went level up");
            }
            msg.readLong();  // type
            break;

        case SMSG_BEING_EMOTION:
            if (!(dstBeing = beingManager->findBeing(msg.readLong())))
            {
                break;
            }

            dstBeing->mEmotion = msg.readByte();
            dstBeing->mEmotionTime = EMOTION_TIME;
            break;

        case SMSG_BEING_CHANGE_LOOKS:
        {
            if (!(dstBeing = beingManager->findBeing(msg.readLong())))
            {
                break;
            }

            int type = msg.readByte();
            int id = msg.readByte();

            switch (type) {
                case 1:
                    dstBeing->setHairStyle(id);
                    break;
                case 2:
                    dstBeing->setWeapon(id);
                    break;
                case 3:     // Change lower headgear for eAthena, pants for us
                    dstBeing->setVisibleEquipment(
                            Being::BOTTOMCLOTHES_SPRITE,
                            id);
                    break;
                case 4:     // Change upper headgear for eAthena, hat for us
                    dstBeing->setVisibleEquipment(
                            Being::HAT_SPRITE,
                            id);
                    break;
                case 5:     // Change middle headgear for eathena, armor for us
                     dstBeing->setVisibleEquipment(
                            Being::TOPCLOTHES_SPRITE,
                            id);
                    break;
                case 6:
                    dstBeing->setHairColor(id);
                    break;
                default:
                    logger->log("c3: %i\n", id); // unsupported
                    break;
            }
        }
            break;

        case SMSG_BEING_NAME_RESPONSE:
            if ((dstBeing = beingManager->findBeing(msg.readLong())))
            {
                dstBeing->setName(msg.readString(24));
            }
            break;

        case SMSG_PLAYER_UPDATE_1:
        case SMSG_PLAYER_UPDATE_2:
        case SMSG_PLAYER_MOVE:
            // An update about a player, potentially including movement.
            id = msg.readLong();
            speed = msg.readShort();
            msg.readShort();  // option 1
            msg.readShort();  // option 2
            msg.readShort();  // option
            job = msg.readShort();

            dstBeing = beingManager->findBeing(id);

            if (dstBeing == NULL)
            {
                dstBeing = beingManager->createBeing(id, job);
            }

            dstBeing->setWalkSpeed(speed);
            dstBeing->mJob = job;
            dstBeing->setHairStyle(msg.readShort());
            dstBeing->setWeaponById(msg.readShort());  // item id 1
            msg.readShort();  // item id 2
            headBottom = msg.readShort();

            if (msg.getId() == SMSG_PLAYER_MOVE)
            {
                msg.readLong(); // server tick
            }

            headTop = msg.readShort();
            headMid = msg.readShort();
            dstBeing->setHairColor(msg.readShort());
            msg.readShort();  // unknown
            msg.readShort();  // head dir
            msg.readLong();  // guild
            msg.readLong();  // emblem
            msg.readShort();  // manner
            msg.readByte();   // karma
            dstBeing->setSex(1 - msg.readByte());   // sex
            dstBeing->setVisibleEquipment(
                    Being::BOTTOMCLOTHES_SPRITE, headBottom);
            dstBeing->setVisibleEquipment(Being::HAT_SPRITE, headTop);
            dstBeing->setVisibleEquipment(Being::TOPCLOTHES_SPRITE, headMid);

            if (msg.getId() == SMSG_PLAYER_MOVE)
            {
                //Uint16 srcX, srcY, dstX, dstY;
                //msg.readCoordinatePair(srcX, srcY, dstX, dstY);
                //dstBeing->mX = srcX;
                //dstBeing->mY = srcY;
                //dstBeing->setDestination(dstX, dstY);
            }
            else
            {
                //Uint8 dir;
                //msg->readCoordinates(dstBeing->mX, dstBeing->mY, dir);
                //dstBeing->setDirection(dir);
            }

            msg.readByte();   // unknown
            msg.readByte();   // unknown

            if (msg.getId() == SMSG_PLAYER_UPDATE_1)
            {
                if (msg.readByte() == 2)
                {
                    dstBeing->setAction(Being::SIT);
                }
            }
            else if (msg.getId() == SMSG_PLAYER_MOVE)
            {
                msg.readByte(); // unknown
            }

            msg.readByte();   // Lv
            msg.readByte();   // unknown

            dstBeing->mWalkTime = tick_time;
            dstBeing->mFrame = 0;
            break;

        case 0x0119:
            // Change in players look
            logger->log("0x0119 %li %i %i %x %i\n", msg.readLong(),
                   msg.readShort(), msg.readShort(), msg.readShort(),
                   msg.readByte());
            break;
        */
    }
}

void
BeingHandler::handleBeingEnterMessage(MessageIn &msg)
{
    int type = msg.readByte(); // type
    int id = msg.readShort();
    Being::Action action = (Being::Action)msg.readByte();
    Uint16 px = msg.readShort();
    Uint16 py = msg.readShort();

    switch (type)
    {
        case OBJECT_PLAYER:
        {
            std::string name = msg.readString();
            Being *being;
            if (player_node->getName() == name)
            {
                being = player_node;
                being->setId(id);
            }
            else
            {
                being = beingManager->createBeing(id, 0);
                being->setName(name);
            }
            being->setHairStyle(msg.readByte());
            being->setHairColor(msg.readByte());
            being->setSex(msg.readByte());
            being->mX = px;
            being->mY = py;
            being->setDestination(px, py);
            being->setAction(action);
        } break;
        case OBJECT_MONSTER:
        {
            int monsterId = msg.readShort();
            Being *being;
            being = beingManager->createBeing(id, 1002 + monsterId);
            being->setWalkSpeed(150); // TODO
            being->mX = px;
            being->mY = py;
            being->setDestination(px, py);
            being->setAction(action);
        } break;
    }
}

void BeingHandler::handleBeingLeaveMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readShort());
    if (!being) return;

    beingManager->destroyBeing(being);
}

void BeingHandler::handleBeingsMoveMessage(MessageIn &msg)
{
    while (msg.getUnreadLength())
    {
        Uint16 id = msg.readShort();
        Uint8 flags = msg.readByte();
        Being *being = beingManager->findBeing(id);
        int sx = 0, sy = 0, dx = 0, dy = 0;
        if (flags & MOVING_POSITION)
        {
            Uint16 sx2, sy2;
            msg.readCoordinates(sx2, sy2);
            sx = sx2 * 32 + 16;
            sy = sy2 * 32 + 16;
        }
        if (flags & MOVING_DESTINATION)
        {
            dx = msg.readShort();
            dy = msg.readShort();
            if (!(flags & MOVING_POSITION))
            {
                sx = dx;
                sy = dy;
            }
        }
        if (!being || !(flags & (MOVING_POSITION | MOVING_DESTINATION)))
        {
            continue;
        }
        if (abs(being->mX - sx) + abs(being->mY - sy) > 4 * 32)
        {
            // Too large a desynchronization.
            being->mX = sx;
            being->mY = sy;
            being->setDestination(dx, dy);
        }
        else if (!(flags & MOVING_POSITION))
        {
            being->setDestination(dx, dy);
        }
        else if (!(flags & MOVING_DESTINATION))
        {
            being->adjustCourse(sx, sy);
        }
        else
        {
            being->adjustCourse(sx, sy, dx, dy);
        }
    }
}

void BeingHandler::handleBeingAttackMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readShort());
    if (!being) return;

    being->setAction(Being::ATTACK);
}

void BeingHandler::handleBeingsDamageMessage(MessageIn &msg)
{
    while (msg.getUnreadLength())
    {
        Being *being = beingManager->findBeing(msg.readShort());
        int damage = msg.readShort();
        if (being)
        {
            being->takeDamage(damage);
        }
    }
}

void BeingHandler::handleBeingActionChangeMessage(MessageIn &msg)
{
    Being* being = beingManager->findBeing(msg.readShort());
    if (!being) return;

    being->setAction((Being::Action) msg.readByte());
}
