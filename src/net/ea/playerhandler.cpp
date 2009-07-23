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

#include "net/ea/playerhandler.h"

#include "net/ea/protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "engine.h"
#include "localplayer.h"
#include "log.h"
#include "npc.h"
#include "units.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/gui.h"
#include "gui/okdialog.h"
#include "gui/sell.h"
#include "gui/statuswindow.h"
#include "gui/storagewindow.h"
#include "gui/viewport.h"

#include "gui/widgets/chattab.h"

#include "utils/stringutils.h"
#include "utils/gettext.h"

// TODO Move somewhere else
OkDialog *weightNotice = NULL;
OkDialog *deathNotice = NULL;

// Max. distance we are willing to scroll after a teleport;
// everything beyond will reset the port hard.
static const int MAP_TELEPORT_SCROLL_DISTANCE = 8;

#define ATTR_BONUS(atr) \
(player_node->getAttributeEffective(atr) - player_node->getAttributeBase(atr))

// TODO Move somewhere else
namespace {

    /**
     * Listener used for handling the overweigth message.
     */
    struct WeightListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            weightNotice = NULL;
        }
    } weightListener;

    /**
     * Listener used for handling death message.
     */
    struct DeathListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            Net::getPlayerHandler()->respawn();
            deathNotice = NULL;
            buyDialog->setVisible(false);
            sellDialog->setVisible(false);
            buySellDialog->setVisible(false);

            if (storageWindow->isVisible())
                storageWindow->close();
            viewport->closePopupMenu();
        }
    } deathListener;

} // anonymous namespace

static const char *randomDeathMessage()
{
    static char const *const deadMsg[] =
    {
        N_("You are dead."),
        N_("We regret to inform you that your character was killed in "
           "battle."),
        N_("You are not that alive anymore."),
        N_("The cold hands of the grim reaper are grabbing for your soul."),
        N_("Game Over!"),
        N_("Insert coin to continue."),
        N_("No, kids. Your character did not really die. It... "
           "err... went to a better place."),
        N_("Your plan of breaking your enemies weapon by "
           "bashing it with your throat failed."),
        N_("I guess this did not run too well."),
        // NetHack reference:
        N_("Do you want your possessions identified?"),
        // Secret of Mana reference:
        N_("Sadly, no trace of you was ever found..."),
        // Final Fantasy VI reference:
        N_("Annihilated."),
        // Earthbound reference:
        N_("Looks like you got your head handed to you."),
        // Leisure Suit Larry 1 reference:
        N_("You screwed up again, dump your body down the tubes "
           "and get you another one."),
        // Monty Python references (Dead Parrot sketch mostly):
        N_("You're not dead yet. You're just resting."),
        N_("You are no more."),
        N_("You have ceased to be."),
        N_("You've expired and gone to meet your maker."),
        N_("You're a stiff."),
        N_("Bereft of life, you rest in peace."),
        N_("If you weren't so animated, you'd be pushing up the daisies."),
        N_("Your metabolic processes are now history."),
        N_("You're off the twig."),
        N_("You've kicked the bucket."),
        N_("You've shuffled off your mortal coil, run down the "
           "curtain and joined the bleedin' choir invisibile."),
        N_("You are an ex-player."),
        N_("You're pining for the fjords.")
    };

    const int random = rand() % (sizeof(deadMsg) / sizeof(deadMsg[0]));
    return gettext(deadMsg[random]);
}

Net::PlayerHandler *playerHandler;

namespace EAthena {

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
    playerHandler = this;
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
                mapPath = mapPath.substr(0, mapPath.rfind("."));
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

                logger->log("Adjust scrolling by %d:%d", (int) scrollOffsetX,
                           (int) scrollOffsetY);

