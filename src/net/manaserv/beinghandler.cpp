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

#include "net/manaserv/beinghandler.h"

#include "actorspritemanager.h"
#include "being.h"
#include "effectmanager.h"
#include "localplayer.h"

#include "gui/okdialog.h"

#include "net/net.h"

#include "net/inventoryhandler.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/playerhandler.h"
#include "net/manaserv/manaserv_protocol.h"

#include "playerrelations.h"
#include "resources/abilitydb.h"
#include "resources/emotedb.h"
#include "resources/hairdb.h"

#include "utils/gettext.h"

#define POSITION_DIFF_TOLERANCE 48

namespace ManaServ {

BeingHandler::BeingHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_BEING_ENTER,
        GPMSG_BEING_LEAVE,
        GPMSG_BEING_EMOTE,
        GPMSG_BEINGS_MOVE,
        GPMSG_BEING_ABILITY_POINT,
        GPMSG_BEING_ABILITY_BEING,
        GPMSG_BEING_ABILITY_DIRECTION,
        GPMSG_BEINGS_DAMAGE,
        GPMSG_BEING_ACTION_CHANGE,
        GPMSG_BEING_LOOKS_CHANGE,
        GPMSG_BEING_DIR_CHANGE,
        0
    };
    handledMessages = _messages;
}

void BeingHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_BEING_ENTER:
            handleBeingEnterMessage(msg);
            break;
        case GPMSG_BEING_LEAVE:
            handleBeingLeaveMessage(msg);
            break;
        case GPMSG_BEING_EMOTE:
            handleBeingEmoteMessage(msg);
            break;
        case GPMSG_BEINGS_MOVE:
            handleBeingsMoveMessage(msg);
            break;
        case GPMSG_BEING_ABILITY_POINT:
            handleBeingAbilityPointMessage(msg);
            break;
        case GPMSG_BEING_ABILITY_BEING:
            handleBeingAbilityBeingMessage(msg);
            break;
        case GPMSG_BEING_ABILITY_DIRECTION:
            handleBeingAbilityDirectionMessage(msg);
            break;
        case GPMSG_BEINGS_DAMAGE:
            handleBeingsDamageMessage(msg);
            break;
        case GPMSG_BEING_ACTION_CHANGE:
            handleBeingActionChangeMessage(msg);
            break;
        case GPMSG_BEING_LOOKS_CHANGE:
            handleBeingLooksChangeMessage(msg);
            break;
        case GPMSG_BEING_DIR_CHANGE:
            handleBeingDirChangeMessage(msg);
            break;
    }
}

static void handleLooks(Being *being, MessageIn &msg)
{
    const int hairStyle = msg.readInt8();
    const int hairColor = msg.readInt8();
    being->setSprite(SPRITE_LAYER_HAIR, hairStyle * -1,
                     hairDB.getHairColor(hairColor));

    std::map<unsigned, int> equippedSlots;

    if (msg.getUnreadLength() > 1) {
        int equippedSlotCount = msg.readInt8();
        while (equippedSlotCount-- > 0) {
            unsigned slot = msg.readInt8();
            int itemId = msg.readInt16();
            equippedSlots[slot] = itemId;
        }
    }

    unsigned endSlot = equippedSlots.empty() ? 0 : equippedSlots.rbegin()->first + 1;
    if (being->getSpriteCount() > endSlot + FIXED_SPRITE_LAYER_SIZE)
        endSlot = being->getSpriteCount() - FIXED_SPRITE_LAYER_SIZE;

    for (unsigned slot = 0; slot < endSlot; ++slot)
    {
        auto it = equippedSlots.find(slot);
        if (it == equippedSlots.end())
        {
            being->setSprite(slot + FIXED_SPRITE_LAYER_SIZE, 0);
        }
        else
        {
            being->setSprite(slot + FIXED_SPRITE_LAYER_SIZE,
                             it->second,
                             std::string(),
                             Net::getInventoryHandler()->isWeaponSlot(slot));
        }
    }
}

void BeingHandler::handleBeingEnterMessage(MessageIn &msg)
{
    const int type = msg.readInt8();
    const int id = msg.readInt16();
    const auto action = static_cast<Being::Action>(msg.readInt8());
    const int px = msg.readInt16();
    const int py = msg.readInt16();
    const auto direction = static_cast<BeingDirection>(msg.readInt8());
    const Gender gender = getGender(msg.readInt8());

    Being *being;

    switch (type)
    {
        case OBJECT_CHARACTER:
        {
            std::string name = msg.readString();
            if (local_player->getName() == name)
            {
                being = local_player;
                being->setId(id);
            }
            else
            {
                being = actorSpriteManager->createBeing(id,
                                                    ActorSprite::PLAYER, 0);
                being->setName(name);
            }

            handleLooks(being, msg);
        } break;

        case OBJECT_MONSTER:
        case OBJECT_NPC:
        {
            int subtype = msg.readInt16();
            being = actorSpriteManager->createBeing(id, type == OBJECT_MONSTER
                           ? ActorSprite::MONSTER : ActorSprite::NPC, subtype);
            std::string name = msg.readString();
            if (!name.empty())
                being->setName(name);
        } break;

        default:
            return;
    }

    being->setPosition(px, py);
    being->setDestination(px, py);
    being->setDirection(direction);
    being->setAction(action);
    being->setGender(gender);
}

