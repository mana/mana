/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "net/tmwa/playerhandler.h"

#include "eventmanager.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "playerinfo.h"
#include "units.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/gui.h"
#include "gui/npcdialog.h"
#include "gui/okdialog.h"
#include "gui/sell.h"
#include "gui/statuswindow.h"
#include "gui/viewport.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "net/tmwa/protocol.h"
#include "net/tmwa/npchandler.h"

#include "utils/stringutils.h"
#include "utils/gettext.h"

extern OkDialog *weightNotice;
extern OkDialog *deathNotice;

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

            BuyDialog::closeAll();
            BuySellDialog::closeAll();
            NpcDialog::closeAll();
            SellDialog::closeAll();

            viewport->closePopupMenu();

            TmwAthena::NpcHandler *handler =
                    static_cast<TmwAthena::NpcHandler*>(Net::getNpcHandler());
            handler->clearDialogs();
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

extern Net::PlayerHandler *playerHandler;

namespace TmwAthena {

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

void PlayerHandler::handleMessage(Net::MessageIn &msg)
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
                int x = msg.readInt16();
                int y = msg.readInt16();

                logger->log("Warping to %s (%d, %d)", mapPath.c_str(), x, y);

                /*
                 * We must clear the local player's target *before* the call
                 * to changeMap, as it deletes all beings.
                 */
                player_node->stopAttack();

                Game *game = Game::instance();

                const std::string &currentMapName = game->getCurrentMapName();
                bool sameMap = (currentMapName == mapPath);

                // Switch the actual map, deleting the previous one if necessary
                mapPath = mapPath.substr(0, mapPath.rfind("."));
                game->changeMap(mapPath);

                float scrollOffsetX = 0.0f;
                float scrollOffsetY = 0.0f;

                /* Scroll if neccessary */
                if (!sameMap
                    || (abs(x - player_node->getTileX()) > MAP_TELEPORT_SCROLL_DISTANCE)
                    || (abs(y - player_node->getTileY()) > MAP_TELEPORT_SCROLL_DISTANCE))
                {
                    Map *map = game->getCurrentMap();
                    scrollOffsetX = (x - player_node->getTileX())
                                    * map->getTileWidth();
                    scrollOffsetY = (y - player_node->getTileY())
                                    * map->getTileHeight();
                }

                player_node->setAction(Being::STAND);
                player_node->setTileCoords(x, y);

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
                    case 0x0000:
                      player_node->setWalkSpeed(Vector(value, value, 0));
                    break;
                    case 0x0004: break; // manner
                    case 0x0005: PlayerInfo::setAttribute(HP, value); break;
                    case 0x0006: PlayerInfo::setAttribute(MAX_HP, value); break;
                    case 0x0007: PlayerInfo::setAttribute(MP, value); break;
                    case 0x0008: PlayerInfo::setAttribute(MAX_MP, value); break;
                    case 0x0009: PlayerInfo::setAttribute(CHAR_POINTS, value); break;
                    case 0x000b: PlayerInfo::setAttribute(LEVEL, value); break;
                    case 0x000c: PlayerInfo::setAttribute(SKILL_POINTS, value); break;
                    case 0x0018:
                                 if (value >= PlayerInfo::getAttribute(MAX_WEIGHT) / 2 &&
                                         PlayerInfo::getAttribute(TOTAL_WEIGHT) <
                                         PlayerInfo::getAttribute(MAX_WEIGHT) / 2)
                                 {
                                     weightNotice = new OkDialog(_("Message"),
                                             _("You are carrying more than "
                                               "half your weight. You are "
                                               "unable to regain health."));
                                     weightNotice->addActionListener(
                                             &weightListener);
                                 }
                                 PlayerInfo::setAttribute(TOTAL_WEIGHT, value);
                                 break;
                    case 0x0019: PlayerInfo::setAttribute(MAX_WEIGHT, value); break;

                    case 0x0029: PlayerInfo::setStatBase(ATK, value); break;
                    case 0x002a: PlayerInfo::setStatMod(ATK, value); break;

                    case 0x002b: PlayerInfo::setStatBase(MATK, value); break;
                    case 0x002c: PlayerInfo::setStatMod(MATK, value); break;

                    case 0x002d: PlayerInfo::setStatBase(DEF, value); break;
                    case 0x002e: PlayerInfo::setStatMod(DEF, value); break;

                    case 0x002f: PlayerInfo::setStatBase(MDEF, value); break;
                    case 0x0030: PlayerInfo::setStatMod(MDEF, value); break;

                    case 0x0031: PlayerInfo::setStatBase(HIT, value); break;

                    case 0x0032: PlayerInfo::setStatBase(FLEE, value); break;
                    case 0x0033: PlayerInfo::setStatMod(FLEE, value); break;

                    case 0x0034: PlayerInfo::setStatBase(CRIT, value); break;

                    case 0x0035: player_node->setAttackSpeed(value); break;

                    case 0x0037: PlayerInfo::setStatBase(JOB, value); break;

