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
#include "configuration.h"

NPC::NPC(int id, int subtype, Map *map):
    Player(id, subtype, map, true)
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

    NPCInfo info = NPCDB::get(subtype);

    mSprites.clear();
    // Setup NPC sprites
    for (std::list<NPCsprite*>::const_iterator i = info.sprites.begin();
         i != info.sprites.end();
         i++)
    {
        std::string file = paths.getValue("sprites",
                                          "graphics/sprites/") + (*i)->sprite;
        int variant = (*i)->variant;
        mSprites.push_back(AnimatedSprite::load(file, variant));
        mSpriteIDs.push_back(0);
        mSpriteColors.push_back("");
    }

    if (Particle::enabled)
    {
        //setup particle effects
        for (std::list<std::string>::const_iterator i = info.particles.begin();
             i != info.particles.end();
             i++)
        {
            Particle *p = particleEngine->addEffect(*i, 0, 0);
            this->controlParticle(p);
        }
    }
}

void NPC::talk()
{
    Net::getNpcHandler()->talk(mId);
}

void NPC::setSprite(unsigned int slot, int id, const std::string &color)
{
    // Do nothing
}

bool NPC::isTalking()
{
    return NpcDialog::isActive() || BuyDialog::isActive() ||
           SellDialog::isActive() || BuySellDialog::isActive() ||
           NpcPostDialog::isActive();
}
