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

#include "net/tmwa/network.h"

#include "log.h"

#include "net/tmwa/messagein.h"
#include "net/tmwa/protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <cassert>
#include <sstream>

/** Warning: buffers and other variables are shared,
    so there can be only one connection active at a time */

namespace TmwAthena {

struct PacketInfo
{
    uint16_t id;
    uint16_t length;
    const char *name;
};

// indicator for a variable-length packet
const uint16_t VAR = 1;

static const PacketInfo packet_infos[] = {
    // login server messages
    { SMSG_UPDATE_HOST,               VAR, "SMSG_UPDATE_HOST" },
    { CMSG_LOGIN_REGISTER,            55, "CMSG_LOGIN_REGISTER" },
    { SMSG_LOGIN_DATA,                VAR, "SMSG_LOGIN_DATA" },
    { SMSG_LOGIN_ERROR,               23, "SMSG_LOGIN_ERROR" },

    // char server messages
    { CMSG_CHAR_PASSWORD_CHANGE,      50, "CMSG_CHAR_PASSWORD_CHANGE" },
    { SMSG_CHAR_PASSWORD_RESPONSE,    3, "SMSG_CHAR_PASSWORD_RESPONSE" },
    { CMSG_CHAR_SERVER_CONNECT,       17, "CMSG_CHAR_SERVER_CONNECT" },
    { CMSG_CHAR_SELECT,               3, "CMSG_CHAR_SELECT" },
    { CMSG_CHAR_CREATE,               37, "CMSG_CHAR_CREATE" },
    { CMSG_CHAR_DELETE,               46, "CMSG_CHAR_DELETE" },
    { SMSG_CHAR_LOGIN,                VAR, "SMSG_CHAR_LOGIN" },
    { SMSG_CHAR_LOGIN_ERROR,          3, "SMSG_CHAR_LOGIN_ERROR" },
    { SMSG_CHAR_CREATE_SUCCEEDED,     108, "SMSG_CHAR_CREATE_SUCCEEDED" },
    { SMSG_CHAR_CREATE_FAILED,        3, "SMSG_CHAR_CREATE_FAILED" },
    { SMSG_CHAR_DELETE_SUCCEEDED,     2, "SMSG_CHAR_DELETE_SUCCEEDED" },
    { SMSG_CHAR_DELETE_FAILED,        3, "SMSG_CHAR_DELETE_FAILED" },
    { SMSG_CHAR_MAP_INFO,             28, "SMSG_CHAR_MAP_INFO" },

    // map server messages
    { CMSG_MAP_SERVER_CONNECT,        19, "CMSG_MAP_SERVER_CONNECT" },
    { SMSG_MAP_LOGIN_SUCCESS,         11, "SMSG_MAP_LOGIN_SUCCESS" },
    { SMSG_BEING_VISIBLE,             54, "SMSG_BEING_VISIBLE" },
    { SMSG_BEING_MOVE,                60, "SMSG_BEING_MOVE" },
    { SMSG_BEING_SPAWN,               41, "SMSG_BEING_SPAWN" },
    { CMSG_MAP_LOADED,                2, "CMSG_MAP_LOADED" },
    { CMSG_MAP_PING,                  6, "CMSG_MAP_PING" },
    { SMSG_SERVER_PING,               6, "SMSG_SERVER_PING" },
    { SMSG_BEING_REMOVE,              7, "SMSG_BEING_REMOVE" },
    { CMSG_PLAYER_CHANGE_DEST,        5, "CMSG_PLAYER_CHANGE_DEST" },
    { SMSG_WALK_RESPONSE,             12, "SMSG_WALK_RESPONSE" },
    { SMSG_PLAYER_STOP,               10, "SMSG_PLAYER_STOP" },
    { CMSG_PLAYER_CHANGE_ACT,         7, "CMSG_PLAYER_CHANGE_ACT" },
    { SMSG_BEING_ACTION,              29, "SMSG_BEING_ACTION" },
    { CMSG_CHAT_MESSAGE,              VAR, "CMSG_CHAT_MESSAGE" },
    { SMSG_BEING_CHAT,                VAR, "SMSG_BEING_CHAT" },
    { SMSG_PLAYER_CHAT,               VAR, "SMSG_PLAYER_CHAT" },
    { CMSG_NPC_TALK,                  7, "CMSG_NPC_TALK" },
    { SMSG_PLAYER_WARP,               22, "SMSG_PLAYER_WARP" },
    { SMSG_CHANGE_MAP_SERVER,         28, "SMSG_CHANGE_MAP_SERVER" },
    { CMSG_NAME_REQUEST,              6, "CMSG_NAME_REQUEST" },
    { SMSG_BEING_NAME_RESPONSE,       30, "SMSG_BEING_NAME_RESPONSE" },
    { CMSG_CHAT_WHISPER,              VAR, "CMSG_CHAT_WHISPER" },
    { SMSG_WHISPER,                   VAR, "SMSG_WHISPER" },
    { SMSG_WHISPER_RESPONSE,          3, "SMSG_WHISPER_RESPONSE" },
    { SMSG_GM_CHAT,                   VAR, "SMSG_GM_CHAT" },
    { CMSG_PLAYER_CHANGE_DIR,         5, "CMSG_PLAYER_CHANGE_DIR" },
    { SMSG_BEING_CHANGE_DIRECTION,    9, "SMSG_BEING_CHANGE_DIRECTION" },
    { SMSG_ITEM_VISIBLE,              17, "SMSG_ITEM_VISIBLE" },
    { SMSG_ITEM_DROPPED,              17, "SMSG_ITEM_DROPPED" },
    { CMSG_ITEM_PICKUP,               6, "CMSG_ITEM_PICKUP" },
    { SMSG_PLAYER_INVENTORY_ADD,      23, "SMSG_PLAYER_INVENTORY_ADD" },
    { SMSG_ITEM_REMOVE,               6, "SMSG_ITEM_REMOVE" },
    { CMSG_PLAYER_INVENTORY_DROP,     6, "CMSG_PLAYER_INVENTORY_DROP" },
    { SMSG_PLAYER_EQUIPMENT,          VAR, "SMSG_PLAYER_EQUIPMENT" },
    { SMSG_PLAYER_STORAGE_EQUIP,      VAR, "SMSG_PLAYER_STORAGE_EQUIP" },
    { CMSG_PLAYER_INVENTORY_USE,      8, "CMSG_PLAYER_INVENTORY_USE" },
    { SMSG_ITEM_USE_RESPONSE,         7, "SMSG_ITEM_USE_RESPONSE" },
    { CMSG_PLAYER_EQUIP,              6, "CMSG_PLAYER_EQUIP" },
    { SMSG_PLAYER_EQUIP,              7, "SMSG_PLAYER_EQUIP" },
    { CMSG_PLAYER_UNEQUIP,            4, "CMSG_PLAYER_UNEQUIP" },
    { SMSG_PLAYER_UNEQUIP,            7, "SMSG_PLAYER_UNEQUIP" },
    { SMSG_PLAYER_INVENTORY_REMOVE,   6, "SMSG_PLAYER_INVENTORY_REMOVE" },
    { SMSG_PLAYER_STAT_UPDATE_1,      8, "SMSG_PLAYER_STAT_UPDATE_1" },
    { SMSG_PLAYER_STAT_UPDATE_2,      8, "SMSG_PLAYER_STAT_UPDATE_2" },
    { CMSG_PLAYER_REBOOT,             3, "CMSG_PLAYER_REBOOT" },
    { SMSG_CHAR_SWITCH_RESPONSE,      3, "SMSG_CHAR_SWITCH_RESPONSE" },
    { SMSG_NPC_MESSAGE,               VAR, "SMSG_NPC_MESSAGE" },
    { SMSG_NPC_NEXT,                  6, "SMSG_NPC_NEXT" },
    { SMSG_NPC_CLOSE,                 6, "SMSG_NPC_CLOSE" },
    { SMSG_NPC_CHOICE,                VAR, "SMSG_NPC_CHOICE" },
    { CMSG_NPC_LIST_CHOICE,           7, "CMSG_NPC_LIST_CHOICE" },
    { CMSG_NPC_NEXT_REQUEST,          6, "CMSG_NPC_NEXT_REQUEST" },
    { CMSG_STAT_UPDATE_REQUEST,       5, "CMSG_STAT_UPDATE_REQUEST" },
    { SMSG_PLAYER_STAT_UPDATE_4,      6, "SMSG_PLAYER_STAT_UPDATE_4" },
    { SMSG_PLAYER_STAT_UPDATE_5,      44, "SMSG_PLAYER_STAT_UPDATE_5" },
    { SMSG_PLAYER_STAT_UPDATE_6,      5, "SMSG_PLAYER_STAT_UPDATE_6" },
    { CMSG_PLAYER_EMOTE,              3, "CMSG_PLAYER_EMOTE" },
    { SMSG_BEING_EMOTION,             7, "SMSG_BEING_EMOTION" },
    { SMSG_NPC_BUY_SELL_CHOICE,       6, "SMSG_NPC_BUY_SELL_CHOICE" },
    { CMSG_NPC_BUY_SELL_REQUEST,      7, "CMSG_NPC_BUY_SELL_REQUEST" },
    { SMSG_NPC_BUY,                   VAR, "SMSG_NPC_BUY" },
    { SMSG_NPC_SELL,                  VAR, "SMSG_NPC_SELL" },
    { CMSG_NPC_BUY_REQUEST,           VAR, "CMSG_NPC_BUY_REQUEST" },
    { CMSG_NPC_SELL_REQUEST,          VAR, "CMSG_NPC_SELL_REQUEST" },
    { SMSG_NPC_BUY_RESPONSE,          3, "SMSG_NPC_BUY_RESPONSE" },
    { SMSG_NPC_SELL_RESPONSE,         3, "SMSG_NPC_SELL_RESPONSE" },
    { SMSG_ADMIN_KICK_ACK,            6, "SMSG_ADMIN_KICK_ACK" },
    { CMSG_TRADE_REQUEST,             6, "CMSG_TRADE_REQUEST" },
    { SMSG_TRADE_REQUEST,             26, "SMSG_TRADE_REQUEST" },
    { CMSG_TRADE_RESPONSE,            3, "CMSG_TRADE_RESPONSE" },
    { SMSG_TRADE_RESPONSE,            3, "SMSG_TRADE_RESPONSE" },
    { CMSG_TRADE_ITEM_ADD_REQUEST,    8, "CMSG_TRADE_ITEM_ADD_REQUEST" },
    { SMSG_TRADE_ITEM_ADD,            19, "SMSG_TRADE_ITEM_ADD" },
    { CMSG_TRADE_ADD_COMPLETE,        2, "CMSG_TRADE_ADD_COMPLETE" },
    { SMSG_TRADE_OK,                  3, "SMSG_TRADE_OK" },
    { CMSG_TRADE_CANCEL_REQUEST,      2, "CMSG_TRADE_CANCEL_REQUEST" },
    { SMSG_TRADE_CANCEL,              2, "SMSG_TRADE_CANCEL" },
    { CMSG_TRADE_OK,                  2, "CMSG_TRADE_OK" },
    { SMSG_TRADE_COMPLETE,            3, "SMSG_TRADE_COMPLETE" },
    { SMSG_PLAYER_STORAGE_STATUS,     6, "SMSG_PLAYER_STORAGE_STATUS" },
    { CMSG_MOVE_TO_STORAGE,           8, "CMSG_MOVE_TO_STORAGE" },
    { SMSG_PLAYER_STORAGE_ADD,        21, "SMSG_PLAYER_STORAGE_ADD" },
    { CMSG_MOVE_FROM_STORAGE,         8, "CMSG_MOVE_FROM_STORAGE" },
    { SMSG_PLAYER_STORAGE_REMOVE,     8, "SMSG_PLAYER_STORAGE_REMOVE" },
    { CMSG_CLOSE_STORAGE,             2, "CMSG_CLOSE_STORAGE" },
    { SMSG_PLAYER_STORAGE_CLOSE,      2, "SMSG_PLAYER_STORAGE_CLOSE" },
    { CMSG_PARTY_CREATE,              26, "CMSG_PARTY_CREATE" },
    { SMSG_PARTY_CREATE,              3, "SMSG_PARTY_CREATE" },
    { SMSG_PARTY_INFO,                VAR, "SMSG_PARTY_INFO" },
    { CMSG_PARTY_INVITE,              6, "CMSG_PARTY_INVITE" },
    { SMSG_PARTY_INVITE_RESPONSE,     27, "SMSG_PARTY_INVITE_RESPONSE" },
    { SMSG_PARTY_INVITED,             30, "SMSG_PARTY_INVITED" },
    { CMSG_PARTY_INVITED,             10, "CMSG_PARTY_INVITED" },
    { CMSG_PARTY_LEAVE,               2, "CMSG_PARTY_LEAVE" },
    { SMSG_PARTY_SETTINGS,            6, "SMSG_PARTY_SETTINGS" },
    { CMSG_PARTY_SETTINGS,            6, "CMSG_PARTY_SETTINGS" },
    { CMSG_PARTY_KICK,                30, "CMSG_PARTY_KICK" },
    { SMSG_PARTY_LEAVE,               31, "SMSG_PARTY_LEAVE" },
    { SMSG_PARTY_UPDATE_HP,           10, "SMSG_PARTY_UPDATE_HP" },
    { SMSG_PARTY_UPDATE_COORDS,       10, "SMSG_PARTY_UPDATE_COORDS" },
    { CMSG_PARTY_MESSAGE,             VAR, "CMSG_PARTY_MESSAGE" },
    { SMSG_PARTY_MESSAGE,             VAR, "SMSG_PARTY_MESSAGE" },
    { SMSG_PLAYER_SKILL_UP,           11, "SMSG_PLAYER_SKILL_UP" },
    { SMSG_PLAYER_SKILLS,             VAR, "SMSG_PLAYER_SKILLS" },
    { SMSG_SKILL_FAILED,              10, "SMSG_SKILL_FAILED" },
    { CMSG_SKILL_LEVELUP_REQUEST,     4, "CMSG_SKILL_LEVELUP_REQUEST" },
    { CMSG_PLAYER_STOP_ATTACK,        2, "CMSG_PLAYER_STOP_ATTACK" },
    { SMSG_PLAYER_STATUS_CHANGE,      13, "SMSG_PLAYER_STATUS_CHANGE" },
    { SMSG_PLAYER_MOVE_TO_ATTACK,     16, "SMSG_PLAYER_MOVE_TO_ATTACK" },
    { SMSG_PLAYER_ATTACK_RANGE,       4, "SMSG_PLAYER_ATTACK_RANGE" },
    { SMSG_PLAYER_ARROW_MESSAGE,      4, "SMSG_PLAYER_ARROW_MESSAGE" },
    { SMSG_PLAYER_ARROW_EQUIP,        4, "SMSG_PLAYER_ARROW_EQUIP" },
    { SMSG_PLAYER_STAT_UPDATE_3,      14, "SMSG_PLAYER_STAT_UPDATE_3" },
    { SMSG_NPC_INT_INPUT,             6, "SMSG_NPC_INT_INPUT" },
    { CMSG_NPC_INT_RESPONSE,          10, "CMSG_NPC_INT_RESPONSE" },
    { CMSG_NPC_CLOSE,                 6, "CMSG_NPC_CLOSE" },
    { SMSG_BEING_RESURRECT,           8, "SMSG_BEING_RESURRECT" },
    { CMSG_CLIENT_QUIT,               4, "CMSG_CLIENT_QUIT" },
    { SMSG_MAP_QUIT_RESPONSE,         4, "SMSG_MAP_QUIT_RESPONSE" },
    { SMSG_PLAYER_GUILD_PARTY_INFO,   102, "SMSG_PLAYER_GUILD_PARTY_INFO" },
    { SMSG_BEING_STATUS_CHANGE,       9, "SMSG_BEING_STATUS_CHANGE" },
    { SMSG_PVP_MAP_MODE,              4, "SMSG_PVP_MAP_MODE" },
    { SMSG_PVP_SET,                   14, "SMSG_PVP_SET" },
    { SMSG_BEING_SELFEFFECT,          10, "SMSG_BEING_SELFEFFECT" },
    { SMSG_TRADE_ITEM_ADD_RESPONSE,   7, "SMSG_TRADE_ITEM_ADD_RESPONSE" },
    { SMSG_PLAYER_INVENTORY_USE,      13, "SMSG_PLAYER_INVENTORY_USE" },
    { SMSG_NPC_STR_INPUT,             6, "SMSG_NPC_STR_INPUT" },
    { CMSG_NPC_STR_RESPONSE,          VAR, "CMSG_NPC_STR_RESPONSE" },
    { SMSG_BEING_CHANGE_LOOKS2,       11, "SMSG_BEING_CHANGE_LOOKS2" },
    { SMSG_PLAYER_UPDATE_1,           54, "SMSG_PLAYER_UPDATE_1" },
    { SMSG_PLAYER_UPDATE_2,           53, "SMSG_PLAYER_UPDATE_2" },
    { SMSG_PLAYER_MOVE,               60, "SMSG_PLAYER_MOVE" },
    { SMSG_SKILL_DAMAGE,              33, "SMSG_SKILL_DAMAGE" },
    { SMSG_PLAYER_INVENTORY,          VAR, "SMSG_PLAYER_INVENTORY" },
    { SMSG_PLAYER_STORAGE_ITEMS,      VAR, "SMSG_PLAYER_STORAGE_ITEMS" },
    { SMSG_BEING_IP_RESPONSE,         10, "SMSG_BEING_IP_RESPONSE" },
    { CMSG_ONLINE_LIST,               2, "CMSG_ONLINE_LIST" },
    { SMSG_ONLINE_LIST,               VAR, "SMSG_ONLINE_LIST" },
    { SMSG_NPC_COMMAND,               16, "SMSG_NPC_COMMAND" },
    { SMSG_QUEST_SET_VAR,             8, "SMSG_QUEST_SET_VAR" },
    { SMSG_QUEST_PLAYER_VARS,         VAR, "SMSG_QUEST_PLAYER_VARS" },
    { SMSG_BEING_MOVE3,               VAR, "SMSG_BEING_MOVE3" },
    { SMSG_MAP_MASK,                  10, "SMSG_MAP_MASK" },
    { SMSG_MAP_MUSIC,                 VAR, "SMSG_MAP_MUSIC" },
    { SMSG_NPC_CHANGETITLE,           VAR, "SMSG_NPC_CHANGETITLE" },
    { SMSG_SCRIPT_MESSAGE,            VAR, "SMSG_SCRIPT_MESSAGE" },
    { SMSG_PLAYER_CLIENT_COMMAND,     VAR, "SMSG_PLAYER_CLIENT_COMMAND" },
    { SMSG_MAP_SET_TILES_TYPE,        34, "SMSG_MAP_SET_TILES_TYPE" },
    { SMSG_PLAYER_HP,                 10, "SMSG_PLAYER_HP" },
    { SMSG_PLAYER_HP_FULL,            14, "SMSG_PLAYER_HP_FULL" },

    // any server messages
    { SMSG_CONNECTION_PROBLEM,        3, "SMSG_CONNECTION_PROBLEM" },
    { CMSG_SERVER_VERSION_REQUEST,    2, "CMSG_SERVER_VERSION_REQUEST" },
    { SMSG_SERVER_VERSION_RESPONSE,   10, "SMSG_SERVER_VERSION_RESPONSE" },
    { CMSG_CLIENT_DISCONNECT,         2, "CMSG_CLIENT_DISCONNECT" },
};

const unsigned int BUFFER_SIZE = 65536;

int networkThread(void *data)
{
    auto *network = static_cast<Network*>(data);

    if (!network->realConnect())
        return -1;

    network->receive();

    return 0;
}

Network *Network::mInstance = nullptr;

Network::Network():
    mInBuffer(new char[BUFFER_SIZE]),
    mOutBuffer(new char[BUFFER_SIZE])
{
    SDLNet_Init();

    mInstance = this;

    for (const auto &packetInfo : packet_infos)
    {
        assert(packetInfo.length != 0);
        mPacketInfo[packetInfo.id] = &packetInfo;
    }
}

Network::~Network()
{
    clearHandlers();

    if (mState != IDLE && mState != NET_ERROR)
        disconnect();

    mInstance = nullptr;

    delete[] mInBuffer;
    delete[] mOutBuffer;

    SDLNet_Quit();
}

bool Network::connect(const ServerInfo &server)
{
    if (mState != IDLE && mState != NET_ERROR)
    {
        logger->log("Tried to connect an already connected socket!");
        assert(false);
        return false;
    }

    if (server.hostname.empty())
    {
        setError(_("Empty address given to Network::connect()!"));
        return false;
    }

    logger->log("Network::Connecting to %s:%i", server.hostname.c_str(),
                                                server.port);

    mServer.hostname = server.hostname;
    mServer.port = server.port;

    // Reset to sane values
    mOutSize = 0;
    mInSize = 0;
    mToSkip = 0;

    mState = CONNECTING;
    mWorkerThread = SDL_CreateThread(networkThread, "Network", this);
    if (!mWorkerThread)
    {
        setError("Unable to create network worker thread");
        return false;
    }

    return true;
}

void Network::disconnect()
{
    mState = IDLE;

    if (mWorkerThread)
    {
        SDL_WaitThread(mWorkerThread, nullptr);
        mWorkerThread = nullptr;
    }

    if (mSocket)
    {
        SDLNet_TCP_Close(mSocket);
        mSocket = nullptr;
    }
}

void Network::registerHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; ++i)
    {
        mMessageHandlers[*i] = handler;
    }

    handler->setNetwork(this);
}

