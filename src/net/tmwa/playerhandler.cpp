/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "actorspritemanager.h"
#include "being.h"
#include "client.h"
#include "configuration.h"
#include "effectmanager.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "playerinfo.h"
#include "units.h"

#include "gui/buydialog.h"
#include "gui/buyselldialog.h"
#include "gui/okdialog.h"
#include "gui/selldialog.h"
#include "gui/statuswindow.h"
#include "gui/viewport.h"

#include "net/tmwa/messagein.h"
#include "net/tmwa/messageout.h"
#include "net/tmwa/protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/time.h"

extern OkDialog *weightNotice;
extern OkDialog *deathNotice;

// Max. distance we are willing to scroll after a teleport;
// everything beyond will reset the port hard.
const int MAP_TELEPORT_SCROLL_DISTANCE = 8;

// TODO Move somewhere else
namespace {

    /**
     * Listener used for handling the overweigth message.
     */
    struct WeightListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event) override
        {
            weightNotice = nullptr;
        }
    } weightListener;

    /**
     * Listener used for handling death message.
     */
    struct DeathListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event) override
        {
            Net::getPlayerHandler()->respawn();
            deathNotice = nullptr;

            BuyDialog::closeAll();
            BuySellDialog::closeAll();
            SellDialog::closeAll();

            viewport->closePopupMenu();

            Event::trigger(Event::NpcChannel, Event::CloseAll);
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
        SMSG_MAP_MASK,
        SMSG_QUEST_SET_VAR,
        SMSG_QUEST_PLAYER_VARS,
        0
    };
    handledMessages = _messages;
    playerHandler = this;

    listen(Event::GameChannel);
}

