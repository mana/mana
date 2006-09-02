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
        GPMSG_BEING_ENTER,
        GPMSG_BEING_LEAVE,
        GPMSG_BEINGS_MOVE,
        0
    };
    handledMessages = _messages;
}

void BeingHandler::handleMessage(MessageIn &msg)
{
    /*
    Uint32 id;
    Uint16 job, speed;
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
            dstBeing->setVisibleEquipment(3, msg.readShort()); // head bottom

            if (msg.getId() == SMSG_BEING_MOVE)
            {
                msg.readLong(); // server tick
            }

            msg.readShort();  // shield
            dstBeing->setVisibleEquipment(4, msg.readShort()); // head top
            dstBeing->setVisibleEquipment(5, msg.readShort()); // head mid
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
                //msg.readCoordinates(dstBeing->mX, dstBeing->mY, dstBeing->mDirection);
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
                // Death
                switch (dstBeing->getType())
                {
                    case Being::MONSTER:
                        dstBeing->setAction(Being::MONSTER_DEAD);
                        dstBeing->mFrame = 0;
                        dstBeing->mWalkTime = tick_time;
                        break;

                    default:
                        dstBeing->setAction(Being::DEAD);
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
                    if (dstBeing == NULL) break;

                    dstBeing->setDamage(param1, SPEECH_TIME);

                    if (srcBeing != NULL &&
                            srcBeing != player_node)
                    {
                        // buggy
                        if (srcBeing->getType() == Being::MONSTER)
                        {
                            srcBeing->setAction(Being::MONSTER_ATTACK);
                        }
                        else
                        {
                            srcBeing->setAction(Being::ATTACK);
                        }
                        srcBeing->mFrame = 0;
                        srcBeing->mWalkTime = tick_time;
                    }
                    break;

                case 2: // Sit
                    if (srcBeing == NULL) break;
                    srcBeing->mFrame = 0;
                    srcBeing->setAction(Being::SIT);
                    break;

                case 3: // Stand up
                    if (srcBeing == NULL) break;
                    srcBeing->mFrame = 0;
                    srcBeing->setAction(Being::STAND);
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

            switch (type) {
                case 1:
                    dstBeing->setHairStyle(msg.readByte());
                    break;
                case 2:
                    dstBeing->setWeapon(msg.readByte());
                    break;
                case 3:
                case 4:
                case 5:
                    // Equip/unequip head 3. Bottom 4. Top 5. Middle
                    dstBeing->setVisibleEquipment(type, msg.readByte());
                    // First 3 slots  of mVisibleEquipments are reserved for
                    // later use, probably accessories.
                    break;
                case 6:
                    dstBeing->setHairColor(msg.readByte());
                    break;
                default:
                    printf("c3: %i\n", msg.readByte()); // unsupported
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
            dstBeing->setVisibleEquipment(3, msg.readShort()); // head bottom

            if (msg.getId() == SMSG_PLAYER_MOVE)
            {
                msg.readLong(); // server tick
            }

            dstBeing->setVisibleEquipment(4, msg.readShort()); // head top
            dstBeing->setVisibleEquipment(5, msg.readShort()); // head mid
            dstBeing->setHairColor(msg.readShort());
            msg.readShort();  // unknown
            msg.readShort();  // head dir
            msg.readLong();  // guild
            msg.readLong();  // emblem
            msg.readShort();  // manner
            msg.readByte();   // karma
            dstBeing->setSex(1 - msg.readByte());   // sex

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
                //msg.readCoordinates(dstBeing->mX, dstBeing->mY, dstBeing->mDirection);
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
            printf("0x0119 %li %i %i %x %i\n", msg.readLong(),
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

    switch (type) {
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
    } break;
    case OBJECT_MONSTER:
    {
        int monsterId = msg.readShort();
        Being *being;
        being = beingManager->createBeing(id, 1002 + monsterId);
        being->setWalkSpeed(150); // TODO
    } break;
    }
}

void BeingHandler::handleBeingLeaveMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readShort());
    if (!being) return;
    if (being == player_node->getTarget())
    {
        player_node->stopAttack();
    }
    beingManager->destroyBeing(being);
}

void BeingHandler::handleBeingsMoveMessage(MessageIn &msg)
{
    for (int nb = (msg.getLength() - 2) / (2 + 4 * 2); nb > 0; --nb)
    {
        Uint16 id = msg.readShort();
        Being *being = beingManager->findBeing(id);
        if (!being) continue;
        int sx = msg.readShort(), sy = msg.readShort(),
            dx = msg.readShort(), dy = msg.readShort();
        bool update = being != player_node; // the local player already knows where he wants to go
        if (abs(being->mX - sx) + abs(being->mY - sy) > 4 * 32)
        {
            // crude handling of synchronization messages
            being->mX = sx;
            being->mY = sy;
            update = true;
        }
        if (update)
        {
            being->setDestination(dx, dy);
        }
    }
}
