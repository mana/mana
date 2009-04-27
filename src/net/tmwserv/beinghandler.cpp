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

#include "net/tmwserv/beinghandler.h"

#include "net/tmwserv/protocol.h"

#include "net/messagein.h"

#include "being.h"
#include "beingmanager.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "main.h"
#include "npc.h"
#include "particle.h"
#include "sound.h"

#include "gui/okdialog.h"

#include "utils/gettext.h"

#include "net/tmwserv/gameserver/player.h"

namespace TmwServ {

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

static void handleLooks(Player *being, MessageIn &msg)
{
    // Order of sent slots. Has to be in sync with the server code.
    static int const nb_slots = 4;
    static int const slots[nb_slots] =
        { Being::WEAPON_SPRITE, Being::HAT_SPRITE, Being::TOPCLOTHES_SPRITE,
          Being::BOTTOMCLOTHES_SPRITE };

    int mask = msg.readInt8();

    if (mask & (1 << 7))
    {
        // The equipment has to be cleared first.
        for (int i = 0; i < nb_slots; ++i)
        {
            being->setSprite(slots[i], 0);
        }
    }

    // Fill slots enumerated by the bitmask.
    for (int i = 0; i < nb_slots; ++i)
    {
        if (!(mask & (1 << i))) continue;
        int id = msg.readInt16();
        being->setSprite(slots[i], id);
    }
}

void BeingHandler::handleBeingEnterMessage(MessageIn &msg)
{
    int type = msg.readInt8();
    int id = msg.readInt16();
    Being::Action action = (Being::Action)msg.readInt8();
    int px = msg.readInt16();
    int py = msg.readInt16();
    Being *being;

    switch (type)
    {
        case OBJECT_PLAYER:
        {
            std::string name = msg.readString();
            if (player_node->getName() == name)
            {
                being = player_node;
                being->setId(id);
            }
            else
            {
                being = beingManager->createBeing(id, Being::PLAYER, 0);
                being->setName(name);
            }
            Player *p = static_cast< Player * >(being);
            int hs = msg.readInt8(), hc = msg.readInt8();
            p->setHairStyle(hs, hc);
            p->setGender(msg.readInt8() == GENDER_MALE ?
                    GENDER_MALE : GENDER_FEMALE);
            handleLooks(p, msg);
        } break;

        case OBJECT_MONSTER:
        case OBJECT_NPC:
        {
            int subtype = msg.readInt16();
            being = beingManager->createBeing(id, type == OBJECT_MONSTER ?
                                    Being::MONSTER : Being::NPC, subtype);
            std::string name = msg.readString();
            if (name.length() > 0) being->setName(name);
        } break;

        default:
            return;
    }

    being->setPosition(px, py);
    being->setDestination(px, py);
    being->setAction(action);
}

void BeingHandler::handleBeingLeaveMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readInt16());
    if (!being)
        return;

    beingManager->destroyBeing(being);
}

void BeingHandler::handleBeingsMoveMessage(MessageIn &msg)
{
    while (msg.getUnreadLength())
    {
        int id = msg.readInt16();
        int flags = msg.readInt8();
        Being *being = beingManager->findBeing(id);
        int sx = 0;
        int sy = 0;
        int dx = 0;
        int dy = 0;
        int speed = 0;

        printf("handleBeingsMoveMessage for %p (%s | %s)\n",
                (void*) being,
                (flags & MOVING_POSITION) ? "pos" : "",
                (flags & MOVING_DESTINATION) ? "dest" : "");

        if (flags & MOVING_POSITION)
        {
            Uint16 sx2, sy2;
            msg.readCoordinates(sx2, sy2);
            sx = sx2 * 32 + 16;
            sy = sy2 * 32 + 16;
            speed = msg.readInt8();
        }
        if (flags & MOVING_DESTINATION)
        {
            dx = msg.readInt16();
            dy = msg.readInt16();
            if (!(flags & MOVING_POSITION))
            {
                sx = dx;
                sy = dy;
            }
        }
        if (!being || !(flags & (MOVING_POSITION | MOVING_DESTINATION)))
        {
            continue;
        }
        if (speed)
        {
            /* The speed on the server is the cost of moving from one tile to
             * the next. Beings get 1000 cost units per second. The speed is
             * transferred as devided by 10, so that slower speeds fit in a
             * byte. Here we convert the speed to pixels per second.
             */
            const float tilesPerSecond = 100.0f / speed;
            being->setWalkSpeed((int) (tilesPerSecond * 32));
        }

        // Ignore messages from the server for the local player
        if (being == player_node)
            continue;

        // If being is a player, and he only moves a little, its ok to be a little out of sync
        if (being->getType() == Being::PLAYER && abs(being->getPixelX() - dx) +
                                                 abs(being->getPixelY() - dy) < 2 * 32 &&
                                                 (dx != being->getDestination().x && dy != being->getDestination().y))
        {
            being->setDestination(being->getPixelX(),being->getPixelY());
            continue;
        }
        if (abs(being->getPixelX() - sx) +
                abs(being->getPixelY() - sy) > 10 * 32)
        {
            // Too large a desynchronization.
            being->setPosition(sx, sy);
            being->setDestination(dx, dy);
        }
        else if (!(flags & MOVING_POSITION))
        {
            being->setDestination(dx, dy);
        }
        else if (!(flags & MOVING_DESTINATION))
        {
            being->adjustCourse(sx, sy);
        }
        else
        {
            being->adjustCourse(sx, sy, dx, dy);
        }
    }
}

