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
#include "npc.h"
#include "particle.h"
#include "text.h"

#include "gui/npc_text.h"

#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/gameserver/player.h"
#else
#include "net/messageout.h"
#include "net/ea/protocol.h"
#endif

#include "resources/npcdb.h"

extern NpcTextDialog *npcTextDialog;

NPC *current_npc = 0;

static const int NAME_X_OFFSET = 15;
static const int NAME_Y_OFFSET = 30;

#ifdef TMWSERV_SUPPORT
NPC::NPC(Uint16 id, int job, Map *map):
    Player(id, job, map)
#else
NPC::NPC(Uint32 id, Uint16 job, Map *map, Network *network):
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

    mNameColor = 0x21bbbb;
}

NPC::~NPC()
{
    delete mName;

    if (current_npc == this) handleDeath();
}

void NPC::setName(const std::string &name)
{
    const std::string displayName = name.substr(0, name.find('#', 0));

    delete mName;
    mName = new Text(displayName, mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET,
                     gcn::Graphics::CENTER, gcn::Color(200, 200, 255));
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
#ifdef TMWSERV_SUPPORT
    Net::GameServer::Player::talkToNPC(mId, true);
#else
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_TALK);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(0);
#endif
    current_npc = this;
}

void NPC::nextDialog()
{
#ifdef TMWSERV_SUPPORT
    Net::GameServer::Player::talkToNPC(mId, false);
#else
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_NEXT_REQUEST);
    outMsg.writeInt32(mId);
#endif
}

void NPC::dialogChoice(char choice)
{
#ifdef TMWSERV_SUPPORT
    Net::GameServer::Player::selectFromNPC(mId, choice);
#else
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_LIST_CHOICE);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(choice);
#endif
}

void NPC::integerInput(int value)
{
#ifdef EATHENA_SUPPORT
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_INT_RESPONSE);
    outMsg.writeInt32(mId);
    outMsg.writeInt32(value);
#endif
}

void NPC::stringInput(const std::string &value)
{
#ifdef EATHENA_SUPPORT
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_STR_RESPONSE);
    outMsg.writeInt16(value.length() + 9);
    outMsg.writeInt32(mId);
    outMsg.writeString(value, value.length());
    outMsg.writeInt8(0);
#endif
}

/*
 * TODO Unify the buy() and sell() methods, without sacrificing readability of
 * the code calling the method. buy(bool buySell) would be bad...
 */
void NPC::buy()
{
    // XXX Convert for new server
#ifdef EATHENA_SUPPORT
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(0);
#endif
}

void NPC::sell()
{
    // XXX Convert for new server
#ifdef EATHENA_SUPPORT
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(1);
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

void NPC::handleDeath()
{
    printf("NPC::handleDeath\n");
    if (this != current_npc) return;

    if (npcTextDialog->isVisible())
        npcTextDialog->showCloseButton();
    else current_npc = NULL;
}
