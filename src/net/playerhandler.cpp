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

#include "playerhandler.h"

#include "messagein.h"
#include "protocol.h"

#include "../engine.h"
#include "../localplayer.h"
#include "../log.h"
#include "../npc.h"

#include "../gui/chat.h"
#include "../gui/ok_dialog.h"
#include "../gui/skill.h"

// TODO Move somewhere else
OkDialog *weightNotice = NULL;
OkDialog *deathNotice = NULL;

/**
 * Listener used for handling the overweigth message.
 */
// TODO Move somewhere else
class WeightNoticeListener : public gcn::ActionListener
{
    public:
        void action(const std::string &eventId)
        {
            weightNotice = NULL;
        }
} weightNoticeListener;


/**
 * Listener used for handling death message.
 */
// TODO Move somewhere else
class DeathNoticeListener : public gcn::ActionListener {
    public:
        void action(const std::string &eventId) {
            player_node->revive();
            deathNotice = NULL;
        }
} deathNoticeListener;

PlayerHandler::PlayerHandler()
{
    static const Uint16 _messages[] = {
        SMSG_WALK_RESPONSE,
        SMSG_PLAYER_WARP,
        SMSG_PLAYER_STAT_UPDATE_1,
        SMSG_PLAYER_STAT_UPDATE_2,
        SMSG_PLAYER_STAT_UPDATE_3,
        SMSG_PLAYER_STAT_UPDATE_4,
        SMSG_PLAYER_STAT_UPDATE_5,
        SMSG_PLAYER_STAT_UPDATE_6,
        SMSG_PLAYER_ARROW_MESSAGE,
        0
    };
    handledMessages = _messages;
}

