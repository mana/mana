/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "net/tmwa/network.h"

#include "log.h"

#include "utils/format.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace TmwAthena {

/**
 * The live Network instances by WebSocket handle. WebSocket events arrive
 * asynchronously, so an event can still be delivered for a socket whose
 * Network is already gone. Looking the instance up here keeps that harmless.
 */
static std::map<EMSCRIPTEN_WEBSOCKET_T, Network *> networksBySocket;

/*
 * Sockets closed locally whose emscripten handle is kept until the close
 * event arrives. Deleting the handle right away lets the runtime recycle
 * the id for the next emscripten_websocket_new call, after which the late
 * close event is attributed to the new connection and shows up as a bogus
 * "Connection closed (1000)" failure.
 */
static std::set<EMSCRIPTEN_WEBSOCKET_T> pendingClose;

static Network *findNetwork(EMSCRIPTEN_WEBSOCKET_T socket)
{
    auto it = networksBySocket.find(socket);
    return it != networksBySocket.end() ? it->second : nullptr;
}

// EM_ASM blocks are not scanned for the JS library functions they use
EM_JS_DEPS(mana_network, "$stringToNewUTF8");

/**
 * Returns the base URL of the WebSocket-to-TCP proxy, always ending in a
 * slash. The page can point the client at a proxy by setting
 * Module.manaProxyUrl, otherwise the page origin with a "/tmwa/" path is
 * assumed.
 */
static std::string proxyBaseUrl()
{
    char *url = (char *) EM_ASM_PTR({
        var url = Module['manaProxyUrl'];
        if (typeof url !== 'string' || url.length === 0) {
            url = (location.protocol === 'https:' ? 'wss://' : 'ws://')
                  + location.host + '/tmwa/';
        }
        if (url.charAt(url.length - 1) !== '/')
            url += '/';
        return stringToNewUTF8(url);
    });

    std::string result(url);
    free(url);
    return result;
}

void Network::initTransport()
{
}

void Network::deinitTransport()
{
}

bool Network::connectTransport(const ServerInfo &server)
{
    if (!emscripten_websocket_is_supported())
    {
        setError(_("This browser does not support WebSockets"));
        return false;
    }

    // Release a socket left over by a previous connection attempt
    closeSocket();

    const std::string url = proxyBaseUrl() + server.hostname + '/'
                                           + toString(server.port);

    Log::info("Network::Connecting through {}", url);

    EmscriptenWebSocketCreateAttributes attributes;
    emscripten_websocket_init_create_attributes(&attributes);
    attributes.url = url.c_str();
    attributes.protocols = "binary";
    attributes.createOnMainThread = true;

    const EMSCRIPTEN_WEBSOCKET_T socket =
            emscripten_websocket_new(&attributes);
    if (socket <= 0)
    {
        setError(strformat(_("Unable to open a WebSocket to \"{}\""), url));
        return false;
    }

    mSocket = socket;
    mOpened = false;
    networksBySocket[socket] = this;

    emscripten_websocket_set_onopen_callback(socket, nullptr, onOpen);
    emscripten_websocket_set_onmessage_callback(socket, nullptr, onMessage);
    emscripten_websocket_set_onerror_callback(socket, nullptr, onError);
    emscripten_websocket_set_onclose_callback(socket, nullptr, onClose);

    return true;
}

void Network::disconnectTransport()
{
    closeSocket();
}

void Network::closeSocket()
{
    if (!mSocket)
        return;

    // Dropping the registration first makes sure that no event still in
    // flight reaches this Network. The handle itself is deleted once its
    // close event arrives (see pendingClose) so the id cannot be recycled
    // while that event is pending.
    networksBySocket.erase(mSocket);
    pendingClose.insert(mSocket);

    // When the close cannot be issued there is no close event to wait for
    // either, so the handle is deleted right away.
    if (emscripten_websocket_close(mSocket, 1000, "Client disconnected") !=
            EMSCRIPTEN_RESULT_SUCCESS)
    {
        pendingClose.erase(mSocket);
        emscripten_websocket_delete(mSocket);
    }

    mSocket = 0;
    mOpened = false;
}

void Network::sendTransport(const char *data, size_t size)
{
    // The browser queues the data, so this never blocks
    if (emscripten_websocket_send_binary(mSocket, const_cast<char *>(data),
                                         size) < 0)
    {
        setError(_("Failed to send data to the server"));
    }
}

bool Network::onOpen(int, const EmscriptenWebSocketOpenEvent *event, void *)
{
    Network *network = findNetwork(event->socket);
    if (!network)
        return true;

    network->mOpened = true;
    network->mState = CONNECTED;

    Log::info("Network::Started session with {}:{}",
              network->mServer.hostname, network->mServer.port);

    return true;
}

bool Network::onMessage(int,
                        const EmscriptenWebSocketMessageEvent *event,
                        void *)
{
    Network *network = findNetwork(event->socket);
    if (!network)
        return true;

    if (event->isText)
    {
        Log::info("Network: Ignoring text frame of {} bytes", event->numBytes);
        return true;
    }

    network->received(reinterpret_cast<const char *>(event->data),
                      event->numBytes);

    return true;
}

bool Network::onError(int,
                      const EmscriptenWebSocketErrorEvent *event,
                      void *)
{
    Network *network = findNetwork(event->socket);
    if (!network)
        return true;

    // The browser does not tell us what went wrong. A close event normally
    // follows with the code and reason, replacing this message.
    network->setError(strformat(_("Unable to reach {}:{}"),
                                network->mServer.hostname,
                                network->mServer.port));

    return true;
}

bool Network::onClose(int,
                      const EmscriptenWebSocketCloseEvent *event,
                      void *)
{
    if (pendingClose.erase(event->socket))
    {
        emscripten_websocket_delete(event->socket);
        return true;
    }

    Network *network = findNetwork(event->socket);
    if (!network)
        return true;

    const bool wasOpen = network->mOpened;
    network->mOpened = false;

    if (wasOpen && event->wasClean && event->code == 1000)
    {
        // The server hung up, which is how a session normally ends
        network->mState = IDLE;
        Log::info("Disconnected.");
    }
    else if (event->reason[0])
    {
        network->setError(strformat(_("Connection closed ({}): {}"),
                                    (int) event->code, event->reason));
    }
    else
    {
        network->setError(strformat(_("Connection closed ({})"),
                                    (int) event->code));
    }

    // The close event already arrived, so the handle can go now instead
    // of waiting in pendingClose for an event that will not come again.
    networksBySocket.erase(event->socket);
    emscripten_websocket_delete(event->socket);
    network->mSocket = 0;

    return true;
}

} // namespace TmwAthena