                    case 500: player_node->setGMLevel(value); break;
                }

                if (PlayerInfo::getAttribute(HP) == 0 && !deathNotice)
                {
                    deathNotice = new OkDialog(_("Message"),
                                               randomDeathMessage(),
                                               false);
                    deathNotice->addActionListener(&deathListener);
                    player_node->setAction(Being::DEAD);
                }
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_2:
            switch (msg.readInt16())
            {
                case 0x0001:
                    PlayerInfo::setAttribute(EXP, msg.readInt32());
                    break;
                case 0x0002:
                    PlayerInfo::setStatExperience(JOB, msg.readInt32(),
                                                  PlayerInfo::getStatExperience(JOB).second);
                    break;
                case 0x0014: {
                        int oldMoney = PlayerInfo::getAttribute(MONEY);
                        int newMoney = msg.readInt32();
                        if (newMoney > oldMoney)
                            SERVER_NOTICE(strprintf(_("You picked up %s."),
                                         Units::formatCurrency(newMoney -
                                         oldMoney).c_str()))
                    }
                    break;
                case 0x0016:
                    PlayerInfo::setAttribute(EXP_NEEDED, msg.readInt32());
                    break;
                case 0x0017:
                    PlayerInfo::setStatExperience(JOB,
                                                  PlayerInfo::getStatExperience(JOB).first,
                                                  msg.readInt32());
                    break;
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_3: // Update a base attribute
            {
                int type = msg.readInt32();
                int base = msg.readInt32();
                int bonus = msg.readInt32();

                PlayerInfo::setStatBase(type, base, false);
                PlayerInfo::setStatMod(type, bonus);
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_4: // Attribute increase ack
            {
                int type = msg.readInt16();
                int ok = msg.readInt8();
                int value = msg.readInt8();

                if (ok != 1)
                {
                    SERVER_NOTICE(_("Cannot raise skill!"))
                }

                PlayerInfo::setStatBase(type, value);
            }
            break;

        // Updates stats and status points
        case SMSG_PLAYER_STAT_UPDATE_5:
            PlayerInfo::setAttribute(CHAR_POINTS, msg.readInt16());

            {
                int val = msg.readInt8();
                PlayerInfo::setStatBase(STR, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(STR, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(STR, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(AGI, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(AGI, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(AGI, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(VIT, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(VIT, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(VIT, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(INT, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(INT, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(INT, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(DEX, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(DEX, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(DEX, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(LUK, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(LUK, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(LUK, msg.readInt8());
                }

                PlayerInfo::setStatBase(ATK, msg.readInt16(), false);
                PlayerInfo::setStatMod(ATK, msg.readInt16());

                PlayerInfo::setStatBase(MATK, msg.readInt16(), false);
                PlayerInfo::setStatMod(MATK, msg.readInt16());


                PlayerInfo::setStatBase(DEF, msg.readInt16(), false);
                PlayerInfo::setStatMod(DEF, msg.readInt16());

                PlayerInfo::setStatBase(MDEF, msg.readInt16(), false);
                PlayerInfo::setStatMod(MDEF, msg.readInt16());

                PlayerInfo::setStatBase(HIT, msg.readInt16());

                PlayerInfo::setStatBase(FLEE, msg.readInt16(), false);
                PlayerInfo::setStatMod(FLEE, msg.readInt16());

                PlayerInfo::setStatBase(CRIT, msg.readInt16());
            }

            msg.readInt16();  // manner
            break;

        case SMSG_PLAYER_STAT_UPDATE_6:
            switch (msg.readInt16())
            {
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

                switch (type)
                {
                    case 0:
                        {
                            SERVER_NOTICE(_("Equip arrows first."))
                        }
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

void PlayerHandler::increaseAttribute(int attr)
{
    if (attr >= STR && attr <= LUK)
    {
        MessageOut outMsg(CMSG_STAT_UPDATE_REQUEST);
        outMsg.writeInt16(attr);
        outMsg.writeInt8(1);
    }
}

void PlayerHandler::decreaseAttribute(int attr)
{
    // Supported by eA?
}

void PlayerHandler::increaseSkill(int skillId)
{
    if (PlayerInfo::getAttribute(SKILL_POINTS) <= 0)
        return;

    MessageOut outMsg(CMSG_SKILL_LEVELUP_REQUEST);
    outMsg.writeInt16(skillId);
}

void PlayerHandler::pickUp(FloorItem *floorItem)
{
    if (floorItem)
    {
        MessageOut outMsg(CMSG_ITEM_PICKUP);
        outMsg.writeInt32(floorItem->getId());
    }
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
    MessageOut outMsg(CMSG_PLAYER_RESTART);
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

bool PlayerHandler::canUseMagic()
{
    return PlayerInfo::getStatEffective(MATK) > 0;
}

bool PlayerHandler::canCorrectAttributes()
{
    return false;
}

int PlayerHandler::getJobLocation()
{
    return JOB;
}

Vector PlayerHandler::getDefaultWalkSpeed()
{
    // Return an normalized speed for any side
    // as the offset is calculated elsewhere.
    return Vector(150, 150, 0);
}

} // namespace TmwAthena
