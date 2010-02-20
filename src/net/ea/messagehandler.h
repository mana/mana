/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#ifndef NET_EA_MESSAGEHANDLER_H
#define NET_EA_MESSAGEHANDLER_H

#include "net/messagehandler.h"
#include "net/messagein.h"

#include "net/ea/messageout.h"

#include <SDL_types.h>

#include <memory>

namespace EAthena {

class Network;

/**
 * \ingroup Network
 */
class MessageHandler : public Net::MessageHandler
{
    public:
        MessageHandler();

        ~MessageHandler();

        void setNetwork(Network *network);

    protected:
        Network *mNetwork;
};

typedef const std::auto_ptr<MessageHandler> MessageHandlerPtr;

}

#endif // NET_EA_MESSAGEHANDLER_H
