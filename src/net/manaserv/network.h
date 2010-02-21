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

#ifndef NET_MANASERV_NETWORK_H
#define NET_MANASERV_NETWORK_H

#include <iosfwd>

/**
 * \ingroup Network
 */
namespace ManaServ
{
    class MessageHandler;
    class MessageOut;

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
    void clearNetworkHandlers();

    /*
     * Handles all events and dispatches incoming messages to the
     * registered handlers
     */
    void flush();
} // namespace ManaServ

#endif
