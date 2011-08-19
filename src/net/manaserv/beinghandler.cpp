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
#include "net/messagein.h"

#include "net/manaserv/inventoryhandler.h"
#include "net/manaserv/playerhandler.h"
#include "net/manaserv/manaserv_protocol.h"

#include "resources/colordb.h"

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

void BeingHandler::handleMessage(Net::MessageIn &msg)
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

static void handleLooks(Being *being, Net::MessageIn &msg)
{
    int lookChanges = msg.readInt8();

    if (lookChanges <= 0)
        return;

    while (lookChanges-- > 0)
    {
        unsigned int slotTypeId = msg.readInt8();
        being->setSprite(slotTypeId + FIXED_SPRITE_LAYER_SIZE,
                         msg.readInt16(), "",
                         Net::getInventoryHandler()->isWeaponSlot(slotTypeId));
    }
}

void BeingHandler::handleBeingEnterMessage(Net::MessageIn &msg)
{
    int type = msg.readInt8();
    int id = msg.readInt16();
    Being::Action action = (Being::Action)msg.readInt8();
    int px = msg.readInt16();
    int py = msg.readInt16();
    BeingDirection direction = (BeingDirection)msg.readInt8();
    Being *being;

    if (!Game::instance()->getCurrentMap()->containsPixel(px, py))
    {
        logger->log("Warning: Received GPMSG_BEING_ENTER for being id %i "
                    "with position outside the map boundaries "
                    "(x = %i, y = %i)", id, px, py);
        return;
    }

    switch (type)
    {
        case OBJECT_CHARACTER:
        {
            std::string name = msg.readString();
            if (player_node->getName() == name)
            {
                being = player_node;
                being->setId(id);
            }
            else
            {
                being = actorSpriteManager->createBeing(id,
                                                    ActorSprite::PLAYER, 0);
                being->setName(name);
            }
            int hs = msg.readInt8(), hc = msg.readInt8();
            being->setSprite(SPRITE_LAYER_HAIR, hs * -1, ColorDB::get(hc));
            being->setGender(msg.readInt8() == GENDER_MALE ?
                             GENDER_MALE : GENDER_FEMALE);
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
}

void BeingHandler::handleBeingLeaveMessage(Net::MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being)
        return;

    actorSpriteManager->destroy(being);
}

void BeingHandler::handleBeingsMoveMessage(Net::MessageIn &msg)
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
        if (being == player_node)
            continue;

        // If the position differs too much from the actual one, we resync
        // the being position
        if (flags & MOVING_POSITION)
        {
            if (!being->getMap()->containsPixel(sx, sy))
            {
                logger->log("Warning: Received GPMSG_BEINGS_MOVE for being id "
                            "%i with position outside the map boundaries "
                            "(x = %i, y = %i)", id, sx, sy);
                continue;
            }

            Vector serverPos(sx, sy);
            if (serverPos.length()
                - being->getPosition().length() > POSITION_DIFF_TOLERANCE)
                being->setPosition(serverPos);
        }

        if (flags & MOVING_DESTINATION)
        {
            if (!being->getMap()->containsPixel(dx, dy))
            {
                logger->log("Warning: Received GPMSG_BEINGS_MOVE for being id "
                            "%i with destination outside the map boundaries "
                            "(x = %i, y = %i)", id, dx, dy);
                continue;
            }

            being->setDestination(dx, dy);
        }
    }
}

void BeingHandler::handleBeingAttackMessage(Net::MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    const BeingDirection direction = (BeingDirection) msg.readInt8();
    const int attackId = msg.readInt8();

    if (!being)
        return;

    being->setDirection(direction);

    being->setAction(Being::ATTACK, attackId);
}

void BeingHandler::handleBeingsDamageMessage(Net::MessageIn &msg)
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

void BeingHandler::handleBeingActionChangeMessage(Net::MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    Being::Action action = (Being::Action) msg.readInt8();
    if (!being)
        return;

    being->setAction(action);

    if (action == Being::DEAD && being == player_node)
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

void BeingHandler::handleBeingLooksChangeMessage(Net::MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being || being->getType() != ActorSprite::PLAYER)
        return;
    handleLooks(being, msg);
    if (msg.getUnreadLength())
    {
        int style = msg.readInt16();
        int color = msg.readInt16();
        being->setSprite(SPRITE_LAYER_HAIR, style * -1, ColorDB::get(color));
    }
}

void BeingHandler::handleBeingDirChangeMessage(Net::MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being)
        return;
    int data = msg.readInt8();

    // The direction for the player's character is handled on client side.
    if (being != player_node)
        being->setDirection((BeingDirection) data);
}

} // namespace ManaServ
