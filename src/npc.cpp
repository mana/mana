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

#include "animatedsprite.h"
#include "beingmanager.h"
#include "npc.h"
#include "particle.h"
#include "text.h"

#include "gui/npc_text.h"
#include "gui/palette.h"

#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/gameserver/player.h"
#else
#include "net/messageout.h"
#include "net/ea/protocol.h"
#endif

#include "resources/npcdb.h"

bool NPC::isTalking = false;
int current_npc = 0;

static const int NAME_X_OFFSET = 15;
static const int NAME_Y_OFFSET = 30;

#ifdef TMWSERV_SUPPORT
NPC::NPC(Uint16 id, int job, Map *map):
    Player(id, job, map)
#else
NPC::NPC(int id, Uint16 job, Map *map, Network *network):
    Player(id, job, map),
    mNetwork(network)
#endif
{
    NPCInfo info = NPCDB::get(job);

    // Setup NPC sprites
    int c = BASE_SPRITE;
    for (std::list<NPCsprite*>::const_iterator i = info.sprites.begin();
         i != info.sprites.end();
         i++)
    {
        if (c == VECTOREND_SPRITE)
            break;

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

    mNameColor = &guiPalette->getColor(Palette::NPC);
}

NPC::~NPC()
{
    delete mName;
}

void NPC::setName(const std::string &name)
{
    const std::string displayName = name.substr(0, name.find('#', 0));

    delete mName;
    mName = new Text(displayName, mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET,
                     gcn::Graphics::CENTER,
                     &guiPalette->getColor(Palette::NPC));
    Being::setName(displayName + " (NPC)");
}

void NPC::setGender(Gender gender)
{
    Being::setGender(gender);
}

void NPC::setSprite(int slot, int id, std::string color)
{
    // Fix this later should it not be adequate enough.
    Being::setSprite(slot, id, color);
}

Being::Type NPC::getType() const
{
    return Being::NPC;
}

void NPC::talk()
{
    if (isTalking)
        return;

    isTalking = true;

#ifdef TMWSERV_SUPPORT
    Net::GameServer::Player::talkToNPC(mId, true);
#else
    if (!mNetwork)
        return;

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_TALK);
    outMsg.writeInt16(CMSG_NPC_TALK);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(0);
#endif
}

void NPC::updateCoords()
{
    if (mName)
    {
#ifdef TMWSERV_SUPPORT
        const Vector &pos = getPosition();
        const int px = (int) pos.x + NAME_X_OFFSET;
        const int py = (int) pos.y + NAME_Y_OFFSET;
#else
        const int px = mPx + NAME_X_OFFSET;
        const int py = mPy + NAME_Y_OFFSET;
#endif
        mName->adviseXY(px, py);
    }
}
