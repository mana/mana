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
 */

#include "animatedsprite.h"
#include "graphics.h"
#include "localplayer.h"
#include "npc.h"
#include "particle.h"
#include "text.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "resources/npcdb.h"

#include "gui/gui.h"

NPC *current_npc = 0;

static const int NAME_X_OFFSET = 15;
static const int NAME_Y_OFFSET = 30;

NPC::NPC(Uint32 id, Uint16 job, Map *map, Network *network):
    Player(id, job, map), mNetwork(network)
{
    NPCInfo info = NPCDB::get(job);

    // Setup NPC sprites
    int c = BASE_SPRITE;
    for (std::list<NPCsprite*>::const_iterator i = info.sprites.begin();
         i != info.sprites.end();
         i++)
    {
        if (c == VECTOREND_SPRITE) break;

        std::string file = "graphics/sprites/" + (*i)->sprite;
        int variant = (*i)->variant;
        mSprites[c] = AnimatedSprite::load(file, variant);
        c++;
    }

    if (mParticleEffects)
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
    mName = 0;
}

NPC::~NPC()
{
    if (mName)
    {
        delete mName;
        player_node->setTarget(NULL);
    }
}

void NPC::setName(const std::string &name)
{
    if (mName)
    {
        delete mName;
    }
    std::string displayName = name.substr(0, name.find('#', 0));

    mName = new Text(displayName, mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET,
                     gcn::Graphics::CENTER, npcNameFont,
                     gcn::Color(200, 200, 255));
    Being::setName(displayName + " (NPC)");
}

void NPC::setGender(int gender)
{
    Being::setGender(gender);
}

void NPC::setSprite(int slot, int id, std::string color)
{
    // Fix this later should it not be adequate enough.
    Being::setSprite(slot, id, color);
}

Being::Type
NPC::getType() const
{
    return Being::NPC;
}

void
NPC::talk()
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_TALK);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(0);
    current_npc = this;
}

void
NPC::nextDialog()
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_NEXT_REQUEST);
    outMsg.writeInt32(mId);
}

void
NPC::dialogChoice(char choice)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_LIST_CHOICE);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(choice);
}

/*
 * TODO Unify the buy() and sell() methods, without sacrificing readability of
 * the code calling the method. buy(bool buySell) would be bad...
 */
void
NPC::buy()
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(0);
}

void
NPC::sell()
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(1);
}

void NPC::updateCoords()
{
    if (mName)
    {
        mName->adviseXY(mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET);
    }
}