void PlayerHandler::handleMessage(MessageIn &msg)
{
    if (!local_player)
        return;

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
                local_player->stopAttack();

                Game *game = Game::instance();

                const std::string &currentMapName = game->getCurrentMapName();
                bool sameMap = (currentMapName == mapPath);

                // Switch the actual map, deleting the previous one if necessary
                mapPath = mapPath.substr(0, mapPath.rfind("."));
                game->changeMap(mapPath);

                float scrollOffsetX = 0.0f;
                float scrollOffsetY = 0.0f;

                /* Scroll if necessary */
                Map *map = game->getCurrentMap();
                int tileX = local_player->getTileX();
                int tileY = local_player->getTileY();
                if (!sameMap
                    || (abs(x - tileX) > MAP_TELEPORT_SCROLL_DISTANCE)
                    || (abs(y - tileY) > MAP_TELEPORT_SCROLL_DISTANCE))
                {
                    scrollOffsetX = (x - tileX) * map->getTileWidth();
                    scrollOffsetY = (y - tileY) * map->getTileHeight();
                }

                local_player->setAction(Being::STAND);
                Vector pos = map->getTileCenter(x, y);
                local_player->setPosition(pos);
                // Stop movement
                local_player->setDestination(pos.x, pos.y);

                logger->log("Adjust scrolling by %d:%d", (int) scrollOffsetX,
                           (int) scrollOffsetY);

                viewport->scrollBy(scrollOffsetX, scrollOffsetY);
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_1:
            {
                if (!local_player)
                    break;
                int type = msg.readInt16();
                int value = msg.readInt32();

                switch (type)
                {
                    case 0x0000:
                      local_player->setMoveSpeed(Vector(value / 10,
                                                       value / 10, 0));
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

                    case 0x0035: local_player->setAttackSpeed(value); break;

                    case 0x0037: PlayerInfo::setStatBase(JOB, value); break;

                    case 500: local_player->setGMLevel(value); break;
                }

                if (PlayerInfo::getAttribute(HP) == 0 && !deathNotice)
                {
                    viewport->shakeScreen(100);
                    deathNotice = new OkDialog(_("Message"),
                                               randomDeathMessage(),
                                               false);
                    deathNotice->addActionListener(&deathListener);
                    local_player->setAction(Being::DEAD);
                }
            }

            if (statusWindow)
                statusWindow->updateAttrs();

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

                case 0x0014:
                    {
                        int oldMoney = PlayerInfo::getAttribute(MONEY);
                        int newMoney = msg.readInt32();
                        PlayerInfo::setAttribute(MONEY, newMoney);
                        if (newMoney > oldMoney)
                        {
                            std::string money = Units::formatCurrency(newMoney - oldMoney);
                            if (config.showPickupChat)
                                serverNotice(strprintf(_("You picked up %s."), money.c_str()));
                            if (config.showPickupParticle)
                                local_player->addMessageToQueue(money, UserPalette::PICKUP_INFO);
                        }
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
                    serverNotice(_("Cannot raise skill!"));

                PlayerInfo::setStatBase(type, value);
            }
            break;

        // Updates stats and status points
        case SMSG_PLAYER_STAT_UPDATE_5:
            PlayerInfo::setAttribute(CHAR_POINTS, msg.readInt16());

            {
                int val = msg.readInt8();
                PlayerInfo::setStatBase(STRENGTH, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(STRENGTH, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(STRENGTH, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(AGILITY, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(AGILITY, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(AGILITY, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(VITALITY, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(VITALITY, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(VITALITY, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(INTELLIGENCE, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(INTELLIGENCE, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(INTELLIGENCE, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(DEXTERITY, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(DEXTERITY, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(DEXTERITY, msg.readInt8());
                }

                val = msg.readInt8();
                PlayerInfo::setStatBase(LUCK, val);
                if (val >= 99)
                {
                    statusWindow->setPointsNeeded(LUCK, 0);
                    msg.readInt8();
                }
                else
                {
                    statusWindow->setPointsNeeded(LUCK, msg.readInt8());
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
                    statusWindow->setPointsNeeded(STRENGTH, msg.readInt8());
                    break;
                case 0x0021:
                    statusWindow->setPointsNeeded(AGILITY, msg.readInt8());
                    break;
                case 0x0022:
                    statusWindow->setPointsNeeded(VITALITY, msg.readInt8());
                    break;
                case 0x0023:
                    statusWindow->setPointsNeeded(INTELLIGENCE, msg.readInt8());
                    break;
                case 0x0024:
                    statusWindow->setPointsNeeded(DEXTERITY, msg.readInt8());
                    break;
                case 0x0025:
                    statusWindow->setPointsNeeded(LUCK, msg.readInt8());
                    break;
            }
            break;

        case SMSG_PLAYER_ARROW_MESSAGE:
            {
                int type = msg.readInt16();

                switch (type)
                {
                    case 0:
                        serverNotice(_("Equip arrows first."));
                        break;
                    default:
                        logger->log("0x013b: Unhandled message %i", type);
                        break;
                }
            }
            break;

        case SMSG_MAP_MASK:
            {
                const int mask = msg.readInt32();
                msg.readInt32();  // unused
                if (auto game = Game::instance())
                    if (Map *map = game->getCurrentMap())
                        map->setMask(mask);
            }
            break;

        case SMSG_QUEST_SET_VAR:
        {
            int variable = msg.readInt16();
            int value = msg.readInt32();
            int oldValue = mQuestVars.get(variable);

            mQuestVars.set(variable, value);
            updateQuestStatusEffects();
            Event::trigger(Event::QuestsChannel, Event::QuestVarsChanged);

            if (effectManager && local_player)
            {
                switch (QuestDB::questChange(variable, oldValue, value))
                {
                    case QuestChange::None:
                        break;
                    case QuestChange::New:
                        effectManager->trigger(paths.getIntValue("newQuestEffectId"), local_player);
                        break;
                    case QuestChange::Completed:
                        effectManager->trigger(paths.getIntValue("completeQuestEffectId"), local_player);
                        break;
                }
            }
            break;
        }

        case SMSG_QUEST_PLAYER_VARS:
        {
            msg.readInt16();  // length
            mQuestVars.clear();
            unsigned int count = (msg.getLength() - 4) / 6;
            for (unsigned int i = 0; i < count; ++i)
            {
                int variable = msg.readInt16();
                int value = msg.readInt32();
                mQuestVars.set(variable, value);
            }
            updateQuestStatusEffects();
            Event::trigger(Event::QuestsChannel, Event::QuestVarsChanged);
            break;
        }
    }
}

void PlayerHandler::attack(int id)
{
    MessageOut outMsg(CMSG_PLAYER_CHANGE_ACT);
    outMsg.writeInt32(id);
    outMsg.writeInt8(0);
}

void PlayerHandler::emote(int emoteId)
{
    MessageOut outMsg(CMSG_PLAYER_EMOTE);
    outMsg.writeInt8(emoteId + 1);
}

void PlayerHandler::increaseAttribute(int attr)
{
    if (attr >= STRENGTH && attr <= LUCK)
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
    static Timer lastPickupTimer;

    // Avoid spamming the server with pick-up requests to prevent the player
    // from being kicked.
    if (!floorItem || !lastPickupTimer.passed())
        return;

    MessageOut outMsg(CMSG_ITEM_PICKUP);
    outMsg.writeInt32(floorItem->getId());

    lastPickupTimer.set(100);
}

void PlayerHandler::setDirection(char direction)
{
    MessageOut outMsg(CMSG_PLAYER_CHANGE_DIR);
    outMsg.writeInt16(0);
    outMsg.writeInt8(direction);
}

void PlayerHandler::setDestination(int x, int y, int direction)
{
    // The destination coordinates are received in pixel, so we translate them
    // into tiles.
    Map *map = Game::instance()->getCurrentMap();
    MessageOut outMsg(CMSG_PLAYER_CHANGE_DEST);
    outMsg.writeCoordinates(x / map->getTileWidth(), y / map->getTileHeight(),
                            direction);
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
    MessageOut outMsg(CMSG_PLAYER_REBOOT);
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

Vector PlayerHandler::getDefaultMoveSpeed() const
{
    // Return an normalized speed for any side
    // as the offset is calculated elsewhere.
    // in ticks per tile.
    return Vector(15.0f, 15.0f, 0.0f);
}

Vector PlayerHandler::getPixelsPerSecondMoveSpeed(const Vector &speed, Map *map)
{
    Game *game = Game::instance();

    if (game && !map)
        map = game->getCurrentMap();

    if (!map || speed.x == 0 || speed.y == 0)
    {
        logger->log("TmwAthena::PlayerHandler: Speed set to default: "
                    "Map not yet initialized or invalid speed.");
        return getDefaultMoveSpeed();
    }

    Vector pixelsPerSecond;

    constexpr float ticksPerSecond = 1000.0 / MILLISECONDS_IN_A_TICK;

    pixelsPerSecond.x = map->getTileWidth() / speed.x * ticksPerSecond;
    pixelsPerSecond.y = map->getTileHeight() / speed.y * ticksPerSecond;

    return pixelsPerSecond;
}

void PlayerHandler::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::GameChannel)
    {
        if (event.getType() == Event::MapLoaded)
        {
            updateQuestStatusEffects();
        }
    }
}

void PlayerHandler::applyQuestStatusEffects(Being *npc)
{
    const auto npcId = npc->getSubType();
    const auto effect = mActiveQuestEffects.get(npcId);
    if (effect != 0)
        npc->setStatusEffect(effect, true);
}

void PlayerHandler::updateQuestStatusEffects()
{
    auto game = Game::instance();
    if (!game)
        return;

    const auto &currentMapName = game->getCurrentMapName();
    auto updatedQuestEffects = QuestDB::getActiveEffects(mQuestVars, currentMapName);

    // Loop over all NPCs, disabling no longer active effects and enabling new ones
    for (auto actor : actorSpriteManager->getAll()) {
        if (actor->getType() != ActorSprite::NPC)
            continue;

        auto *npc = static_cast<Being *>(actor);
        const auto npcId = npc->getSubType();
        const auto oldEffect = mActiveQuestEffects.get(npcId);
        const auto newEffect = updatedQuestEffects.get(npcId);

        if (oldEffect != newEffect)
        {
            if (oldEffect != 0)
                npc->setStatusEffect(oldEffect, false);
            if (newEffect != 0)
                npc->setStatusEffect(newEffect, true);
        }
    }

    std::swap(mActiveQuestEffects, updatedQuestEffects);
}

} // namespace TmwAthena