void Network::unregisterHandler(MessageHandler *handler)
{
    for (const Uint16 *i = handler->handledMessages; *i; ++i)
    {
        mMessageHandlers.erase(*i);
    }

    handler->setNetwork(nullptr);
}

void Network::clearHandlers()
{
    for (auto& [_, messageHandler] : mMessageHandlers)
    {
        messageHandler->setNetwork(nullptr);
    }
    mMessageHandlers.clear();
}

void Network::dispatchMessages()
{
    MutexLocker lock(&mMutex);

    while (true) {
        // Not even a message ID has been received
        if (mInSize < 2)
            break;

        const uint16_t msgId = readWord(0);

        auto packetInfoIt = mPacketInfo.find(msgId);
        if (packetInfoIt == mPacketInfo.end())
        {
            auto error = strprintf("Unknown packet 0x%x received.", msgId);
            logger->error(error);
            break;
        }

        auto packetInfo = packetInfoIt->second;

        // Determine the length of the packet
        uint16_t len = packetInfo->length;
        if (len == VAR)
        {
            // We have not received the length yet
            if (mInSize < 4)
                break;

            len = readWord(2);

            if (len < 4)
            {
                auto error = strprintf("Variable length packet 0x%x has invalid length %d.",
                                       msgId, len);
                logger->error(error);
                break;
            }
        }

        // The message has not been fully received yet
        if (mInSize < len)
            break;

        MessageIn message(mInBuffer, len);

        // Dispatch the message to the appropriate handler
        auto iter = mMessageHandlers.find(msgId);
        if (iter != mMessageHandlers.end())
        {
#ifdef DEBUG
            logger->log("Handling %s (0x%x) of length %d", packetInfo->name, msgId, len);
#endif

            iter->second->handleMessage(message);
        }
        else
        {
            logger->log("Unhandled %s (0x%x) of length %d", packetInfo->name, msgId, len);
        }

        skip(len);
    }
}

