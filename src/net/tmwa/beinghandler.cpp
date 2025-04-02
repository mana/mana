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

#include "net/tmwa/beinghandler.h"

#include "actorspritemanager.h"
#include "being.h"
#include "client.h"
#include "effectmanager.h"
#include "game.h"
#include "guild.h"
#include "localplayer.h"
#include "log.h"
#include "party.h"
#include "playerrelations.h"

#include "net/net.h"
#include "net/playerhandler.h"
#include "net/tmwa/messagein.h"
#include "net/tmwa/messageout.h"
#include "net/tmwa/protocol.h"

#include "resources/emotedb.h"
#include "resources/hairdb.h"
#include "resources/statuseffectdb.h"

#include <cmath>

namespace TmwAthena {

// Number of pixels where we decide that the position doesn't need to be reset.
static const float POS_DEST_DIFF_TOLERANCE = 48.0f;

BeingHandler::BeingHandler(bool enableSync):
   mSync(enableSync)
{
    static const Uint16 _messages[] =
    {
        SMSG_BEING_VISIBLE,
        SMSG_BEING_MOVE,
        SMSG_BEING_SPAWN,
        SMSG_BEING_REMOVE,
        SMSG_SKILL_DAMAGE,
        SMSG_BEING_ACTION,
        SMSG_BEING_SELFEFFECT,
        SMSG_BEING_EMOTION,
        SMSG_BEING_CHANGE_LOOKS2,
        SMSG_BEING_NAME_RESPONSE,
        SMSG_PLAYER_GUILD_PARTY_INFO,
        SMSG_BEING_CHANGE_DIRECTION,
        SMSG_PLAYER_UPDATE_1,
        SMSG_PLAYER_UPDATE_2,
        SMSG_PLAYER_MOVE,
        SMSG_PLAYER_STOP,
        SMSG_PLAYER_MOVE_TO_ATTACK,
        SMSG_PLAYER_STATUS_CHANGE,
        SMSG_BEING_STATUS_CHANGE,
        SMSG_BEING_RESURRECT,
        0
    };
    handledMessages = _messages;
}

static ActorSprite::Type typeFromJob(short job)
{
    if (job <= 25 || (job >= 4001 && job <= 4049))
        return ActorSprite::PLAYER;
    if (job >= 46 && job <= 1000)
        return ActorSprite::NPC;
    if (job > 1000 && job <= 2000)
        return ActorSprite::MONSTER;
    if (job == 45)
        return ActorSprite::PORTAL;

    return ActorSprite::UNKNOWN;
}

static Being *createBeing(int id, short job)
{
    const auto type = typeFromJob(job);
    if (type == ActorSprite::PORTAL)
        return nullptr; // Skip portals

    Being *being = actorSpriteManager->createBeing(id, type, job);

    if (type == ActorSprite::PLAYER || type == ActorSprite::NPC)
    {
        MessageOut outMsg(CMSG_NAME_REQUEST);
        outMsg.writeInt32(id);
    }

    return being;
}

static void updateBeingType(Being *being, short job)
{
    const auto type = typeFromJob(job);
    const bool typeChanged = being->getType() != type;

    being->setType(type, job);

    if (typeChanged && type == ActorSprite::PLAYER)
    {
        MessageOut outMsg(CMSG_NAME_REQUEST);
        outMsg.writeInt32(being->getId());
    }
}

static void handleMoveMessage(Map *map, Being *dstBeing,
                              Uint16 srcX, Uint16 srcY,
                              Uint16 dstX, Uint16 dstY)
{
    // Avoid dealing with flawed destination
    if (map && dstBeing && srcX && srcY && dstX && dstY)
    {
        Vector pos = map->getTileCenter(srcX, srcY);
        Vector dest = map->getTileCenter(dstX, dstY);
        Vector beingPos = dstBeing->getPosition();

        // Don't set the position as the movement algorithm
        // can guess it and it would break the animation played,
        // when we're close enough.
        if (std::abs(beingPos.x - pos.x) > POS_DEST_DIFF_TOLERANCE
            || std::abs(beingPos.y - pos.y) > POS_DEST_DIFF_TOLERANCE)
            dstBeing->setPosition(pos);

        dstBeing->setDestination(dest.x, dest.y);
    }
}

static void handlePosMessage(Map *map, Being *dstBeing, Uint16 x, Uint16 y,
                             Uint8 dir = 0)
{
    // Avoid dealing with flawed destination
    if (map && dstBeing && x && y)
    {
        Vector pos =  map->getTileCenter(x, y);
        Vector beingPos = dstBeing->getPosition();
        // Don't set the position as the movement algorithm
        // can guess it and it would break the animation played,
        // when we're close enough.
        if (std::abs(beingPos.x - pos.x) > POS_DEST_DIFF_TOLERANCE
            || std::abs(beingPos.y - pos.y) > POS_DEST_DIFF_TOLERANCE)
            dstBeing->setPosition(pos);

        // Set also the destination to the desired position.
        dstBeing->setDestination(pos.x, pos.y);

        if (dir)
            dstBeing->setDirection(dir);
    }
}

static void applyStatusEffectsByOption1(Being *being,
                                        const StatusEffectDB::OptionsMap &map,
                                        uint16_t option)
{
    for (auto &[opt, id] : map)
        being->setStatusEffect(id, option == opt);
}

static void applyStatusEffectsByOption(Being *being,
                                       const StatusEffectDB::OptionsMap &map,
                                       uint16_t option)
{
    for (auto &[opt, id] : map)
    {
        const bool enabled = (option & opt) != 0;
        being->setStatusEffect(id, enabled);
    }
}

/**
 * Maps flags or indexes to their corresponding status effect index and
 * updates the state of the given being. This is tmwAthena-specific.
 */
static void applyStatusEffects(Being *being,
                               uint16_t opt0,
                               uint16_t opt1,
                               uint16_t opt2,
                               std::optional<uint16_t> opt3 = {})
{
    applyStatusEffectsByOption(being, StatusEffectDB::opt0ToIdMap(), opt0);
    applyStatusEffectsByOption1(being, StatusEffectDB::opt1ToIdMap(), opt1);
    applyStatusEffectsByOption(being, StatusEffectDB::opt2ToIdMap(), opt2);
    if (opt3)
        applyStatusEffectsByOption(being, StatusEffectDB::opt3ToIdMap(), *opt3);
}

void BeingHandler::handleMessage(MessageIn &msg)
{
    if (!actorSpriteManager)
        return;

    int id;
    short job, gender;
    float speed;
    Uint16 headTop, headMid, headBottom;
    Uint16 shoes, gloves;
    Uint16 weapon, shield;
    Uint16 gmstatus;
    int param1;
    uint16_t opt0;
    uint16_t opt1;
    uint16_t opt2;
    uint16_t opt3;
    int type, guild;
    Uint16 status;
    Being *srcBeing, *dstBeing;
    int hairStyle, hairColor, flag;

    // Prepare useful translation variables
    Map *map = Game::instance()->getCurrentMap();

    switch (msg.getId())
    {
        case SMSG_BEING_VISIBLE:
        case SMSG_BEING_MOVE:
            // Information about a being in range
            id = msg.readInt32();
            speed = (float)msg.readInt16();
            opt1 = msg.readInt16();
            opt2 = msg.readInt16();
            opt0 = msg.readInt16();
            job = msg.readInt16();  // class

            dstBeing = actorSpriteManager->findBeing(id);

            if (!dstBeing)
            {
                // Being with id >= 110000000 and job 0 are better
                // known as ghosts, so don't create those.
                if (job == 0 && id >= 110000000)
                {
                    break;
                }

                dstBeing = createBeing(id, job);

                if (!dstBeing)
                    break;
            }

            if (msg.getId() == SMSG_BEING_VISIBLE)
            {
                dstBeing->clearPath();
                dstBeing->setAction(Being::STAND);
            }

            // Prevent division by 0 when calculating frame
            if (speed == 0)
                speed = 150.0f; // In ticks per tile * 10

            dstBeing->setMoveSpeed(Vector(speed / 10, speed / 10));
            updateBeingType(dstBeing, job);
            hairStyle = msg.readInt16();
            weapon = msg.readInt16();
            headBottom = msg.readInt16();

            if (msg.getId() == SMSG_BEING_MOVE)
            {
                msg.readInt32(); // server tick
            }

            shield = msg.readInt16();
            headTop = msg.readInt16();
            headMid = msg.readInt16();
            hairColor = msg.readInt16();
            shoes = msg.readInt16();  // clothes color - "abused" as shoes
            gloves = msg.readInt16();  // head dir - "abused" as gloves
            guild = msg.readInt32();  // guild
            if (guild == 0)
            {
                dstBeing->clearGuilds();
            }
            else
            {
                dstBeing->addGuild(Guild::getGuild(guild));
            }
            msg.readInt16();  // guild emblem
            msg.readInt16();  // manner
            opt3 = msg.readInt16();
            msg.readInt8();   // karma
            gender = msg.readInt8();

            if (dstBeing->getType() == ActorSprite::PLAYER)
            {
                dstBeing->setGender(gender == 0 ? Gender::Female
                                                : Gender::Male);
                // Set these after the gender, as the sprites may be gender-specific
                dstBeing->setSprite(SPRITE_HAIR, hairStyle * -1,
                                    hairDB.getHairColor(hairColor));
                dstBeing->setSprite(SPRITE_BOTTOMCLOTHES, headBottom);
                dstBeing->setSprite(SPRITE_TOPCLOTHES, headMid);
                dstBeing->setSprite(SPRITE_HAT, headTop);
                dstBeing->setSprite(SPRITE_SHOE, shoes);
                dstBeing->setSprite(SPRITE_GLOVES, gloves);
                dstBeing->setSprite(SPRITE_WEAPON, weapon, "", true);
                dstBeing->setSprite(SPRITE_SHIELD, shield);
            }

            if (msg.getId() == SMSG_BEING_MOVE)
            {
                Uint16 srcX, srcY, dstX, dstY;
                msg.readCoordinatePair(srcX, srcY, dstX, dstY);
                handleMoveMessage(map, dstBeing, srcX, srcY, dstX, dstY);
            }
            else
            {
                Uint8 dir;
                Uint16 x, y;
                msg.readCoordinates(x, y, dir);
                handlePosMessage(map, dstBeing, x, y, dir);
            }

            msg.readInt8();   // unknown
            msg.readInt8();   // unknown
            msg.readInt8();   // unknown / sit

            applyStatusEffects(dstBeing, opt0, opt1, opt2, opt3);
            break;

        case SMSG_BEING_SPAWN:
            /*
             * TODO: This packet might need handling in the future.
             */
             // Do nothing.
             break;

        case SMSG_BEING_REMOVE:
            // A being should be removed or has died
            id = msg.readInt32();
            dstBeing = actorSpriteManager->findBeing(id);
            if (!dstBeing)
                break;

            // If this is player's current target, clear it.
            if (dstBeing == local_player->getTarget())
                local_player->stopAttack();

            if (msg.readInt8() == 1)
                dstBeing->setAction(Being::DEAD);
            else
                actorSpriteManager->destroyActor(dstBeing);

            break;

        case SMSG_BEING_RESURRECT:
            // A being changed mortality status
            id = msg.readInt32();
            dstBeing = actorSpriteManager->findBeing(id);
            if (!dstBeing)
                break;

            // If this is player's current target, clear it.
            if (dstBeing == local_player->getTarget())
                local_player->stopAttack();

            if (msg.readInt8() == 1)
                dstBeing->setAction(Being::STAND);

            break;

        case SMSG_SKILL_DAMAGE:
        {
            msg.readInt16(); // Skill Id
            srcBeing = actorSpriteManager->findBeing(msg.readInt32());
            dstBeing = actorSpriteManager->findBeing(msg.readInt32());
            msg.readInt32(); // Server tick
            int attackSpeed = msg.readInt32(); // src speed
            msg.readInt32(); // dst speed
            param1 = msg.readInt32(); // Damage
            msg.readInt16(); // Skill level
            msg.readInt16(); // Div
            msg.readInt8(); // Skill hit/type (?)
            if (attackSpeed && srcBeing && srcBeing != local_player)
                srcBeing->setAttackSpeed(attackSpeed);
            if (dstBeing)
                dstBeing->takeDamage(srcBeing, param1, Being::HIT); // Perhaps a new skill attack type should be created and used?
            if (srcBeing)
                srcBeing->handleAttack(dstBeing, param1);
            break;
        }
        case SMSG_BEING_ACTION:
            srcBeing = actorSpriteManager->findBeing(msg.readInt32());
            dstBeing = actorSpriteManager->findBeing(msg.readInt32());
            msg.readInt32();   // server tick
            msg.readInt32();   // src speed
            msg.readInt32();   // dst speed
            param1 = msg.readInt16();
            msg.readInt16();  // param 2
            type = msg.readInt8();
            msg.readInt16();  // param 3

            switch (type)
            {
                case Being::HIT: // Damage
                case Being::CRITICAL: // Critical Damage
                case Being::MULTI: // Critical Damage
                case Being::REFLECT: // Reflected Damage
                case Being::FLEE: // Lucky Dodge
                    if (dstBeing)
                        dstBeing->takeDamage(srcBeing, param1,
                                static_cast<Being::AttackType>(type));
                    if (srcBeing)
                        srcBeing->handleAttack(dstBeing, param1);
                    break;

                case 0x02: // Sit
                    if (srcBeing)
                        srcBeing->setAction(Being::SIT);
                    break;

                case 0x03: // Stand up
                    if (srcBeing)
                        srcBeing->setAction(Being::STAND);
                    break;
            }
            break;

        case SMSG_BEING_SELFEFFECT:
        {
            id = (Uint32)msg.readInt32();
            Being* being = actorSpriteManager->findBeing(id);
            if (!being)
                break;

            int effectType = msg.readInt32();

            effectManager->trigger(effectType, being);
            break;
        }

        case SMSG_BEING_EMOTION:
            if (!(dstBeing = actorSpriteManager->findBeing(msg.readInt32())))
            {
                break;
            }

            if (player_relations.hasPermission(dstBeing, PlayerPermissions::EMOTE))
            {
                const int fx = EmoteDB::get(msg.readInt8() - 1).effectId;
                effectManager->trigger(fx, dstBeing);
            }

            break;

        case SMSG_BEING_CHANGE_LOOKS2:
        {
            if (!(dstBeing = actorSpriteManager->findBeing(msg.readInt32())))
            {
                break;
            }

            const LOOK type = static_cast<LOOK>(msg.readInt8());
            const int id = msg.readInt16();
            const int id2 = msg.readInt16();

            switch (type)
            {
                case LOOK::BASE:
                    updateBeingType(dstBeing, id);
                    break;
                case LOOK::HAIR:
                {
                    // const int look = id / 256;
                    const int hair = id % 256;
                    dstBeing->setSpriteID(SPRITE_HAIR, hair * -1);
                    break;
                }
                case LOOK::WEAPON:      // Weapon ID in id, Shield ID in id2
                    dstBeing->setSprite(SPRITE_WEAPON, id, std::string(), true);
                    dstBeing->setSprite(SPRITE_SHIELD, id2);
                    break;
                case LOOK::HEAD_BOTTOM: // Change lower headgear for eAthena, pants for us
                    dstBeing->setSprite(SPRITE_BOTTOMCLOTHES, id);
                    break;
                case LOOK::HEAD_TOP:    // Change upper headgear for eAthena, hat for us
                    dstBeing->setSprite(SPRITE_HAT, id);
                    break;
                case LOOK::HEAD_MID:    // Change middle headgear for eathena, armor for us
                    dstBeing->setSprite(SPRITE_TOPCLOTHES, id);
                    break;
                case LOOK::HAIR_COLOR:
                    dstBeing->setSpriteColor(SPRITE_HAIR,
                                             hairDB.getHairColor(id));
                    break;
                case LOOK::CLOTHES_COLOR:
                    // ignoring it
                    break;
                case LOOK::SHIELD:
                    dstBeing->setSprite(SPRITE_SHIELD, id);
                    break;
                case LOOK::SHOES:
                    dstBeing->setSprite(SPRITE_SHOE, id);
                    break;
                case LOOK::GLOVES:
                    dstBeing->setSprite(SPRITE_GLOVES, id);
                    break;
                case LOOK::CAPE:
                    dstBeing->setSprite(SPRITE_CAPE, id);
                    break;
                case LOOK::MISC1:
                    dstBeing->setSprite(SPRITE_MISC1, id);
                    break;
                case LOOK::MISC2:
                    dstBeing->setSprite(SPRITE_MISC2, id);
                    break;
                default:
                    logger->log("SMSG_BEING_CHANGE_LOOKS2: unsupported type: "
                                "%d, id: %d", static_cast<int>(type), id);
                    break;
            }
        }
            break;

        case SMSG_BEING_NAME_RESPONSE:
            if ((dstBeing = actorSpriteManager->findBeing(msg.readInt32())))
            {
                dstBeing->setName(msg.readString(24));
            }
            break;
        case SMSG_PLAYER_GUILD_PARTY_INFO:
            if ((dstBeing = actorSpriteManager->findBeing(msg.readInt32())))
            {
                dstBeing->setPartyName(msg.readString(24));
                dstBeing->setGuildName(msg.readString(24));
                dstBeing->setGuildPos(msg.readString(24));
                msg.readString(24); // Discard this
            }
            break;
        case SMSG_BEING_CHANGE_DIRECTION:
        {
            if (!(dstBeing = actorSpriteManager->findBeing(msg.readInt32())))
            {
                break;
            }

            msg.readInt16(); // unused
            Uint8 dir = msg.readInt8();
            if (dir)
                dstBeing->setDirection(dir);
        }
            break;

        case SMSG_PLAYER_UPDATE_1:
        case SMSG_PLAYER_UPDATE_2:
        case SMSG_PLAYER_MOVE:
            // An update about a player, potentially including movement.
            id = msg.readInt32();
            speed = msg.readInt16();
            opt1 = msg.readInt16();
            opt2 = msg.readInt16();
            opt0 = msg.readInt16();
            job = msg.readInt16();

            dstBeing = actorSpriteManager->findBeing(id);

            if (!dstBeing)
            {
                dstBeing = createBeing(id, job);

                if (!dstBeing)
                    break;
            }

            if (Party *party = local_player->getParty())
                if (party->isMember(id))
                    dstBeing->setParty(party);

            // The original speed is ticks per tile * 10
            if (speed)
                dstBeing->setMoveSpeed(Vector(speed / 10, speed / 10));
            else
                dstBeing->setMoveSpeed(Net::getPlayerHandler()->getDefaultMoveSpeed());

            updateBeingType(dstBeing, job);
            hairStyle = msg.readInt16();
            weapon = msg.readInt16();
            shield = msg.readInt16();
            headBottom = msg.readInt16();

            if (msg.getId() == SMSG_PLAYER_MOVE)
            {
                msg.readInt32(); // server tick
            }

            headTop = msg.readInt16();
            headMid = msg.readInt16();
            hairColor = msg.readInt16();
            msg.readInt16();  // clothes_color
            msg.readInt8();   // head_dir
            msg.readInt8();   // unused2
            msg.readInt32();  // guild
            msg.readInt16();  // emblem
            msg.readInt16();  // manner
            opt3 = msg.readInt16();
            msg.readInt8();   // karma
            dstBeing->setGender(msg.readInt8() == 0 ? Gender::Female
                                                    : Gender::Male);
            // Set these after the gender, as the sprites may be gender-specific
            dstBeing->setSprite(SPRITE_WEAPON, weapon, "", true);
            dstBeing->setSprite(SPRITE_SHIELD, shield);
            //dstBeing->setSprite(SPRITE_SHOE, shoes);
            dstBeing->setSprite(SPRITE_BOTTOMCLOTHES, headBottom);
            dstBeing->setSprite(SPRITE_TOPCLOTHES, headMid);
            dstBeing->setSprite(SPRITE_HAT, headTop);
            //dstBeing->setSprite(SPRITE_GLOVES, gloves);
            //dstBeing->setSprite(SPRITE_CAPE, cape);
            //dstBeing->setSprite(SPRITE_MISC1, misc1);
            //dstBeing->setSprite(SPRITE_MISC2, misc2);
            dstBeing->setSprite(SPRITE_HAIR, hairStyle * -1,
                                hairDB.getHairColor(hairColor));

            if (msg.getId() == SMSG_PLAYER_MOVE)
            {
                Uint16 srcX, srcY, dstX, dstY;
                msg.readCoordinatePair(srcX, srcY, dstX, dstY);
                handleMoveMessage(map, dstBeing, srcX, srcY, dstX, dstY);
            }
            else
            {
                Uint8 dir;
                Uint16 x, y;
                msg.readCoordinates(x, y, dir);
                handlePosMessage(map, dstBeing, x, y, dir);
            }

            gmstatus = msg.readInt16();
            if (gmstatus & 0x80)
                dstBeing->setGM(true);

            if (msg.getId() == SMSG_PLAYER_UPDATE_1)
            {
                switch (msg.readInt8())
                {
                    case 1:
                        dstBeing->setAction(Being::DEAD);
                        break;

                    case 2:
                        dstBeing->setAction(Being::SIT);
                        break;
                }
            }
            else if (msg.getId() == SMSG_PLAYER_MOVE)
            {
                msg.readInt8(); // five
            }

            msg.readInt8();   // Lv
            msg.readInt8();   // unused

            applyStatusEffects(dstBeing, opt0, opt1, opt2, opt3);
            break;

        case SMSG_PLAYER_STOP:
            /*
             *  Instruction from server to stop walking at x, y.
             *
             *  Some people like having this enabled.  Others absolutely
             *  despise it.  So I'm setting to so that it only affects the
             *  local player if the person has set a key "EnableSync" to "1"
             *  in their config.xml file.
             *
             *  This packet will be honored for all other beings, regardless
             *  of the config setting.
             */

            id = msg.readInt32();
            if (mSync || id != local_player->getId())
            {
                dstBeing = actorSpriteManager->findBeing(id);
                if (dstBeing)
                {
                    Uint16 x = msg.readInt16();
                    Uint16 y = msg.readInt16();
                    handlePosMessage(map, dstBeing, x, y);
                }
            }
            break;

        case SMSG_PLAYER_MOVE_TO_ATTACK:
            /*
             * This is an *advisory* message, telling the client that
             * it needs to move the character before attacking
             * a target (out of range, obstruction in line of fire).
             * We can safely ignore this...
             */
            break;

        case SMSG_PLAYER_STATUS_CHANGE:
            // Change in players' flags
            id = msg.readInt32();
            dstBeing = actorSpriteManager->findBeing(id);
            if (!dstBeing)
                break;

            opt1 = msg.readInt16();
            opt2 = msg.readInt16();
            opt0 = msg.readInt16();
            msg.readInt8(); // zero

            applyStatusEffects(dstBeing, opt0, opt1, opt2);
            break;

        case SMSG_BEING_STATUS_CHANGE:
            // Status change
            status = msg.readInt16();
            id = msg.readInt32();
            flag = msg.readInt8(); // 0: stop, 1: start

            dstBeing = actorSpriteManager->findBeing(id);
            if (dstBeing)
                dstBeing->setStatusEffect(status, flag);
            break;
    }
}

} // namespace TmwAthena
