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

#include "../messagein.h"
#include "playerhandler.h"
#include "protocol.h"

#include "../../engine.h"
#include "../../localplayer.h"
#include "../../log.h"
#include "../../npc.h"
#include "../../units.h"

#include "../../gui/buy.h"
#include "../../gui/buysell.h"
#include "../../gui/chat.h"
#include "../../gui/gui.h"
#include "../../gui/npc_text.h"
#include "../../gui/npcintegerdialog.h"
#include "../../gui/npclistdialog.h"
#include "../../gui/npcstringdialog.h"
#include "../../gui/ok_dialog.h"
#include "../../gui/sell.h"
#include "../../gui/skill.h"
#include "../../gui/storagewindow.h"
#include "../../gui/viewport.h"

#include "../../utils/stringutils.h"
#include "../../utils/gettext.h"

// TODO Move somewhere else
OkDialog *weightNotice = NULL;
OkDialog *deathNotice = NULL;

// Max. distance we are willing to scroll after a teleport;
// everything beyond will reset the port hard.
static const int MAP_TELEPORT_SCROLL_DISTANCE = 8;

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
            npcIntegerDialog->reset();
            npcIntegerDialog->setVisible(false);
            npcListDialog->reset();
            npcListDialog->setVisible(false);
            npcStringDialog->setValue("");
            npcStringDialog->setVisible(false);
            npcTextDialog->clearText();
            npcTextDialog->setVisible(false);
            buyDialog->setVisible(false);
            sellDialog->setVisible(false);
            buySellDialog->setVisible(false);

            if (storageWindow->isVisible()) storageWindow->close();
        }
    } deathListener;
}

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

void PlayerHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_WALK_RESPONSE:
            /*
             * This client assumes that all walk messages succeed,
             * and that the server will send a correction notice
             * otherwise.
             */
            break;

        case SMSG_PLAYER_WARP:
            {
                std::string mapPath = msg.readString(16);
                bool nearby;
                Uint16 x = msg.readInt16();
                Uint16 y = msg.readInt16();

                logger->log("Warping to %s (%d, %d)", mapPath.c_str(), x, y);

                /*
                 * We must clear the local player's target *before* the call
                 * to changeMap, as it deletes all beings.
                 */
                player_node->stopAttack();

                nearby = (engine->getCurrentMapName() == mapPath);

                // Switch the actual map, deleting the previous one if necessary
                engine->changeMap(mapPath);

                float scrollOffsetX = 0.0f;
                float scrollOffsetY = 0.0f;

                /* Scroll if neccessary */
                if (!nearby
                    || (abs(x - player_node->mX) > MAP_TELEPORT_SCROLL_DISTANCE)
                    || (abs(y - player_node->mY) > MAP_TELEPORT_SCROLL_DISTANCE))
                {
                    scrollOffsetX = (x - player_node->mX) * 32;
                    scrollOffsetY = (y - player_node->mY) * 32;
                }

                player_node->setAction(Being::STAND);
                player_node->mFrame = 0;
                player_node->mX = x;
                player_node->mY = y;

                logger->log("Adjust scrolling by %d:%d",
                        (int) scrollOffsetX,
                        (int) scrollOffsetY);

                viewport->scrollBy(scrollOffsetX, scrollOffsetY);
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_1:
            {
                int type = msg.readInt16();
                Uint32 value = msg.readInt32();

                switch (type)
                {
                    //case 0x0000:
                    //    player_node->setWalkSpeed(msg.readInt32());
                    //    break;
                    case 0x0005: player_node->setHp(value); break;
                    case 0x0006: player_node->setMaxHp(value); break;
                    case 0x0007: player_node->mMp = value; break;
                    case 0x0008: player_node->mMaxMp = value; break;
                    case 0x0009:
                                 player_node->mStatsPointsToAttribute = value;
                                 break;
                    case 0x000b: player_node->setLevel(value); break;
                    case 0x000c:
                                 player_node->mSkillPoint = value;
                                 skillDialog->update();
                                 break;
                    case 0x0018:
                                 if ((int) value >= player_node->getMaxWeight() / 2 &&
                                         player_node->getTotalWeight() <
                                         player_node->getMaxWeight() / 2)
                                 {
                                     weightNotice = new OkDialog(_("Message"),
                                             _("You are carrying more than "
                                               "half your weight. You are "
                                               "unable to regain health."));
                                     weightNotice->addActionListener(
                                             &weightListener);
                                 }
                                 player_node->setTotalWeight(value);
                                 break;
                    case 0x0019: player_node->setMaxWeight(value); break;
                    case 0x0029: player_node->ATK = value; break;
                    case 0x002b: player_node->MATK = value; break;
                    case 0x002d: player_node->DEF = value; break;
                    case 0x002e: player_node->DEF_BONUS = value; break;
                    case 0x002f: player_node->MDEF = value; break;
                    case 0x0031: player_node->HIT = value; break;
                    case 0x0032: player_node->FLEE = value; break;
                    case 0x0035: player_node->mAttackSpeed = value; break;
                    case 0x0037: player_node->mJobLevel = value; break;
                }

                if (player_node->getHp() == 0 && !deathNotice)
                {
                    static char const *const deadMsg[] =
                    {
                        _("You are dead."),
                        _("We regret to inform you that your character was "
                          "killed in battle."),
                        _("You are not that alive anymore."),
                        _("The cold hands of the grim reaper are grabbing for "
                          "your soul."),
                        _("Game Over!"),
                        _("Insert coin to continue"),
                        _("No, kids. Your character did not really die. It... "
                          "err... went to a better place."),
                        _("Your plan of breaking your enemies weapon by "
                          "bashing it with your throat failed."),
                        _("I guess this did not run too well."),
                        // NetHack reference:
                        _("Do you want your possessions identified?"),
                        // Secret of Mana reference:
                        _("Sadly, no trace of you was ever found..."),
                        // Final Fantasy VI reference:
                        _("Annihilated."),
                        // Earthbound reference:
                        _("Looks like you got your head handed to you."),
                        // Leisure Suit Larry 1 reference:
                        _("You screwed up again, dump your body down the tubes "
                          "and get you another one."),
                        // Monty Python references (Dead Parrot sketch mostly):
                        _("You're not dead yet. You're just resting."),
                        _("You are no more."),
                        _("You have ceased to be."),
                        _("You've expired and gone to meet your maker."),
                        _("You're a stiff."),
                        _("Bereft of life, you rest in peace."),
                        _("If you weren't so animated, you'd be pushing up the "
                          "daisies."),
                        _("Your metabolic processes are now history."),
                        _("You're off the twig."),
                        _("You've kicked the bucket."),
                        _("You've shuffled off your mortal coil, run down the "
                          "curtain and joined the bleedin' choir invisibile."),
                        _("You are an ex-player."),
                        _("You're pining for the fjords.")
                    };
                    std::string message(deadMsg[rand()%27]);

                    deathNotice = new OkDialog(_("Message"), message);
                    deathNotice->addActionListener(&deathListener);
                    player_node->setAction(Being::DEAD);
                }
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_2:
            switch (msg.readInt16()) {
                case 0x0001:
                    player_node->setXp(msg.readInt32());
                    break;
                case 0x0002:
                    player_node->mJobXp = msg.readInt32();
                    break;
                case 0x0014: {
                        int curGp = player_node->getMoney();
                        player_node->setMoney(msg.readInt32());
                        if (player_node->getMoney() > curGp)
                            localChatTab->chatLog(_("You picked up ") +
                                Units::formatCurrency(player_node->getMoney()
                                    - curGp), BY_SERVER);
                    }
                    break;
                case 0x0016:
                    player_node->mXpForNextLevel = msg.readInt32();
                    break;
                case 0x0017:
                    player_node->mJobXpForNextLevel = msg.readInt32();
                    break;
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_3:
            {
                int type = msg.readInt32();
                int base = msg.readInt32();
                int bonus = msg.readInt32();
                int total = base + bonus;

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
                int type = msg.readInt16();
                int fail = msg.readInt8();
                int value = msg.readInt8();

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
            player_node->mStatsPointsToAttribute = msg.readInt16();
            player_node->mAttr[LocalPlayer::STR] = msg.readInt8();
            player_node->mAttrUp[LocalPlayer::STR] = msg.readInt8();
            player_node->mAttr[LocalPlayer::AGI] = msg.readInt8();
            player_node->mAttrUp[LocalPlayer::AGI] = msg.readInt8();
            player_node->mAttr[LocalPlayer::VIT] = msg.readInt8();
            player_node->mAttrUp[LocalPlayer::VIT] = msg.readInt8();
            player_node->mAttr[LocalPlayer::INT] = msg.readInt8();
            player_node->mAttrUp[LocalPlayer::INT] = msg.readInt8();
            player_node->mAttr[LocalPlayer::DEX] = msg.readInt8();
            player_node->mAttrUp[LocalPlayer::DEX] = msg.readInt8();
            player_node->mAttr[LocalPlayer::LUK] = msg.readInt8();
            player_node->mAttrUp[LocalPlayer::LUK] = msg.readInt8();
            player_node->ATK       = msg.readInt16();  // ATK
            player_node->ATK_BONUS  = msg.readInt16();  // ATK bonus
            player_node->MATK      = msg.readInt16();  // MATK max
            player_node->MATK_BONUS = msg.readInt16();  // MATK min
            player_node->DEF       = msg.readInt16();  // DEF
            player_node->DEF_BONUS  = msg.readInt16();  // DEF bonus
            player_node->MDEF      = msg.readInt16();  // MDEF
            player_node->MDEF_BONUS = msg.readInt16();  // MDEF bonus
            player_node->HIT       = msg.readInt16();  // HIT
            player_node->FLEE      = msg.readInt16();  // FLEE
            player_node->FLEE_BONUS = msg.readInt16();  // FLEE bonus
            msg.readInt16();  // critical
            msg.readInt16();  // unknown
            break;

        case SMSG_PLAYER_STAT_UPDATE_6:
            switch (msg.readInt16()) {
                case 0x0020:
                    player_node->mAttrUp[LocalPlayer::STR] = msg.readInt8();
                    break;
                case 0x0021:
                    player_node->mAttrUp[LocalPlayer::AGI] = msg.readInt8();
                    break;
                case 0x0022:
                    player_node->mAttrUp[LocalPlayer::VIT] = msg.readInt8();
                    break;
                case 0x0023:
                    player_node->mAttrUp[LocalPlayer::INT] = msg.readInt8();
                    break;
                case 0x0024:
                    player_node->mAttrUp[LocalPlayer::DEX] = msg.readInt8();
                    break;
                case 0x0025:
                    player_node->mAttrUp[LocalPlayer::LUK] = msg.readInt8();
                    break;
            }
            break;

        case SMSG_PLAYER_ARROW_MESSAGE:
            {
                int type = msg.readInt16();

                switch (type) {
                    case 0:
                        localChatTab->chatLog(_("Equip arrows first"),
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