void Network::flush()
{
    if (!mOutSize || mState != CONNECTED)
        return;

    int ret;

    MutexLocker lock(&mMutex);
    ret = SDLNet_TCP_Send(mSocket, mOutBuffer, mOutSize);
    if (ret < (int)mOutSize)
    {
        setError("Error in SDLNet_TCP_Send(): " +
                 std::string(SDLNet_GetError()));
    }
    mOutSize = 0;
}

void Network::skip(int len)
{
    MutexLocker lock(&mMutex);
    mToSkip += len;
    if (!mInSize)
        return;

    if (mInSize >= mToSkip)
    {
        mInSize -= mToSkip;
        memmove(mInBuffer, mInBuffer + mToSkip, mInSize);
        mToSkip = 0;
    }
    else
    {
        mToSkip -= mInSize;
        mInSize = 0;
    }
}

bool Network::realConnect()
{
    IPaddress ipAddress;

    if (SDLNet_ResolveHost(&ipAddress, mServer.hostname.c_str(),
                           mServer.port) == -1)
    {
        std::string errorMessage = strprintf(_("Unable to resolve host \"%s\""),
                                             mServer.hostname.c_str());
        setError(errorMessage);
        logger->log("SDLNet_ResolveHost: %s", errorMessage.c_str());
        return false;
    }

    mState = CONNECTING;

    mSocket = SDLNet_TCP_Open(&ipAddress);
    if (!mSocket)
    {
        logger->log("Error in SDLNet_TCP_Open(): %s", SDLNet_GetError());
        setError(SDLNet_GetError());
        return false;
    }

    logger->log("Network::Started session with %s:%i",
                ipToString(ipAddress.host), ipAddress.port);

    mState = CONNECTED;

    return true;
}

