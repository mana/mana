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
        SMSG_BEING_VISIBLE,
        SMSG_BEING_MOVE,
        SMSG_BEING_REMOVE,
        SMSG_BEING_ACTION,
        SMSG_BEING_LEVELUP,
        SMSG_BEING_EMOTION,
        SMSG_BEING_CHANGE_LOOKS,
        SMSG_BEING_NAME_RESPONSE,
        SMSG_PLAYER_UPDATE_1,
        SMSG_PLAYER_UPDATE_2,
        SMSG_PLAYER_MOVE,
        0x0119,
        0
    };
    handledMessages = _messages;
}

void BeingHandler::handleMessage(MessageIn *msg)
{
    Uint32 id;
    Uint16 job, speed;
    Uint16 headBottom, headTop, headMid;
    Sint16 param1;
    Sint8 type;
    Being *srcBeing, *dstBeing;

    switch (msg->getId())
    {
        case SMSG_BEING_VISIBLE:
        case SMSG_BEING_MOVE:
            // Information about a being in range
            id = msg->readInt32();
            speed = msg->readInt16();
            msg->readInt16();  // unknown
            msg->readInt16();  // unknown
            msg->readInt16();  // option
            job = msg->readInt16();  // class

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
            else if (msg->getId() == 0x0078)
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
            dstBeing->setHairStyle(msg->readInt16());
            dstBeing->setWeapon(msg->readInt16());
            dstBeing->setVisibleEquipment(
                    Being::BOTTOMCLOTHES_SPRITE, msg->readInt16());

            if (msg->getId() == SMSG_BEING_MOVE)
            {
                msg->readInt32(); // server tick
            }

            msg->readInt16();  // shield
            dstBeing->setVisibleEquipment(Being::HAT_SPRITE, msg->readInt16());
            dstBeing->setVisibleEquipment(
                    Being::TOPCLOTHES_SPRITE, msg->readInt16());
            dstBeing->setHairColor(msg->readInt16());
            msg->readInt16();  // unknown
            msg->readInt16();  // head dir
            msg->readInt16();  // guild
            msg->readInt16();  // unknown
            msg->readInt16();  // unknown
            msg->readInt16();  // manner
            msg->readInt16();  // karma
            msg->readInt8();   // unknown
            dstBeing->setSex(1 - msg->readInt8());   // sex

            if (msg->getId() == SMSG_BEING_MOVE)
            {
                Uint16 srcX, srcY, dstX, dstY;
                msg->readCoordinatePair(srcX, srcY, dstX, dstY);
                dstBeing->setAction(Being::STAND);
                dstBeing->mX = srcX;
                dstBeing->mY = srcY;
                dstBeing->setDestination(dstX, dstY);
            }
            else
            {
                Uint8 dir;
                msg->readCoordinates(dstBeing->mX, dstBeing->mY, dir);
                dstBeing->setDirection(dir);
            }

            msg->readInt8();   // unknown
            msg->readInt8();   // unknown
            msg->readInt8();   // unknown / sit
            break;

        case SMSG_BEING_REMOVE:
            // A being should be removed or has died
            dstBeing = beingManager->findBeing(msg->readInt32());

            if (!dstBeing)
                break;

            if (msg->readInt8() == 1)
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
            srcBeing = beingManager->findBeing(msg->readInt32());
            dstBeing = beingManager->findBeing(msg->readInt32());
            msg->readInt32();   // server tick
            msg->readInt32();   // src speed
            msg->readInt32();   // dst speed
            param1 = msg->readInt16();
            msg->readInt16();  // param 2
            type = msg->readInt8();
            msg->readInt16();  // param 3

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
            if ((Uint32) msg->readInt32() == player_node->getId()) {
                logger->log("Level up");
                sound.playSfx("sfx/levelup.ogg");
            } else {
                logger->log("Someone else went level up");
            }
            msg->readInt32();  // type
            break;

        case SMSG_BEING_EMOTION:
            if (!(dstBeing = beingManager->findBeing(msg->readInt32())))
            {
                break;
            }

            dstBeing->mEmotion = msg->readInt8();
            dstBeing->mEmotionTime = EMOTION_TIME;
            break;

        case SMSG_BEING_CHANGE_LOOKS:
        {
            if (!(dstBeing = beingManager->findBeing(msg->readInt32())))
            {
                break;
            }

            int type = msg->readInt8();
            int id = msg->readInt8();

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
            if ((dstBeing = beingManager->findBeing(msg->readInt32())))
            {
                dstBeing->setName(msg->readString(24));
            }
            break;

        case SMSG_PLAYER_UPDATE_1:
        case SMSG_PLAYER_UPDATE_2:
        case SMSG_PLAYER_MOVE:
            // An update about a player, potentially including movement.
            id = msg->readInt32();
            speed = msg->readInt16();
            msg->readInt16();  // option 1
            msg->readInt16();  // option 2
            msg->readInt16();  // option
            job = msg->readInt16();

            dstBeing = beingManager->findBeing(id);

            if (dstBeing == NULL)
            {
                dstBeing = beingManager->createBeing(id, job);
            }

            dstBeing->setWalkSpeed(speed);
            dstBeing->mJob = job;
            dstBeing->setHairStyle(msg->readInt16());
            dstBeing->setWeaponById(msg->readInt16());  // item id 1
            msg->readInt16();  // item id 2
            headBottom = msg->readInt16();

            if (msg->getId() == SMSG_PLAYER_MOVE)
            {
                msg->readInt32(); // server tick
            }

            headTop = msg->readInt16();
            headMid = msg->readInt16();
            dstBeing->setHairColor(msg->readInt16());
            msg->readInt16();  // unknown
            msg->readInt16();  // head dir
            msg->readInt32();  // guild
            msg->readInt32();  // emblem
            msg->readInt16();  // manner
            msg->readInt8();   // karma
            dstBeing->setSex(1 - msg->readInt8());   // sex
            dstBeing->setVisibleEquipment(
                    Being::BOTTOMCLOTHES_SPRITE, headBottom);
            dstBeing->setVisibleEquipment(Being::HAT_SPRITE, headTop);
            dstBeing->setVisibleEquipment(Being::TOPCLOTHES_SPRITE, headMid);

            if (msg->getId() == SMSG_PLAYER_MOVE)
            {
                Uint16 srcX, srcY, dstX, dstY;
                msg->readCoordinatePair(srcX, srcY, dstX, dstY);
                dstBeing->mX = srcX;
                dstBeing->mY = srcY;
                dstBeing->setDestination(dstX, dstY);
            }
            else
            {
                Uint8 dir;
                msg->readCoordinates(dstBeing->mX, dstBeing->mY, dir);
                dstBeing->setDirection(dir);
            }

            msg->readInt8();   // unknown
            msg->readInt8();   // unknown

            if (msg->getId() == SMSG_PLAYER_UPDATE_1)
            {
                if (msg->readInt8() == 2)
                {
                    dstBeing->setAction(Being::SIT);
                }
            }
            else if (msg->getId() == SMSG_PLAYER_MOVE)
            {
                msg->readInt8(); // unknown
            }

            msg->readInt8();   // Lv
            msg->readInt8();   // unknown

            dstBeing->mWalkTime = tick_time;
            dstBeing->mFrame = 0;
            break;

        case 0x0119:
            // Change in players look
            logger->log("0x0119 %i %i %i %x %i\n", msg->readInt32(),
                    msg->readInt16(), msg->readInt16(), msg->readInt16(),
                    msg->readInt8());
            break;
    }
}
