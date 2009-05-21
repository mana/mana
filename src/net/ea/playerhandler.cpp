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
#include "gui/skill.h"
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
                    case 0x0029: player_node->ATK = value; break;
                    case 0x002b: player_node->MATK = value; break;
                    case 0x002d: player_node->DEF = value; break;
                    case 0x002e: player_node->DEF_BONUS = value; break;
                    case 0x002f: player_node->MDEF = value; break;
                    case 0x0031: player_node->HIT = value; break;
                    case 0x0032: player_node->FLEE = value; break;
                    case 0x0035: player_node->mAttackSpeed = value; break;
                    case 0x0037: player_node->mJobLevel = value; break;
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

void PlayerHandler::attack(Being *being)
{
    MessageOut outMsg(CMSG_PLAYER_ATTACK);
    outMsg.writeInt32(being->getId());
    outMsg.writeInt8(0);
}

void PlayerHandler::emote(int emoteId)
{
    MessageOut outMsg(CMSG_PLAYER_EMOTE);
    outMsg.writeInt8(emoteId);
}

void PlayerHandler::increaseStat(LocalPlayer::Attribute attr)
{
    MessageOut outMsg(CMSG_STAT_UPDATE_REQUEST);

    switch (attr)
    {
        case LocalPlayer::STR:
            outMsg.writeInt16(0x000d);
            break;

        case LocalPlayer::AGI:
            outMsg.writeInt16(0x000e);
            break;

        case LocalPlayer::VIT:
            outMsg.writeInt16(0x000f);
            break;

        case LocalPlayer::INT:
            outMsg.writeInt16(0x0010);
            break;

        case LocalPlayer::DEX:
            outMsg.writeInt16(0x0011);
            break;

        case LocalPlayer::LUK:
            outMsg.writeInt16(0x0012);
            break;
    }
    outMsg.writeInt8(1);
}

void PlayerHandler::decreaseStat(LocalPlayer::Attribute attr)
{
    // Supported by eA?
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
