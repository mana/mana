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

#include "npc.h"

#include "animatedsprite.h"
#include "graphics.h"

#include "gui/gui.h"
#include "net/messageout.h"
#include "net/gameserver/player.h"

NPC *current_npc = 0;

NPC::NPC(Uint16 id, Uint16 job, Map *map):
    Being(id, job, map)
{
    mSprites[BASE_SPRITE] = AnimatedSprite::load("graphics/sprites/npc.xml",
            job - 100);
}

Being::Type
NPC::getType() const
{
    return Being::NPC;
}

void
NPC::drawName(Graphics *graphics, Sint32 offsetX, Sint32 offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    graphics->setFont(speechFont);
    graphics->setColor(gcn::Color(200, 200, 255));
    graphics->drawText(mName, px + 15, py + 30, gcn::Graphics::CENTER);
}

void
NPC::talk()
{
    Net::GameServer::Player::talkToNPC(mId, true);
    current_npc = this;
}

void
NPC::nextDialog()
{
    Net::GameServer::Player::talkToNPC(mId, false);
}

void
NPC::dialogChoice(int choice)
{
    Net::GameServer::Player::selectFromNPC(mId, choice);
}

/*
 * TODO Unify the buy() and sell() methods, without sacrificing readability of
 * the code calling the method. buy(bool buySell) would be bad...
 */
void
NPC::buy()
{
    // XXX Convert for new server
    /*
    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(0);
    */
}

void
NPC::sell()
{
    // XXX Convert for new server
    /*
    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(1);
    */
}