                viewport->scrollBy(scrollOffsetX, scrollOffsetY);
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_1:
            {
                int type = msg.readInt16();
                int value = msg.readInt32();

                switch (type)
                {
                    case 0x0000: player_node->setWalkSpeed(value); break;
                    case 0x0004: break; // manner
                    case 0x0005: player_node->setHp(value); break;
                    case 0x0006: player_node->setMaxHp(value); break;
                    case 0x0007: player_node->setMP(value); break;
                    case 0x0008: player_node->setMaxMP(value); break;
                    case 0x0009: player_node->setCharacterPoints(value); break;
                    case 0x000b: player_node->setLevel(value); break;
                    case 0x000c: player_node->setSkillPoints(value); break;
                    case 0x0018:
                                 if (value >= player_node->getMaxWeight() / 2 &&
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

                    case 0x0029: player_node->setAttributeEffective(ATK, value
                                                           + ATTR_BONUS(ATK));
                        player_node->setAttributeBase(ATK, value);
                        break;
                    case 0x002a: value += player_node->getAttributeBase(ATK);
                        player_node->setAttributeEffective(ATK, value); break;

                    case 0x002b: player_node->setAttributeEffective(MATK, value
                                                           + ATTR_BONUS(MATK));
                        player_node->setAttributeBase(MATK, value); break;
                    case 0x002c: value += player_node->getAttributeBase(MATK);
                        player_node->setAttributeEffective(MATK, value); break;

                    case 0x002d: player_node->setAttributeEffective(DEF, value
                                                           + ATTR_BONUS(DEF));
                        player_node->setAttributeBase(DEF, value); break;
                    case 0x002e: value += player_node->getAttributeBase(DEF);
                        player_node->setAttributeEffective(DEF, value); break;

                    case 0x002f: player_node->setAttributeEffective(MDEF, value
                                                           + ATTR_BONUS(MDEF));
                        player_node->setAttributeBase(MDEF, value); break;
                    case 0x0030: value += player_node->getAttributeBase(MDEF);
                        player_node->setAttributeEffective(MDEF, value); break;

                    case 0x0031: player_node->setAttributeBase(HIT, value);
                        player_node->setAttributeEffective(HIT, value); break;

                    case 0x0032: player_node->setAttributeEffective(FLEE, value
                                                           + ATTR_BONUS(FLEE));
                        player_node->setAttributeBase(FLEE, value); break;
                    case 0x0033: value += player_node->getAttributeBase(FLEE);
                        player_node->setAttributeEffective(FLEE, value); break;

                    case 0x0034: player_node->setAttributeBase(CRIT, value);
                        player_node->setAttributeEffective(CRIT, value); break;

                    case 0x0035: player_node->mAttackSpeed = value; break;
                    case 0x0037: player_node->setAttributeBase(JOB, value);
                        player_node->setAttributeEffective(JOB, value); break;
                    case 500: player_node->setGMLevel(value); break;
                }

                if (player_node->getHp() == 0 && !deathNotice)
                {
                    deathNotice = new OkDialog(_("Message"),
                                               randomDeathMessage());
                    deathNotice->addActionListener(&deathListener);
                    player_node->setAction(Being::DEAD);
                }
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_2:
            switch (msg.readInt16()) {
                case 0x0001:
                    player_node->setExp(msg.readInt32());
                    break;
                case 0x0002:
                    player_node->setExperience(JOB, msg.readInt32(),
                                    player_node->getExperience(JOB).second);
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
                    player_node->setExpNeeded(msg.readInt32());
                    break;
                case 0x0017:
                    player_node->setExperience(JOB,
                                    player_node->getExperience(JOB).first,
                                    msg.readInt32());
                    break;
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_3:
            {
                int type = msg.readInt32();
                int base = msg.readInt32();
                int bonus = msg.readInt32();

                player_node->setAttributeBase(type, base);
                player_node->setAttributeEffective(type, base + bonus);
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_4: // Attribute increase ack
            {
                int type = msg.readInt16();
                int fail = msg.readInt8();
                int value = msg.readInt8();

                if (fail != 1)
                    break;

                int bonus = ATTR_BONUS(type);

                player_node->setAttributeBase(type, value);
                player_node->setAttributeEffective(type, value + bonus);
            }
            break;

        // Updates stats and status points
        case SMSG_PLAYER_STAT_UPDATE_5:
            player_node->setCharacterPoints(msg.readInt16());

            {
                int val = msg.readInt8();
                player_node->setAttributeEffective(STR, val + ATTR_BONUS(STR));
                player_node->setAttributeBase(STR, val);
                statusWindow->setPointsNeeded(STR, msg.readInt8());

                val = msg.readInt8();
                player_node->setAttributeEffective(AGI, val + ATTR_BONUS(AGI));
                player_node->setAttributeBase(AGI, val);
                statusWindow->setPointsNeeded(AGI, msg.readInt8());

                val = msg.readInt8();
                player_node->setAttributeEffective(VIT, val + ATTR_BONUS(VIT));
                player_node->setAttributeBase(VIT, val);
                statusWindow->setPointsNeeded(VIT, msg.readInt8());

                val = msg.readInt8();
                player_node->setAttributeEffective(INT, val + ATTR_BONUS(INT));
                player_node->setAttributeBase(INT, val);
                statusWindow->setPointsNeeded(INT, msg.readInt8());

                val = msg.readInt8();
                player_node->setAttributeEffective(DEX, val + ATTR_BONUS(DEX));
                player_node->setAttributeBase(DEX, val);
                statusWindow->setPointsNeeded(DEX, msg.readInt8());

                val = msg.readInt8();
                player_node->setAttributeEffective(LUK, val + ATTR_BONUS(LUK));
                player_node->setAttributeBase(LUK, val);
                statusWindow->setPointsNeeded(LUK, msg.readInt8());

                val = msg.readInt16(); // ATK
                player_node->setAttributeBase(ATK, val);
                val += msg.readInt16();  // ATK bonus
                player_node->setAttributeEffective(ATK, val);

                val = msg.readInt16(); // MATK
                player_node->setAttributeBase(MATK, val);
                val += msg.readInt16();  // MATK bonus
                player_node->setAttributeEffective(MATK, val);

                val = msg.readInt16(); // DEF
                player_node->setAttributeBase(DEF, val);
                val += msg.readInt16();  // DEF bonus
                player_node->setAttributeEffective(DEF, val);

                val = msg.readInt16(); // MDEF
                player_node->setAttributeBase(MDEF, val);
                val += msg.readInt16();  // MDEF bonus
                player_node->setAttributeEffective(MDEF, val);

                val = msg.readInt16(); // HIT
                player_node->setAttributeBase(ATK, val);
                player_node->setAttributeEffective(ATK, val);

                val = msg.readInt16(); // FLEE
                player_node->setAttributeBase(FLEE, val);
                val += msg.readInt16();  // FLEE bonus
                player_node->setAttributeEffective(FLEE, val);

                val = msg.readInt16();
                player_node->setAttributeBase(CRIT, val);
                player_node->setAttributeEffective(CRIT, val);
            }

            msg.readInt16();  // manner
            break;

        case SMSG_PLAYER_STAT_UPDATE_6:
            switch (msg.readInt16()) {
                case 0x0020:
                    statusWindow->setPointsNeeded(STR, msg.readInt8());
                    break;
                case 0x0021:
                    statusWindow->setPointsNeeded(AGI, msg.readInt8());
                    break;
                case 0x0022:
                    statusWindow->setPointsNeeded(VIT, msg.readInt8());
                    break;
                case 0x0023:
                    statusWindow->setPointsNeeded(INT, msg.readInt8());
                    break;
                case 0x0024:
                    statusWindow->setPointsNeeded(DEX, msg.readInt8());
                    break;
                case 0x0025:
                    statusWindow->setPointsNeeded(LUK, msg.readInt8());
                    break;
            }
            break;

        case SMSG_PLAYER_ARROW_MESSAGE:
            {
                int type = msg.readInt16();

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
    }
}

void PlayerHandler::attack(int id)
{
    MessageOut outMsg(CMSG_PLAYER_ATTACK);
    outMsg.writeInt32(id);
    outMsg.writeInt8(0);
}

void PlayerHandler::emote(int emoteId)
{
    MessageOut outMsg(CMSG_PLAYER_EMOTE);
    outMsg.writeInt8(emoteId);
}

void PlayerHandler::increaseAttribute(size_t attr)
{
    if (attr >= STR && attr <= LUK)
    {
        MessageOut outMsg(CMSG_STAT_UPDATE_REQUEST);
        outMsg.writeInt16(attr);
        outMsg.writeInt8(1);
    }
}

void PlayerHandler::decreaseAttribute(size_t attr)
{
    // Supported by eA?
}

void PlayerHandler::increaseSkill(int skillId)
{
    if (player_node->getSkillPoints() <= 0)
        return;

    MessageOut outMsg(CMSG_SKILL_LEVELUP_REQUEST);
    outMsg.writeInt16(skillId);
}

void PlayerHandler::pickUp(FloorItem *floorItem)
{
    MessageOut outMsg(CMSG_ITEM_PICKUP);
    outMsg.writeInt32(floorItem->getId());
}

void PlayerHandler::setDirection(char direction)
{
    MessageOut outMsg(CMSG_PLAYER_CHANGE_DIR);
    outMsg.writeInt16(0);
    outMsg.writeInt8(direction);
}

void PlayerHandler::setDestination(int x, int y, int direction)
{
    MessageOut outMsg(CMSG_PLAYER_CHANGE_DEST);
    outMsg.writeCoordinates(x, y, direction);
}

void PlayerHandler::changeAction(Being::Action action)
{
    char type;
    switch (action)
    {
        case Being::SIT: type = 2; break;
        case Being::STAND: type = 3; break;
        default: return;
    }

    MessageOut outMsg(CMSG_PLAYER_CHANGE_ACT);
    outMsg.writeInt32(0);
    outMsg.writeInt8(type);
}

void PlayerHandler::respawn()
{
    MessageOut outMsg(CMSG_PLAYER_RESPAWN);
    outMsg.writeInt8(0);
}

void PlayerHandler::ignorePlayer(const std::string &player, bool ignore)
{
    // TODO
}

void PlayerHandler::ignoreAll(bool ignore)
{
    // TODO
}

} // namespace EAthena
