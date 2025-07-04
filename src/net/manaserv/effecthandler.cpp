/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "net/manaserv/effecthandler.h"

#include "actorspritemanager.h"
#include "effectmanager.h"
#include "localplayer.h"
#include "log.h"

#include "gui/viewport.h"

#include "net/manaserv/manaserv_protocol.h"
#include "net/manaserv/messagein.h"

namespace ManaServ {

EffectHandler::EffectHandler()
{
    static const uint16_t _messages[] = {
        GPMSG_CREATE_EFFECT_POS,
        GPMSG_CREATE_EFFECT_BEING,
        GPMSG_CREATE_TEXT_PARTICLE,
        GPMSG_SHAKE,
        0
    };
    handledMessages = _messages;
}

void EffectHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_CREATE_EFFECT_POS:
            handleCreateEffectPos(msg);
            break;
        case GPMSG_CREATE_EFFECT_BEING:
            handleCreateEffectBeing(msg);
            break;
        case GPMSG_CREATE_TEXT_PARTICLE:
            handleCreateTextParticle(msg);
            break;
        case GPMSG_SHAKE:
            handleShake(msg);
            break;
        default:
            break;
    }
}

void EffectHandler::handleCreateEffectPos(MessageIn &msg)
{
    int id = msg.readInt16();
    uint16_t x = msg.readInt16();
    uint16_t y = msg.readInt16();
    effectManager->trigger(id, x, y);
}

void EffectHandler::handleCreateEffectBeing(MessageIn &msg)
{
    int eid = msg.readInt16();
    int bid = msg.readInt16();
    Being* b = actorSpriteManager->findBeing(bid);
    if (b)
        effectManager->trigger(eid, b);
    else
        Log::warn("CreateEffect called for unknown being #%d", bid);
}

void EffectHandler::handleCreateTextParticle(MessageIn &msg)
{
    const std::string &text = msg.readString();
    if (local_player)
        local_player->addMessageToQueue(text);
}

void EffectHandler::handleShake(MessageIn &msg)
{
    int16_t intensityX = 0;
    int16_t intensityY = 0;
    float decay;
    int duration;

    switch (msg.getUnreadLength())
    {
        case 4:
            intensityX =  msg.readInt16();
            intensityY =  msg.readInt16();
            viewport->shakeScreen(intensityX, intensityY);
            break;
        case 6:
            intensityX =  msg.readInt16();
            intensityY =  msg.readInt16();
            decay =  msg.readInt16() / 10000.0f;
            viewport->shakeScreen(intensityX, intensityY, decay);
            break;
        case 8:
            intensityX =  msg.readInt16();
            intensityY =  msg.readInt16();
            decay =  msg.readInt16() / 10000.0f;
            duration =  msg.readInt16();
            viewport->shakeScreen(intensityX, intensityY, decay, duration);
            break;
        default:
            Log::warn("Received GPMSG_SHAKE message with unexpected length of %d bytes", msg.getUnreadLength());
    }
}

} // namespace ManaServ