void BeingHandler::handleBeingLeaveMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being)
        return;

    actorSpriteManager->destroyActor(being);
}

void BeingHandler::handleBeingEmoteMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being)
        return;

    if (player_relations.hasPermission(being, PlayerPermissions::EMOTE))
    {
        const int fx = EmoteDB::get(msg.readInt8() - 1).effectId;
        effectManager->trigger(fx, being);
    }
}

void BeingHandler::handleBeingsMoveMessage(MessageIn &msg)
{
    while (msg.getUnreadLength())
    {
        const int id = msg.readInt16();
        const int flags = msg.readInt8();

        if (!(flags & (MOVING_POSITION | MOVING_DESTINATION)))
            continue;

        int sx = 0, sy = 0, dx = 0, dy = 0, speed = 0;

        if (flags & MOVING_POSITION)
        {
            sx = msg.readInt16();
            sy = msg.readInt16();
        }

        if (flags & MOVING_DESTINATION)
        {
            dx = msg.readInt16();
            dy = msg.readInt16();
            speed = msg.readInt8();
        }

        Being *being = actorSpriteManager->findBeing(id);
        if (!being)
            continue;

        if (speed)
        {
           /*
            * The being's speed is transfered in tiles per second * 10
            * to keep it transferable in a Byte.
            * We set it back to tiles per second and in a float.
            */
            float speedTilesSeconds = (float) speed / 10;
            being->setMoveSpeed(Vector(speedTilesSeconds, speedTilesSeconds,
                                       0));
        }

        // Ignore messages from the server for the local player
        if (being == local_player)
            continue;

        // If the position differs too much from the actual one, we resync
        // the being position
        if (flags & MOVING_POSITION)
        {
            Vector serverPos(sx, sy);
            if (serverPos.length()
                - being->getPosition().length() > POSITION_DIFF_TOLERANCE)
                being->setPosition(serverPos);
        }

        if (flags & MOVING_DESTINATION)
            being->setDestination(dx, dy);
    }
}

void BeingHandler::handleBeingAbilityPointMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being)
        return;

    const int abilityId = msg.readInt8();
    const int x = msg.readInt16();
    const int y = msg.readInt16();

    being->lookAt(Vector(x, y));

    if (auto ability = AbilityDB::get(abilityId))
        being->setAction(ability->useAction);
}

void BeingHandler::handleBeingAbilityBeingMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being)
        return;

    const int abilityId = msg.readInt8();
    const int targetId = msg.readInt16();

    if (Being *target = actorSpriteManager->findBeing(targetId))
        being->lookAt(target->getPosition());

    if (auto ability = AbilityDB::get(abilityId))
        being->setAction(ability->useAction);
}

void BeingHandler::handleBeingAbilityDirectionMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being)
        return;

    const int abilityId = msg.readInt8();
    const int direction = msg.readInt8();

    being->setDirection(direction);

    if (auto ability = AbilityDB::get(abilityId))
        being->setAction(ability->useAction);
}

void BeingHandler::handleBeingsDamageMessage(MessageIn &msg)
{
    while (msg.getUnreadLength())
    {
        Being *being = actorSpriteManager->findBeing(msg.readInt16());
        int damage = msg.readInt16();
        if (being)
            being->takeDamage(nullptr, damage, Being::HIT);
    }
}

void BeingHandler::handleBeingActionChangeMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    auto action = (Being::Action) msg.readInt8();
    if (!being)
        return;

    being->setAction(action);

    if (action == Being::DEAD && being == local_player)
    {
        static char const *const deadMsg[] =
        {
            _("You are dead."),
            _("We regret to inform you that your character was killed in "
              "battle."),
            _("You are not that alive anymore."),
            _("The cold hands of the grim reaper are grabbing for your soul."),
            _("Game Over!"),
            _("No, kids. Your character did not really die. It... err... "
              "went to a better place."),
            _("Your plan of breaking your enemies weapon by bashing it with "
              "your throat failed."),
            _("I guess this did not run too well."),
            _("Do you want your possessions identified?"), // Nethack reference
            _("Sadly, no trace of you was ever found..."), // Secret of Mana reference
            _("Annihilated."), // Final Fantasy VI reference
            _("Looks like you got your head handed to you."), // Earthbound reference
            _("You screwed up again, dump your body down the tubes and get "
              "you another one.") // Leisure Suit Larry 1 Reference

        };
        std::string message(deadMsg[rand()%13]);
        message.append(std::string(" ") + _("Press OK to respawn."));
        auto *dlg = new OkDialog(_("You Died"), message, false);
        dlg->addActionListener(&(ManaServ::respawnListener));
    }
}

void BeingHandler::handleBeingLooksChangeMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being || being->getType() != ActorSprite::PLAYER)
        return;

    handleLooks(being, msg);
}

void BeingHandler::handleBeingDirChangeMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being)
        return;
    int data = msg.readInt8();

    // The direction for the player's character is handled on client side.
    if (being != local_player)
        being->setDirection((BeingDirection) data);
}

} // namespace ManaServ
