/*
 *  This file is part of The Mana World
 *
 * Copyright (c) 2008, The Mana World Development Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * My name may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 * $Id: $
 */

#include <iostream>
#include "partyhandler.h"

#include "protocol.h"
#include "messagein.h"

#include "chatserver/chatserver.h"

#include "../gui/chat.h"
#include "../log.h"
#include "../localplayer.h"

PartyHandler::PartyHandler()
{
    static const Uint16 _messages[] = {
        CPMSG_PARTY_CREATE_RESPONSE,
        CPMSG_PARTY_QUIT_RESPONSE,
        0
    };
    handledMessages = _messages;

}

void PartyHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case CPMSG_PARTY_CREATE_RESPONSE:
        {
            if (msg.readInt8() == ERRMSG_OK)
            {
                player_node->setInParty(true);
            }
        } break;

        case CPMSG_PARTY_QUIT_RESPONSE:
        {
            if (msg.readInt8() == ERRMSG_OK)
            {
                player_node->setInParty(false);
            }
        } break;
    }
}
