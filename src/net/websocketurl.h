/*
 *  The Mana Client
 *  Copyright (C) 2026  The Mana Developers
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

#pragma once

#include <string>

class ServerInfo;

namespace Net {

/**
 * Returns the base URL of the WebSocket endpoint through which the given
 * server is reached, always ending in a slash. The client appends
 * "<host>/<port>" to it.
 *
 * The URL given by the server itself wins, then the proxy the page set in
 * Module.manaProxyUrl, then the page origin with a "/tmwa/" path.
 *
 * Outside the browser build only the server's own URL is known, which makes
 * the result empty for most servers. Nothing connects through it there.
 */
std::string webSocketUrl(const ServerInfo &server);

/**
 * Returns whether the page set an explicit WebSocket proxy, which is then
 * expected to reach every server. Always false outside the browser build.
 */
bool hasPageWebSocketProxy();

} // namespace Net
