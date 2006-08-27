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
namespace {
    struct WeightListener : public gcn::ActionListener
    {
        void action(const std::string &eventId, gcn::Widget *widget) {
            weightNotice = NULL;
        }
    } weightListener;
}

/**
 * Listener used for handling death message.
 */
// TODO Move somewhere else
namespace {
    struct DeathListener : public gcn::ActionListener {
        void action(const std::string &eventId, gcn::Widget *widget) {
            player_node->revive();
            deathNotice = NULL;
        }
    } deathListener;
}

PlayerHandler::PlayerHandler()
{
    static const Uint16 _messages[] = {
        //SMSG_PLAYER_STAT_UPDATE_1,
        //SMSG_PLAYER_STAT_UPDATE_2,
        //SMSG_PLAYER_STAT_UPDATE_3,
        //SMSG_PLAYER_STAT_UPDATE_4,
        //SMSG_PLAYER_STAT_UPDATE_5,
        //SMSG_PLAYER_STAT_UPDATE_6,
        //SMSG_PLAYER_ARROW_MESSAGE,
        GPMSG_PLAYER_MAP_CHANGE,
        0
    };
    handledMessages = _messages;
}

void PlayerHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_PLAYER_MAP_CHANGE:
            handleMapChangeMessage(msg);
            break;

        case SMSG_PLAYER_STAT_UPDATE_1:
            {
                Sint16 type = msg.readShort();
                Uint32 value = msg.readLong();

                switch (type)
                {
                    //case 0x0000:
                    //    player_node->setWalkSpeed(msg.readLong());
                    //    break;
                    case 0x0005: player_node->mHp = value; break;
                    case 0x0006: player_node->mMaxHp = value; break;
                    case 0x0007: player_node->mMp = value; break;
                    case 0x0008: player_node->mMaxMp = value; break;
                    case 0x000b: player_node->mLevel = value; break;
                    case 0x000c:
                                 player_node->mSkillPoint = value;
                                 skillDialog->update();
                                 break;
                    case 0x0018:
                                 if (value >= player_node->mMaxWeight / 2 &&
                                         player_node->mTotalWeight <
                                         player_node->mMaxWeight / 2)
                                 {
                                     weightNotice = new OkDialog("Message",
                                             "You are carrying more then half your "
                                             "weight. You are unable to regain "
                                             "health.");
                                     weightNotice->addActionListener(&weightListener);
                                 }
                                 player_node->mTotalWeight = value;
                                 break;
                    case 0x0019: player_node->mMaxWeight = value; break;
                    case 0x0037: player_node->mJobLevel = value; break;
                    case 0x0009:
                                 player_node->mStatsPointsToAttribute = value;
                                 break;
                    case 0x0029: player_node->ATK = value; break;
                    case 0x002b: player_node->MATK = value; break;
                    case 0x002d: player_node->DEF = value; break;
                    case 0x002f: player_node->MDEF = value; break;
                    case 0x0031: player_node->HIT = value; break;
                    case 0x0032: player_node->FLEE = value; break;
                    case 0x0035: player_node->mAttackSpeed = value; break;
                }

                if (player_node->mHp == 0 && deathNotice == NULL)
                {
                    deathNotice = new OkDialog("Message",
                            "You're now dead, press ok to restart");
                    deathNotice->addActionListener(&deathListener);
                    player_node->setAction(Being::DEAD);
                }
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_2:
            switch (msg.readShort()) {
                case 0x0001:
                    player_node->mXp = msg.readLong();
                    break;
                case 0x0002:
                    player_node->mJobXp = msg.readLong();
                    break;
                case 0x0014:
                    player_node->mMoney = msg.readLong();
                    break;
                case 0x0016:
                    player_node->mXpForNextLevel = msg.readLong();
                    break;
                case 0x0017:
                    player_node->mJobXpForNextLevel = msg.readLong();
                    break;
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_3:
            {
                Sint32 type = msg.readLong();
                Sint32 base = msg.readLong();
                Sint32 bonus = msg.readLong();
                Sint32 total = base + bonus;

                switch (type) {
                    case 0x000d: player_node->mAttr[LocalPlayer::STR] = total;
                                 break;
                    case 0x000e: player_node->mAttr[LocalPlayer::AGI] = total;
                                 break;
                    case 0x000f: player_node->mAttr[LocalPlayer::VIT] = total;
                                 break;
                    case 0x0010: player_node->mAttr[LocalPlayer::INT] = total;
                                 break;
                    case 0x0011: player_node->mAttr[LocalPlayer::DEX] = total;
                                 break;
                    case 0x0012: player_node->mAttr[LocalPlayer::LUK] = total;
                                 break;
                }
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_4:
            {
                Sint16 type = msg.readShort();
                Sint8 fail = msg.readByte();
                Sint8 value = msg.readByte();

                if (fail != 1)
                    break;

                switch (type) {
                    case 0x000d: player_node->mAttr[LocalPlayer::STR] = value;
                                 break;
                    case 0x000e: player_node->mAttr[LocalPlayer::AGI] = value;
                                 break;
                    case 0x000f: player_node->mAttr[LocalPlayer::VIT] = value;
                                 break;
                    case 0x0010: player_node->mAttr[LocalPlayer::INT] = value;
                                 break;
                    case 0x0011: player_node->mAttr[LocalPlayer::DEX] = value;
                                 break;
                    case 0x0012: player_node->mAttr[LocalPlayer::LUK] = value;
                                 break;
                }
            }
            break;

            // Updates stats and status points
        case SMSG_PLAYER_STAT_UPDATE_5:
            player_node->mStatsPointsToAttribute = msg.readShort();
            player_node->mAttr[LocalPlayer::STR] = msg.readByte();
            player_node->mAttrUp[LocalPlayer::STR] = msg.readByte();
            player_node->mAttr[LocalPlayer::AGI] = msg.readByte();
            player_node->mAttrUp[LocalPlayer::AGI] = msg.readByte();
            player_node->mAttr[LocalPlayer::VIT] = msg.readByte();
            player_node->mAttrUp[LocalPlayer::VIT] = msg.readByte();
            player_node->mAttr[LocalPlayer::INT] = msg.readByte();
            player_node->mAttrUp[LocalPlayer::INT] = msg.readByte();
            player_node->mAttr[LocalPlayer::DEX] = msg.readByte();
            player_node->mAttrUp[LocalPlayer::DEX] = msg.readByte();
            player_node->mAttr[LocalPlayer::LUK] = msg.readByte();
            player_node->mAttrUp[LocalPlayer::LUK] = msg.readByte();
            player_node->ATK       = msg.readShort();  // ATK
            player_node->ATK_BONUS  = msg.readShort();  // ATK bonus
            player_node->MATK      = msg.readShort();  // MATK max
            player_node->MATK_BONUS = msg.readShort();  // MATK min
            player_node->DEF       = msg.readShort();  // DEF
            player_node->DEF_BONUS  = msg.readShort();  // DEF bonus
            player_node->MDEF      = msg.readShort();  // MDEF
            player_node->MDEF_BONUS = msg.readShort();  // MDEF bonus
            player_node->HIT       = msg.readShort();  // HIT
            player_node->FLEE      = msg.readShort();  // FLEE
            player_node->FLEE_BONUS = msg.readShort();  // FLEE bonus
            msg.readShort();  // critical
            msg.readShort();  // unknown
            break;

        case SMSG_PLAYER_STAT_UPDATE_6:
            switch (msg.readShort()) {
                case 0x0020:
                    player_node->mAttrUp[LocalPlayer::STR] = msg.readByte();
                    break;
                case 0x0021:
                    player_node->mAttrUp[LocalPlayer::AGI] = msg.readByte();
                    break;
                case 0x0022:
                    player_node->mAttrUp[LocalPlayer::VIT] = msg.readByte();
                    break;
                case 0x0023:
                    player_node->mAttrUp[LocalPlayer::INT] = msg.readByte();
                    break;
                case 0x0024:
                    player_node->mAttrUp[LocalPlayer::DEX] = msg.readByte();
                    break;
                case 0x0025:
                    player_node->mAttrUp[LocalPlayer::LUK] = msg.readByte();
                    break;
            }
            break;

        case SMSG_PLAYER_ARROW_MESSAGE:
            {
                Sint16 type = msg.readShort();

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
    }
}

void
PlayerHandler::handleMapChangeMessage(MessageIn &msg)
{
    // { "mapname", x, y, B new server [, token, "gameserver", W port] }

    std::string mapName = msg.readString();
    unsigned short x = msg.readShort();
    unsigned short y = msg.readShort();
    unsigned char newServer = msg.readByte();

    logger->log("Changing map to %s (%d, %d) on %s server",
            mapName.c_str(), x, y, (newServer) ? "another" : "same");

    // Switch the actual map, deleting the previous one
    engine->changeMap(mapName);

    current_npc = 0;

    player_node->setAction(Being::STAND);
    player_node->stopAttack();
    player_node->mFrame = 0;

    player_node->mX = x;
    player_node->mY = y;

    if (newServer)
    {
        // TODO: Implement reconnecting to another game server
        //std::string token = msg.readString(32);
        //std::string gameServer = msg.readString();
        //unsigned short gameServerPort = msg.readShort();
    }
}
