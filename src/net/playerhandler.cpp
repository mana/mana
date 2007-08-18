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

#include "../gui/buy.h"
#include "../gui/chat.h"
#include "../gui/npclistdialog.h"
#include "../gui/npc_text.h"
#include "../gui/ok_dialog.h"
#include "../gui/sell.h"
#include "../gui/skill.h"

// TODO Move somewhere else
OkDialog *weightNotice = NULL;
OkDialog *deathNotice = NULL;

extern NpcListDialog *npcListDialog;
extern NpcTextDialog *npcTextDialog;
extern BuyDialog *buyDialog;
extern SellDialog *sellDialog;
extern Window *buySellDialog;

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
            player_node->revive();
            deathNotice = NULL;
            npcListDialog->setVisible(false);
            npcTextDialog->setVisible(false);
            buyDialog->setVisible(false);
            sellDialog->setVisible(false);
            buySellDialog->setVisible(false);
            current_npc = 0;
        }
    } deathListener;
}

PlayerHandler::PlayerHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_PLAYER_MAP_CHANGE,
        GPMSG_PLAYER_SERVER_CHANGE,
        GPMSG_PLAYER_ATTRIBUTE_CHANGE,
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

        case GPMSG_PLAYER_SERVER_CHANGE:
        {   // TODO: Implement reconnecting to another game server
            std::string token = msg.readString(32);
            std::string address = msg.readString();
            int port = msg.readShort();
            logger->log("Changing server to %s:%d", address.c_str(), port);
        } break;

        case GPMSG_PLAYER_ATTRIBUTE_CHANGE:
        {
            logger->log("ATTRIBUTE UPDATE:");
            while (msg.getUnreadLength())
            {
                int stat = msg.readByte();
                int base = msg.readShort();
                int value = msg.readShort();
                logger->log("%d set to %d %d", stat, base, value);

                if (stat == BASE_ATTR_HP)
                {
                    player_node->setMaxHP(base);
                    player_node->setHP(value);
                }
                else if (stat < NB_CHARACTER_ATTRIBUTES)
                {
                    player_node->setAttributeBase(stat, base);
                    player_node->setAttributeEffective(stat, value);
                }
                else
                {
                    logger->log("Warning: server wants to update unknown attribute %d to %d", stat, value);
                }
            }
        } break;
        /*
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
        */
    }
}

void
PlayerHandler::handleMapChangeMessage(MessageIn &msg)
{
    std::string mapName = msg.readString();
    unsigned short x = msg.readShort();
    unsigned short y = msg.readShort();

    logger->log("Changing map to %s (%d, %d)", mapName.c_str(), x, y);

    // Switch the actual map, deleting the previous one
    engine->changeMap(mapName);

    current_npc = 0;

    player_node->setAction(Being::STAND);
    player_node->mX = x;
    player_node->mY = y;
}
