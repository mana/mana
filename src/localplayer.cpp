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

#include "equipment.h"
#include "floor_item.h"
#include "game.h"
#include "inventory.h"
#include "item.h"
#include "main.h"
#include "sound.h"
#include "log.h"

#include "net/gameserver/player.h"

LocalPlayer *player_node = NULL;

LocalPlayer::LocalPlayer():
    Player(65535, 0, NULL),
    mAttackRange(0),
    mInventory(new Inventory()),
    mAttributeBase(NB_BASE_ATTRIBUTES, 0),
    mAttributeEffective(NB_BASE_ATTRIBUTES, 0),
    mLevel(1),
    mTarget(NULL), mPickUpTarget(NULL),
    mTrading(false),
    mLastAction(-1)
{
}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::logic()
{
    // Actions are allowed once per second
    if (get_elapsed_time(mLastAction) >= 1000) {
        mLastAction = -1;
    }

    Being::logic();
}

void LocalPlayer::nextStep()
{
    if (mPath.empty())
    {
        if (mPickUpTarget)
        {
            pickUp(mPickUpTarget);
        }

        if (mWalkingDir)
        {
            walk(mWalkingDir);
        }
    }

    Player::nextStep();
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
    Net::GameServer::Player::equip(item->getInvIndex());
}

void LocalPlayer::unequipItem(Item *item)
{
    if (!item)
        return;

    // XXX Convert for new server
    /*
    MessageOut outMsg(CMSG_PLAYER_UNEQUIP);
    outMsg.writeShort(item->getInvIndex());
    */

    // Tidy equipment directly to avoid weapon still shown bug, by instance
    mEquipment->removeEquipment(item);
}

void LocalPlayer::useItem(Item *item)
{
    // XXX Convert for new server
    /*
    MessageOut outMsg(CMSG_PLAYER_INVENTORY_USE);
    outMsg.writeShort(item->getInvIndex());
    outMsg.writeLong(item->getId());
    // Note: id is dest of item, usually player_node->account_ID ??
    */
}

void LocalPlayer::dropItem(Item *item, int quantity)
{
    Net::GameServer::Player::drop(item->getInvIndex(), quantity);
}

void LocalPlayer::pickUp(FloorItem *item)
{
    int dx = item->getX() - mX / 32;
    int dy = item->getY() - mY / 32;

    if (dx * dx + dy * dy < 4) {
        int id = item->getId();
        Net::GameServer::Player::pickUp(id >> 16, id & 0xFFFF);
        mPickUpTarget = NULL;
    } else {
        setDestination(item->getX() * 32 + 16, item->getY() * 32 + 16);
        mPickUpTarget = item;
    }
}

void LocalPlayer::walk(unsigned char dir)
{
    if (!mMap || !dir)
        return;

    if (mAction == WALK && !mPath.empty())
    {
        // Just finish the current action, otherwise we get out of sync
        Being::setDestination(mX, mY);
        return;
    }

    int dx = 0, dy = 0;
    if (dir & UP)
        dy -= 32;
    if (dir & DOWN)
        dy += 32;
    if (dir & LEFT)
        dx -= 32;
    if (dir & RIGHT)
        dx += 32;

    // Prevent skipping corners over colliding tiles
    if (dx && mMap->tileCollides((mX + dx) / 32, mY / 32))
        dx = 16 - mX % 32;
    if (dy && mMap->tileCollides(mX / 32, (mY + dy) / 32))
        dy = 16 - mY % 32;

    // Choose a straight direction when diagonal target is blocked
    if (dx && dy && !mMap->getWalk((mX + dx) / 32, (mY + dy) / 32))
        dx = 16 - mX % 32;

    // Walk to where the player can actually go
    if ((dx || dy) && mMap->getWalk((mX + dx) / 32, (mY + dy) / 32))
    {
        setDestination(mX + dx, mY + dy);
    }
    else if (dir)
    {
        // If the being can't move, just change direction
        // TODO: Communicate this to the server (waiting on tmwserv)
        setDirection(dir);
    }
}

void LocalPlayer::setDestination(Uint16 x, Uint16 y)
{
    // Fix coordinates so that the player does not seem to dig into walls.
    int tx = x / 32, ty = y / 32, fx = x % 32, fy = y % 32;
    if (fx != 16 && mMap->tileCollides(tx + fx / 16 * 2 - 1, ty)) fx = 16;
    if (fy != 16 && mMap->tileCollides(tx, ty + fy / 16 * 2 - 1)) fy = 16;
    if (fx != 16 && fy != 16 && mMap->tileCollides(tx + fx / 16 * 2 - 1, ty + fy / 16 * 2 - 1)) fx = 16;
    x = tx * 32 + fx;
    y = ty * 32 + fy;

    // Only send a new message to the server when destination changes
    if (x != mDestX || y != mDestY)
    {
        mDestX = x;
        mDestY = y;

        Net::GameServer::Player::walk(x, y);
    }

    mPickUpTarget = NULL;
    Being::setDestination(x, y);
}

void LocalPlayer::setWalkingDir(int dir)
{
    if (mWalkingDir != dir)
    {
        mWalkingDir = dir;
    }

    // If we're not already walking, start walking.
    if (mAction != WALK && dir)
    {
        walk(dir);
    }
}

void LocalPlayer::toggleSit()
{
    if (mLastAction != -1)
        return;
    mLastAction = tick_time;

    Being::Action newAction;
    switch (mAction)
    {
        case Being::STAND: newAction = Being::SIT; break;
        case Being::SIT: newAction = Being::STAND; break;
        default: return;
    }

    setAction(newAction);
    Net::GameServer::Player::changeAction(newAction);
}

void LocalPlayer::emote(Uint8 emotion)
{
    if (mLastAction != -1)
        return;
    mLastAction = tick_time;

    // XXX Convert for new server
    /*
    MessageOut outMsg(0x00bf);
    outMsg.writeByte(emotion);
    */
}

void LocalPlayer::tradeReply(bool accept)
{
    if (!accept)
        mTrading = false;

    // XXX Convert for new server
    /*
    MessageOut outMsg(CMSG_TRADE_RESPONSE);
    outMsg.writeByte(accept ? 3 : 4);
    */
}

void LocalPlayer::trade(Being *being) const
{
    // XXX Convert for new server
    /*
    MessageOut outMsg(CMSG_TRADE_REQUEST);
    outMsg.writeLong(being->getId());
    */
}

bool LocalPlayer::tradeRequestOk() const
{
    return !mTrading;
}

void LocalPlayer::attack()
{
    if (mLastAction != -1)
        return;

    // Can only attack when standing still
    if (mAction != STAND && mAction != ATTACK)
        return;

    mLastAction = tick_time;
    mWalkTime = tick_time;

    setAction(ATTACK);

    if (getWeapon() == 2)
        sound.playSfx("sfx/bow_shoot_1.ogg");
    else
        sound.playSfx("sfx/fist-swish.ogg");

    Net::GameServer::Player::attack(getSpriteDirection());
}

Being* LocalPlayer::getTarget() const
{
    return mTarget;
}

void LocalPlayer::revive()
{
    // XXX Convert for new server
    /*
    MessageOut outMsg(0x00b2);
    outMsg.writeByte(0);
    */
}

void LocalPlayer::raiseAttribute(size_t attr)
{
    mAttributeBase.at(attr)++;
    // TODO: Inform the server about our desire to raise the attribute
}
