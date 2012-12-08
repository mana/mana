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
#include "client.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "particle.h"

#include "gui/okdialog.h"

#include "net/net.h"

#include "net/manaserv/inventoryhandler.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/playerhandler.h"
#include "net/manaserv/manaserv_protocol.h"

#include "resources/hairdb.h"

#include "utils/gettext.h"

#define POSITION_DIFF_TOLERANCE 48

namespace ManaServ {

BeingHandler::BeingHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_BEING_ATTACK,
        GPMSG_BEING_ENTER,
        GPMSG_BEING_LEAVE,
        GPMSG_BEINGS_MOVE,
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
        case GPMSG_BEINGS_MOVE:
            handleBeingsMoveMessage(msg);
            break;
        case GPMSG_BEING_ATTACK:
            handleBeingAttackMessage(msg);
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
    int lookChanges = msg.readInt8();

    if (lookChanges <= 0)
        return;

    while (lookChanges-- > 0)
    {
        unsigned int slotTypeId = msg.readInt8();
        Net::InventoryHandler *ih = Net::getInventoryHandler();
        std::string name = ih->getSlotName(slotTypeId);

        int layer = 0;
        if (name == "Hand 1/2")
            layer = SPRITE_LAYER_WEAPON;
        else if (name == "Hand 2/2")
            layer = SPRITE_LAYER_SHIELD;
        else if (name == "Torso")
            layer = SPRITE_LAYER_TORSO;
        else if (name == "Head")
            layer = SPRITE_LAYER_HEAD;
        else if (name == "Legs")
            layer = SPRITE_LAYER_LEGS;
        else if (name == "Feet")
            layer = SPRITE_LAYER_FEET;
        else
            layer = slotTypeId + FIXED_SPRITE_LAYER_SIZE;

        being->setSprite(layer, msg.readInt16(), "", ih->isWeaponSlot(slotTypeId));
    }
}

void BeingHandler::handleBeingEnterMessage(MessageIn &msg)
{
    int type = msg.readInt8();
    int id = msg.readInt16();
    Being::Action action = (Being::Action)msg.readInt8();
    int px = msg.readInt16();
    int py = msg.readInt16();
    BeingDirection direction = (BeingDirection)msg.readInt8();
    Gender gender;
    int genderAsInt = msg.readInt8();
    if (genderAsInt == ::GENDER_FEMALE)
        gender = ::GENDER_FEMALE;
    else if (genderAsInt == ::GENDER_MALE)
        gender = ::GENDER_MALE;
    else
        gender = ::GENDER_UNSPECIFIED;
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
            int hs = msg.readInt8(), hc = msg.readInt8();
            being->setSprite(SPRITE_LAYER_HAIR, hs * -1,
                             hairDB.getHairColor(hc));
            handleLooks(being, msg);
        } break;

        case OBJECT_MONSTER:
        case OBJECT_NPC:
        {
            int subtype = msg.readInt16();
            being = actorSpriteManager->createBeing(id, type == OBJECT_MONSTER
                           ? ActorSprite::MONSTER : ActorSprite::NPC, subtype);
            std::string name = msg.readString();
            if (name.length() > 0) being->setName(name);
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

    actorSpriteManager->destroy(being);
}

void BeingHandler::handleBeingsMoveMessage(MessageIn &msg)
{
    while (msg.getUnreadLength())
    {
        int id = msg.readInt16();
        int flags = msg.readInt8();
        Being *being = actorSpriteManager->findBeing(id);
        int sx = 0, sy = 0, dx = 0, dy = 0, speed = 0;

        if ((!flags & (MOVING_POSITION | MOVING_DESTINATION)))
            continue;

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

void BeingHandler::handleBeingAttackMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    const BeingDirection direction = (BeingDirection) msg.readInt8();
    const int attackId = msg.readInt8();

    if (!being)
        return;

    being->setDirection(direction);

    being->setAction(Being::ATTACK, attackId);
}

void BeingHandler::handleBeingsDamageMessage(MessageIn &msg)
{
    while (msg.getUnreadLength())
    {
        Being *being = actorSpriteManager->findBeing(msg.readInt16());
        int damage = msg.readInt16();
        if (being)
        {
            being->takeDamage(0, damage, Being::HIT);
        }
    }
}

void BeingHandler::handleBeingActionChangeMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    Being::Action action = (Being::Action) msg.readInt8();
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
        OkDialog *dlg = new OkDialog(_("You Died"), message, false);
        dlg->addActionListener(&(ManaServ::respawnListener));
    }
}

void BeingHandler::handleBeingLooksChangeMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being || being->getType() != ActorSprite::PLAYER)
        return;
    handleLooks(being, msg);
    if (msg.getUnreadLength())
    {
        int style = msg.readInt16();
        int color = msg.readInt16();
        being->setSprite(SPRITE_LAYER_HAIR, style * -1,
                         hairDB.getHairColor(color));
    }
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
