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

#ifndef NET_TMWSERV_NETWORK_H
#define NET_TMWSERV_NETWORK_H

#include <iosfwd>

/**
 * \defgroup Network Core network layer
 */

class MessageHandler;
class MessageOut;

/**
 * \ingroup Network
 */
namespace Net
{
    class Connection;

    /**
     * Initializes the network subsystem.
     */
    void initialize();

    /**
     * Finalizes the network subsystem.
     */
    void finalize();

    /**
     * Returns a new Connection object. Should be deleted by the caller.
     */
    Connection *getConnection();

    /**
     * Registers a message handler. A message handler handles a certain
     * subset of incoming messages.
     */
    void registerHandler(MessageHandler *handler);

    /**
     * Unregisters a message handler.
     */
    void unregisterHandler(MessageHandler *handler);

    /**
     * Clears all registered message handlers.
     */
    void clearHandlers();

    /*
     * Handles all events and dispatches incoming messages to the
     * registered handlers
     */
    void flush();
}

#endif