void PlayerHandler::handleMessage(MessageIn *msg)
{
    switch (msg->getId())
    {
        case SMSG_WALK_RESPONSE:
            // It is assumed by the client any request to walk actually
            // succeeds on the server. The plan is to have a correction
            // message when the server senses the client has the wrong
            // idea.
            break;

        case SMSG_PLAYER_WARP:
            {
                std::string mapPath = msg->readString(16);
                Uint16 x = msg->readInt16();
                Uint16 y = msg->readInt16();

                logger->log("Warping to %s (%d, %d)", mapPath.c_str(), x, y);

                // Switch the actual map, deleting the previous one
                engine->changeMap(mapPath);

                current_npc = 0;

                player_node->action = Being::STAND;
                player_node->stopAttack();
                player_node->mFrame = 0;
                player_node->x = x;
                player_node->y = y;
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_1:
            {
                Sint16 type = msg->readInt16();
                Uint32 value = msg->readInt32();

                switch (type)
                {
                    //case 0x0000:
                    //    player_node->setWalkSpeed(msg->readInt32());
                    //    break;
                    case 0x0005: player_node->hp = value; break;
                    case 0x0006: player_node->maxHp = value; break;
                    case 0x0007: player_node->mp = value; break;
                    case 0x0008: player_node->maxMp = value; break;
                    case 0x000b: player_node->lvl = value; break;
                    case 0x000c:
                                 player_node->skillPoint = value;
                                 skillDialog->update();
                                 break;
                    case 0x0018:
                                 if (value >= player_node->maxWeight / 2 &&
                                         player_node->totalWeight <
                                         player_node->maxWeight / 2)
                                 {
                                     weightNotice = new OkDialog("Message",
                                             "You are carrying more then half your "
                                             "weight. You are unable to regain "
                                             "health.");
                                     weightNotice->addActionListener(&weightNoticeListener);
                                 }
                                 player_node->totalWeight = value;
                                 break;
                    case 0x0019: player_node->maxWeight = value; break;
                    case 0x0037: player_node->jobLvl = value; break;
                    case 0x0009:
                                 player_node->statsPointsToAttribute = value;
                                 break;
                    case 0x0029: player_node->ATK = value; break;
                    case 0x002b: player_node->MATK = value; break;
                    case 0x002d: player_node->DEF = value; break;
                    case 0x002f: player_node->MDEF = value; break;
                    case 0x0031: player_node->HIT = value; break;
                    case 0x0032: player_node->FLEE = value; break;
                    case 0x0035: player_node->aspd = value; break;
                }

                if (player_node->hp == 0 && deathNotice == NULL)
                {
                    deathNotice = new OkDialog("Message",
                            "You're now dead, press ok to restart");
                    deathNotice->addActionListener(&deathNoticeListener);
                    player_node->action = Being::DEAD;
                }
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_2:
            switch (msg->readInt16()) {
                case 0x0001:
                    player_node->xp = msg->readInt32();
                    break;
                case 0x0002:
                    player_node->jobXp = msg->readInt32();
                    break;
                case 0x0014:
                    player_node->gp = msg->readInt32();
                    break;
                case 0x0016:
                    player_node->xpForNextLevel = msg->readInt32();
                    break;
                case 0x0017:
                    player_node->jobXpForNextLevel = msg->readInt32();
                    break;
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_3:
            {
                Sint32 type = msg->readInt32();
                Sint32 base = msg->readInt32();
                Sint32 bonus = msg->readInt32();
                Sint32 total = base + bonus;

                switch (type) {
                    case 0x000d: player_node->ATTR[LocalPlayer::STR] = total; break;
                    case 0x000e: player_node->ATTR[LocalPlayer::AGI] = total; break;
                    case 0x000f: player_node->ATTR[LocalPlayer::VIT] = total; break;
                    case 0x0010: player_node->ATTR[LocalPlayer::INT] = total; break;
                    case 0x0011: player_node->ATTR[LocalPlayer::DEX] = total; break;
                    case 0x0012: player_node->ATTR[LocalPlayer::LUK] = total; break;
                }
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_4:
            {
                Sint16 type = msg->readInt16();
                Sint8 fail = msg->readInt8();
                Sint8 value = msg->readInt8();

                if (fail == 1)
                {
                    switch (type) {
                        case 0x000d: player_node->ATTR[LocalPlayer::STR] = value; break;
                        case 0x000e: player_node->ATTR[LocalPlayer::AGI] = value; break;
                        case 0x000f: player_node->ATTR[LocalPlayer::VIT] = value; break;
                        case 0x0010: player_node->ATTR[LocalPlayer::INT] = value; break;
                        case 0x0011: player_node->ATTR[LocalPlayer::DEX] = value; break;
                        case 0x0012: player_node->ATTR[LocalPlayer::LUK] = value; break;
                    }
                }
            }
            break;

            // Updates stats and status points
        case SMSG_PLAYER_STAT_UPDATE_5:
            player_node->statsPointsToAttribute = msg->readInt16();
            player_node->ATTR[LocalPlayer::STR] = msg->readInt8();
            player_node->ATTR_UP[LocalPlayer::STR] = msg->readInt8();
            player_node->ATTR[LocalPlayer::AGI] = msg->readInt8();
            player_node->ATTR_UP[LocalPlayer::AGI] = msg->readInt8();
            player_node->ATTR[LocalPlayer::VIT] = msg->readInt8();
            player_node->ATTR_UP[LocalPlayer::VIT] = msg->readInt8();
            player_node->ATTR[LocalPlayer::INT] = msg->readInt8();
            player_node->ATTR_UP[LocalPlayer::INT] = msg->readInt8();
            player_node->ATTR[LocalPlayer::DEX] = msg->readInt8();
            player_node->ATTR_UP[LocalPlayer::DEX] = msg->readInt8();
            player_node->ATTR[LocalPlayer::LUK] = msg->readInt8();
            player_node->ATTR_UP[LocalPlayer::LUK] = msg->readInt8();
            player_node->ATK       = msg->readInt16();  // ATK
            player_node->ATK_BONUS  = msg->readInt16();  // ATK bonus
            player_node->MATK      = msg->readInt16();  // MATK max
            player_node->MATK_BONUS = msg->readInt16();  // MATK min
            player_node->DEF       = msg->readInt16();  // DEF
            player_node->DEF_BONUS  = msg->readInt16();  // DEF bonus
            player_node->MDEF      = msg->readInt16();  // MDEF
            player_node->MDEF_BONUS = msg->readInt16();  // MDEF bonus
            player_node->HIT       = msg->readInt16();  // HIT
            player_node->FLEE      = msg->readInt16();  // FLEE
            player_node->FLEE_BONUS = msg->readInt16();  // FLEE bonus
            msg->readInt16();  // critical
            msg->readInt16();  // unknown
            break;

        case SMSG_PLAYER_STAT_UPDATE_6:
            switch (msg->readInt16()) {
                case 0x0020: player_node->ATTR_UP[LocalPlayer::STR] = msg->readInt8(); break;
                case 0x0021: player_node->ATTR_UP[LocalPlayer::AGI] = msg->readInt8(); break;
                case 0x0022: player_node->ATTR_UP[LocalPlayer::VIT] = msg->readInt8(); break;
                case 0x0023: player_node->ATTR_UP[LocalPlayer::INT] = msg->readInt8(); break;
                case 0x0024: player_node->ATTR_UP[LocalPlayer::DEX] = msg->readInt8(); break;
                case 0x0025: player_node->ATTR_UP[LocalPlayer::LUK] = msg->readInt8(); break;
            }
            break;

        case SMSG_PLAYER_ARROW_MESSAGE:
            {
                Sint16 type = msg->readInt16();

                switch (type) {
                    case 0:
                        chatWindow->chatLog("Equip arrows first",
                                             BY_SERVER);
                        break;
                    default:
                        logger->log("0x013b: Unhandled message %i", type);
                        break;
                }
            }
            break;

        //Stop walking
        //case 0x0088:  // Disabled because giving some problems
        //if (being = beingManager->findBeing(readInt32(2))) {
        //    if (being->getId() != player_node->getId()) {
        //        being->action = STAND;
        //        being->mFrame = 0;
        //        set_coordinates(being->coordinates,
        //                        readWord(6), readWord(8),
        //                        get_direction(being->coordinates));
        //    }
        //}
        //break;
    }
}
