/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "localplayer.h"

#include "game.h"
#include "equipment.h"
#include "floor_item.h"
#include "inventory.h"
#include "item.h"
#include "main.h"
#include "sound.h"

#include "net/messageout.h"
#include "net/protocol.h"

LocalPlayer *player_node = NULL;

LocalPlayer::LocalPlayer(Uint32 id, Uint16 job, Map *map):
    Player(id, job, map),
    mInventory(new Inventory()),
    mEquipment(new Equipment()),
    mTarget(NULL),
    mTrading(false)
{
}

LocalPlayer::~LocalPlayer()
{
    delete mInventory;
}

void LocalPlayer::logic()
{
    switch (action) {
        case WALK:
            mFrame = (get_elapsed_time(walk_time) * 4) / mWalkSpeed;
            if (mFrame >= 4) {
                nextStep();
            }
            break;

        case ATTACK:
            mFrame = (get_elapsed_time(walk_time) * 4) / aspd;
            if (mFrame >= 4) {
                nextStep();
                attack();
            }
            break;
    }

    Being::logic();
}

Being::Type LocalPlayer::getType() const
{
    return LOCALPLAYER;
}

void LocalPlayer::clearInventory()
{
    mInventory->clear();
}

void LocalPlayer::addInvItem(int id, int quantity, bool equipment)
{
    mInventory->addItem(id, quantity, equipment);
}

void LocalPlayer::addInvItem(int index, int id, int quantity, bool equipment)
{
    mInventory->addItem(index, id, quantity, equipment);
}

Item* LocalPlayer::getInvItem(int index)
{
    return mInventory->getItem(index);
}

void LocalPlayer::equipItem(Item *item)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_PLAYER_EQUIP);
    outMsg.writeInt16(item->getInvIndex());
    outMsg.writeInt16(0);
}

void LocalPlayer::unequipItem(Item *item)
{
    if (!item)
        return;

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_PLAYER_UNEQUIP);
    outMsg.writeInt16(item->getInvIndex());

    // Tidy equipment directly to avoid weapon still shown bug, by instance
    mEquipment->removeEquipment(item);
}

void LocalPlayer::useItem(Item *item)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_PLAYER_INVENTORY_USE);
    outMsg.writeInt16(item->getInvIndex());
    outMsg.writeInt32(item->getId());
    // Note: id is dest of item, usually player_node->account_ID ??
}

void LocalPlayer::dropItem(Item *item, int quantity)
{
    // TODO: Fix wrong coordinates of drops, serverside?
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_PLAYER_INVENTORY_DROP);
    outMsg.writeInt16(item->getInvIndex());
    outMsg.writeInt16(quantity);
}

void LocalPlayer::pickUp(FloorItem *item)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_ITEM_PICKUP);
    outMsg.writeInt32(item->getId());
}

void LocalPlayer::walk(Being::Direction dir)
{
    if (!mMap || dir == DIR_NONE)
        return;

    if (action == WALK)
    {
        // Avoid sending the coordinates to the server
        Being::setDestination(x, y);
        return;
    }

    Sint16 dx = 0, dy = 0;
    switch (dir)
    {
        case SOUTH:
            dy = 1;
            break;

        case WEST:
            dx = -1;
            break;

        case NORTH:
            dy = -1;
            break;

        case EAST:
            dx = 1;
            break;

        case SW:
            dx = -1;
            dy = 1;
            break;

        case NW:
            dx = -1;
            dy = -1;
            break;

        case NE:
            dx = 1;
            dy = -1;
            break;

        case SE:
            dx = 1;
            dy = 1;
            break;

        default:
            break;
    }

    // Prevent skipping corners over colliding tiles
    if (dx && mMap->tileCollides(x + dx, y))
        dx = 0;
    if (dy && mMap->tileCollides(x, y + dy))
        dy = 0;

    // Choose a straight direction when diagonal target is blocked
    if (dx && dy && !mMap->getWalk(x + dx, y + dy))
        dx = 0;

    // Walk to where the player can actually go
    if ((dx || dy) && mMap->getWalk(x + dx, y + dy))
    {
        setDestination(x + dx, y + dy);
    }
    else if (dir != DIR_NONE)
    {
        // Update the player direction to where he wants to walk
        // Warning: Not communicated to the server yet
        direction = dir;
    }
}

void LocalPlayer::setDestination(Uint16 x, Uint16 y)
{
    char temp[3];
    MessageOut outMsg(mNetwork);
    set_coordinates(temp, x, y, direction);
    outMsg.writeInt16(0x0085);
    outMsg.writeString(temp, 3);
    Being::setDestination(x, y);
}

void LocalPlayer::raiseAttribute(Attribute attr)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_STAT_UPDATE_REQUEST);

    switch (attr)
    {
        case STR:
            outMsg.writeInt16(0x000d);
            break;

        case AGI:
            outMsg.writeInt16(0x000e);
            break;

        case VIT:
            outMsg.writeInt16(0x000f);
            break;

        case INT:
            outMsg.writeInt16(0x0010);
            break;

        case DEX:
            outMsg.writeInt16(0x0011);
            break;

        case LUK:
            outMsg.writeInt16(0x0012);
            break;
    }
    outMsg.writeInt8(1);
}

void LocalPlayer::raiseSkill(Uint16 skillId)
{
    if (skillPoint <= 0)
        return;

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_SKILL_LEVELUP_REQUEST);
    outMsg.writeInt16(skillId);
}

void LocalPlayer::toggleSit()
{
    char type;
    switch (action)
    {
        case STAND: type = 2; break;
        case SIT: type = 3; break;
        default: return;
    }

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x0089);
    outMsg.writeInt32(0);
    outMsg.writeInt8(type);
}

void LocalPlayer::emote(Uint8 emotion)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x00bf);
    outMsg.writeInt8(emotion);
}

void LocalPlayer::tradeReply(bool accept)
{
    if (!accept)
        mTrading = false;

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_TRADE_RESPONSE);
    outMsg.writeInt8(accept ? 3 : 4);
}

void LocalPlayer::trade(Being *being) const
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_TRADE_REQUEST);
    outMsg.writeInt32(being->getId());
}

bool LocalPlayer::tradeRequestOk() const
{
    return !mTrading;
}

void LocalPlayer::attack(Being *target, bool keep)
{
    // Can only attack when standing still
    if (action != STAND)
        return;

    if (keep && target)
        mTarget = target;
    else if (mTarget)
        target = mTarget;

    if (!target)
        return;

    int dist_x = target->x - x;
    int dist_y = target->y - y;

    if (abs(dist_y) >= abs(dist_x))
    {
        if (dist_y > 0)
            direction = SOUTH;
        else
            direction = NORTH;
    }
    else
    {
        if (dist_x > 0)
            direction = EAST;
        else
            direction = WEST;
    }

    // Implement charging attacks here
    lastAttackTime = 0;

    action = ATTACK;
    walk_time = tick_time;
    if (getWeapon() == 2)
        sound.playSfx("sfx/bow_shoot_1.ogg");
    else
        sound.playSfx("sfx/fist-swish.ogg");

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x0089);
    outMsg.writeInt32(target->getId());
    outMsg.writeInt8(0);
}

void LocalPlayer::stopAttack()
{
    mTarget = NULL;
}

Being* LocalPlayer::getTarget() const
{
    return mTarget;
}

void LocalPlayer::revive()
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x00b2);
    outMsg.writeInt8(0);
}