void Network::receive()
{
    SDLNet_SocketSet set;

    if (!(set = SDLNet_AllocSocketSet(1)))
    {
        setError("Error in SDLNet_AllocSocketSet(): " +
                 std::string(SDLNet_GetError()));
        return;
    }

    if (SDLNet_TCP_AddSocket(set, mSocket) == -1)
    {
        setError("Error in SDLNet_AddSocket(): " +
                 std::string(SDLNet_GetError()));
    }

    while (mState == CONNECTED)
    {
        // TODO Try to get this to block all the time while still being able
        // to escape the loop
        int numReady = SDLNet_CheckSockets(set, ((Uint32)500));
        int ret;
        switch (numReady)
        {
            case -1:
                logger->log("Error: SDLNet_CheckSockets");
                // FALLTHROUGH
            case 0:
                break;

            case 1:
            {
                // Receive data from the socket
                MutexLocker lock(&mMutex);
                ret = SDLNet_TCP_Recv(mSocket, mInBuffer + mInSize, BUFFER_SIZE - mInSize);

                if (!ret)
                {
                    // We got disconnected
                    mState = IDLE;
                    logger->log("Disconnected.");
                }
                else if (ret < 0)
                {
                    setError(_("Connection to server terminated. ") +
                             std::string(SDLNet_GetError()));
                }
                else
                {
                    mInSize += ret;
                    if (mToSkip)
                    {
                        if (mInSize >= mToSkip)
                        {
                            mInSize -= mToSkip;
                            memmove(mInBuffer, mInBuffer + mToSkip, mInSize);
                            mToSkip = 0;
                        }
                        else
                        {
                            mToSkip -= mInSize;
                            mInSize = 0;
                        }
                    }
                }
                break;
            }

            default:
                // more than one socket is ready..
                // this should not happen since we only listen once socket.
                std::stringstream errorStream;
                errorStream << "Error in SDLNet_TCP_Recv(), " << numReady
                            << " sockets are ready: " << SDLNet_GetError();
                setError(errorStream.str());
                break;
        }
    }

    if (SDLNet_TCP_DelSocket(set, mSocket) == -1)
    {
        logger->log("Error in SDLNet_DelSocket(): %s", SDLNet_GetError());
    }

    SDLNet_FreeSocketSet(set);
}

void Network::setError(const std::string &error)
{
    logger->log("Network error: %s", error.c_str());
    mError = error;
    mState = NET_ERROR;
}

uint16_t Network::readWord(int pos)
{
    uint16_t value;
    memcpy(&value, mInBuffer + pos, sizeof(uint16_t));
    return SDL_SwapLE16(value);
}

} // namespace TmwAthena