void BeingHandler::handleBeingAttackMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readInt16());
    const int direction = msg.readInt8();
    const int attackType = msg.readInt8();

    if (!being)
        return;

    switch (direction)
    {
        case DIRECTION_UP: being->setDirection(Being::UP); break;
        case DIRECTION_DOWN: being->setDirection(Being::DOWN); break;
        case DIRECTION_LEFT: being->setDirection(Being::LEFT); break;
        case DIRECTION_RIGHT: being->setDirection(Being::RIGHT); break;
    }

    being->setAction(Being::ATTACK, attackType);
}

void BeingHandler::handleBeingsDamageMessage(MessageIn &msg)
{
    while (msg.getUnreadLength())
    {
        Being *being = beingManager->findBeing(msg.readInt16());
        int damage = msg.readInt16();
        if (being)
        {
            being->takeDamage(0, damage, Being::HIT);
        }
    }
}

void BeingHandler::handleBeingActionChangeMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readInt16());
    Being::Action action = (Being::Action) msg.readInt8();
    if (!being)
        return;

    being->setAction(action);

    if (action == Being::DEAD && being == player_node)
    {
        static char const *const deadMsg[] =
        {
            _("You are dead."),
            _("We regret to inform you that your character was killed in battle."),
            _("You are not that alive anymore."),
            _("The cold hands of the grim reaper are grabbing for your soul."),
            _("Game Over!"),
            _("No, kids. Your character did not really die. It... err... went to a better place."),
            _("Your plan of breaking your enemies weapon by bashing it with your throat failed."),
            _("I guess this did not run too well."),
            _("Do you want your possessions identified?"), // Nethack reference
            _("Sadly, no trace of you was ever found..."), // Secret of Mana reference
            _("Annihilated."), // Final Fantasy VI reference
            _("Looks like you got your head handed to you."), //Earthbound reference
            _("You screwed up again, dump your body down the tubes and get you another one.") // Leisure Suit Larry 1 Reference

        };
        std::string message(deadMsg[rand()%13]);
        message.append(_(" Press OK to respawn"));
        OkDialog *dlg = new OkDialog(_("You died"), message);
        dlg->addActionListener(&(Net::GameServer::Player::respawnListener));
    }
}

void BeingHandler::handleBeingLooksChangeMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readInt16());
    if (!being || being->getType() != Being::PLAYER)
        return;
    Player *player = static_cast<Player *>(being);
    handleLooks(player, msg);
    if (msg.getUnreadLength())
    {
        int style = msg.readInt16();
        int color = msg.readInt16();
        player->setHairStyle(style, color);
        player->setGender((Gender)msg.readInt16());
    }
}

void BeingHandler::handleBeingDirChangeMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readInt16());
    if (!being)
        return;
    int data = msg.readInt8();
    switch (data)
    {
       case DIRECTION_UP: being->setDirection(Being::UP); break;
       case DIRECTION_DOWN: being->setDirection(Being::DOWN); break;
       case DIRECTION_LEFT: being->setDirection(Being::LEFT); break;
       case DIRECTION_RIGHT: being->setDirection(Being::RIGHT); break;
    }
}

} // namespace TmwServ
