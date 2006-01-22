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
                dstBeing->walk_time = tick_time;
                dstBeing->action = Being::STAND;
            }

            // Prevent division by 0 when calculating frame
            if (speed == 0) { speed = 150; }

            dstBeing->setWalkSpeed(speed);
            dstBeing->job = job;
            dstBeing->setHairStyle(msg->readInt16());
            dstBeing->setWeapon(msg->readInt16());
            msg->readInt16();  // head option bottom

            if (msg->getId() == SMSG_BEING_MOVE)
            {
                msg->readInt32(); // server tick
            }

            msg->readInt16();  // shield
            msg->readInt16();  // head option top
            msg->readInt16();  // head option mid
            dstBeing->setHairColor(msg->readInt16());
            msg->readInt16();  // unknown
            msg->readInt16();  // head dir
            msg->readInt16();  // guild
            msg->readInt16();  // unknown
            msg->readInt16();  // unknown
            msg->readInt16();  // manner
            msg->readInt16();  // karma
            msg->readInt8();   // unknown
            msg->readInt8();   // sex

            if (msg->getId() == SMSG_BEING_MOVE)
            {
                Uint16 srcX, srcY, dstX, dstY;
                msg->readCoordinatePair(srcX, srcY, dstX, dstY);
                dstBeing->action = Being::STAND;
                dstBeing->x = srcX;
                dstBeing->y = srcY;
                dstBeing->setDestination(dstX, dstY);
            }
            else
            {
                msg->readCoordinates(dstBeing->x, dstBeing->y,
                        dstBeing->direction);
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
                // Death
                switch (dstBeing->getType())
                {
                    case Being::MONSTER:
                        dstBeing->action = Being::MONSTER_DEAD;
                        dstBeing->mFrame = 0;
                        dstBeing->walk_time = tick_time;
                        break;

                    default:
                        dstBeing->action = Being::DEAD;
                        break;
                }
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
                    if (dstBeing == NULL) break;

                    dstBeing->setDamage(param1, SPEECH_TIME);

                    if (srcBeing != NULL &&
                            srcBeing != player_node)
                    {
                        // buggy
                        srcBeing->action = Being::ATTACK;
                        srcBeing->mFrame = 0;
                        srcBeing->walk_time = tick_time;
                    }
                    break;

                case 2: // Sit
                    if (srcBeing == NULL) break;
                    srcBeing->mFrame = 0;
                    srcBeing->action = Being::SIT;
                    break;

                case 3: // Stand up
                    if (srcBeing == NULL) break;
                    srcBeing->mFrame = 0;
                    srcBeing->action = Being::STAND;
                    break;
            }
            break;

        case SMSG_BEING_LEVELUP:
            if ((Uint32)msg->readInt32() == player_node->getId()) {
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

            dstBeing->emotion = msg->readInt8();
            dstBeing->emotion_time = EMOTION_TIME;
            break;

        case SMSG_BEING_CHANGE_LOOKS:
            if (!(dstBeing = beingManager->findBeing(msg->readInt32())))
            {
                break;
            }

            switch (msg->readInt8()) {
                case 1:
                    dstBeing->setHairStyle(msg->readInt8());
                    break;
                case 2:
                    dstBeing->setWeapon(msg->readInt8());
                    break;
                case 6:
                    dstBeing->setHairColor(msg->readInt8());
                    break;
                default:
                    msg->readInt8(); // unsupported
                    break;
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
            dstBeing->job = job;
            dstBeing->setHairStyle(msg->readInt16());
            dstBeing->setWeaponById(msg->readInt16());  // item id 1
            msg->readInt16();  // item id 2
            msg->readInt16();  // head option bottom

            if (msg->getId() == SMSG_PLAYER_MOVE)
            {
                msg->readInt32(); // server tick
            }

            msg->readInt16();  // head option top
            msg->readInt16();  // head option mid
            dstBeing->setHairColor(msg->readInt16());
            msg->readInt16();  // unknown
            msg->readInt16();  // head dir
            msg->readInt32();   // guild
            msg->readInt32();   // emblem
            msg->readInt16();  // manner
            msg->readInt8();   // karma
            msg->readInt8();   // sex

            if (msg->getId() == SMSG_PLAYER_MOVE)
            {
                Uint16 srcX, srcY, dstX, dstY;
                msg->readCoordinatePair(srcX, srcY, dstX, dstY);
                dstBeing->x = srcX;
                dstBeing->y = srcY;
                dstBeing->setDestination(dstX, dstY);
            }
            else
            {
                msg->readCoordinates(dstBeing->x, dstBeing->y,
                        dstBeing->direction);
            }

            msg->readInt8();   // unknown
            msg->readInt8();   // unknown

            if (msg->getId() == SMSG_PLAYER_UPDATE_1)
            {
                if (msg->readInt8() == 2)
                {
                    dstBeing->action = Being::SIT;
                }
            }
            else if (msg->getId() == SMSG_PLAYER_MOVE)
            {
                msg->readInt8(); // unknown
            }

            msg->readInt8();   // Lv
            msg->readInt8();   // unknown

            dstBeing->walk_time = tick_time;
            dstBeing->mFrame = 0;
            break;

        case 0x0119:
            // Change in players look
            break;
    }
}
