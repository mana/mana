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

#include "net/websocketurl.h"

#include "net/serverinfo.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

#include <cstdlib>

// EM_ASM blocks are not scanned for the JS library functions they use
EM_JS_DEPS(mana_websocketurl, "$stringToNewUTF8");

/**
 * Returns the proxy URL the page set in Module.manaProxyUrl, or an empty
 * string when it set none.
 */
static std::string pageProxyUrl()
{
    char *url = (char *) EM_ASM_PTR({
        var url = Module['manaProxyUrl'];
        if (typeof url !== 'string')
            url = "";
        return stringToNewUTF8(url);
    });

    std::string result(url);
    free(url);
    return result;
}

/**
 * Returns the page origin with a "/tmwa/" path, the proxy location assumed
 * when neither the server nor the page names one.
 */
static std::string originProxyUrl()
{
    char *url = (char *) EM_ASM_PTR({
        var url = (location.protocol === 'https:' ? 'wss://' : 'ws://')
                  + location.host + '/tmwa/';
        return stringToNewUTF8(url);
    });

    std::string result(url);
    free(url);
    return result;
}
#endif // __EMSCRIPTEN__

namespace Net {

std::string webSocketUrl(const ServerInfo &server)
{
    std::string url = server.websocket;

#ifdef __EMSCRIPTEN__
    if (url.empty())
        url = pageProxyUrl();
    if (url.empty())
        url = originProxyUrl();
#endif

    if (!url.empty() && url.back() != '/')
        url += '/';

    return url;
}

bool hasPageWebSocketProxy()
{
#ifdef __EMSCRIPTEN__
    return !pageProxyUrl().empty();
#else
    return false;
#endif
}

} // namespace Net
