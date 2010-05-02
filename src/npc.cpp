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

#include "animatedsprite.h"
#include "beingmanager.h"
#include "npc.h"
#include "particle.h"
#include "text.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/npcdialog.h"
#include "gui/npcpostdialog.h"
#include "gui/userpalette.h"
#include "gui/sell.h"

#include "net/net.h"
#include "net/npchandler.h"

#include "resources/npcdb.h"

NPC::NPC(int id, int subtype, Map *map):
    Being(id, subtype, map)
{
    setSubtype(subtype);

    setShowName(true);
}

void NPC::setName(const std::string &name)
{
    const std::string displayName = name.substr(0, name.find('#', 0));

    Being::setName(displayName);

    mNameColor = &userPalette->getColor(UserPalette::NPC);

    mDispName->setColor(mNameColor);
}

void NPC::setSubtype(Uint16 subtype)
{
    Being::setSubtype(subtype);

    setupSpriteDisplay(NPCDB::get(subtype), false);
}

void NPC::talk()
{
    Net::getNpcHandler()->talk(mId);
}

bool NPC::isTalking()
{
    return NpcDialog::isActive() || BuyDialog::isActive() ||
           SellDialog::isActive() || BuySellDialog::isActive() ||
           NpcPostDialog::isActive();
}
