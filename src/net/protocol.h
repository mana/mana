/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifndef _TMW_PROTOCOL_
#define _TMW_PROTOCOL_

// Packets from server to client
#define SMSG_LOGIN_SUCCESS           0x0073 /**< Contains starting location */
#define SMSG_PLAYER_UPDATE_1         0x01d8
#define SMSG_PLAYER_UPDATE_2         0x01d9
#define SMSG_PLAYER_MOVE             0x01da /**< A nearby player moves */
#define SMSG_PLAYER_STAT_UPDATE_1    0x00b0
#define SMSG_PLAYER_STAT_UPDATE_2    0x00b1
#define SMSG_PLAYER_STAT_UPDATE_3    0x0141
#define SMSG_PLAYER_STAT_UPDATE_4    0x00bc
#define SMSG_PLAYER_STAT_UPDATE_5    0x00bd
#define SMSG_PLAYER_STAT_UPDATE_6    0x00be
#define SMSG_WHO_ANSWER              0x00c2
#define SMSG_PLAYER_WARP             0x0091 /**< Warp player to map/location */
#define SMSG_PLAYER_INVENTORY        0x01ee
#define SMSG_PLAYER_INVENTORY_ADD    0x00a0
#define SMSG_PLAYER_INVENTORY_REMOVE 0x00af
#define SMSG_PLAYER_INVENTORY_USE    0x01c8
#define SMSG_PLAYER_EQUIPMENT        0x00a4
#define SMSG_PLAYER_EQUIP            0x00aa
#define SMSG_PLAYER_UNEQUIP          0x00ac
#define SMSG_PLAYER_ARROW_EQUIP      0x013c
#define SMSG_PLAYER_ARROW_MESSAGE    0x013b
#define SMSG_PLAYER_SKILLS           0x010f
#define SMSG_SKILL_FAILED            0x0110
#define SMSG_ITEM_USE_RESPONSE       0x00a8
#define SMSG_ITEM_VISIBLE            0x009d /**< An item is on the floor */
#define SMSG_ITEM_DROPPED            0x009e /**< An item is dropped */
#define SMSG_ITEM_REMOVE             0x00a1 /**< An item disappers */
#define SMSG_BEING_VISIBLE           0x0078
#define SMSG_BEING_MOVE              0x007b /**< A nearby monster moves */
#define SMSG_BEING_REMOVE            0x0080
#define SMSG_BEING_CHANGE_LOOKS      0x00c3
#define SMSG_BEING_LEVELUP           0x019b
#define SMSG_BEING_EMOTION           0x00c0
#define SMSG_BEING_ACTION            0x008a /**< Attack, sit, stand up, ... */
#define SMSG_BEING_CHAT              0x008d /**< A being talks */
#define SMSG_BEING_NAME_RESPONSE     0x0095 /**< Has to be requested */
#define SMSG_NPC_MESSAGE             0x00b4
#define SMSG_NPC_NEXT                0x00b5
#define SMSG_NPC_CLOSE               0x00b6
#define SMSG_NPC_CHOICE              0x00b7 /**< Display a choice */
#define SMSG_NPC_BUY_SELL_CHOICE     0x00c4
#define SMSG_NPC_BUY                 0x00c6
#define SMSG_NPC_SELL                0x00c7
#define SMSG_NPC_BUY_RESPONSE        0x00ca
#define SMSG_NPC_SELL_RESPONSE       0x00cb
#define SMSG_PLAYER_CHAT             0x008e /**< Player talks */
#define SMSG_GM_CHAT                 0x009a /**< GM announce */
#define SMSG_WALK_RESPONSE           0x0087
#define SMSG_TRADE_REQUEST           0x00e5 /**< Receiving a request to trade */
#define SMSG_TRADE_RESPONSE          0x00e7
#define SMSG_TRADE_ITEM_ADD          0x00e9
#define SMSG_TRADE_ITEM_ADD_RESPONSE 0x01b1 /**< Not standard eAthena! */
#define SMSG_TRADE_OK                0x00ec
#define SMSG_TRADE_CANCEL            0x00ee
#define SMSG_TRADE_COMPLETE          0x00f0

// Packets from client to server
#define CMSG_TRADE_RESPONSE          0x00e6
#define CMSG_ITEM_PICKUP             0x009f
#define CMSG_MAP_LOADED              0x007d
#define CMSG_NPC_BUY_REQUEST         0x00c8
#define CMSG_NPC_BUY_SELL_REQUEST    0x00c5
#define CMSG_CHAT_MESSAGE            0x008c
#define CMSG_NPC_LIST_CHOICE         0x00b8
#define CMSG_NPC_NEXT_REQUEST        0x00b9
#define CMSG_NPC_SELL_REQUEST        0x00c9
#define CMSG_SKILL_LEVELUP_REQUEST   0x0112
#define CMSG_STAT_UPDATE_REQUEST     0x00bb
#define CMSG_TRADE_ITEM_ADD_REQUEST  0x00e8
#define CMSG_TRADE_CANCEL_REQUEST    0x00ed
#define CMSG_TRADE_ADD_COMPLETE      0x00eb
#define CMSG_TRADE_OK                0x00ef
#define CMSG_NPC_TALK                0x0090
#define CMSG_TRADE_REQUEST           0x00e4
#define CMSG_PLAYER_INVENTORY_USE    0x00a7
#define CMSG_PLAYER_INVENTORY_DROP   0x00a2
#define CMSG_PLAYER_EQUIP            0x00a9
#define CMSG_PLAYER_UNEQUIP          0x00ab

/**
 * Enumerated type for communicated messages
 * - PAMSG_*: from client to account server
 * - APMSG_*: from account server to client
 * - PCMSG_*: from client to chat server
 * - CPMSG_*: from chat server to client
 * - PGMSG_*: from client to game server
 * - GPMSG_*: from game server to client
 * Components: B byte, W word, D double word, S variable-size string
 */
enum {
    // Login/Register
    PAMSG_REGISTER                 = 0x0000, // L version, S username, S password, S email
    APMSG_REGISTER_RESPONSE        = 0x0002, // B error
    PAMSG_UNREGISTER               = 0x0003, // -
    APMSG_UNREGISTER_RESPONSE      = 0x0004, // B error
    PAMSG_LOGIN                    = 0x0010, // L version, S username, S password
    APMSG_LOGIN_RESPONSE           = 0x0012, // B error
    PAMSG_LOGOUT                   = 0x0013, // -
    APMSG_LOGOUT_RESPONSE          = 0x0014, // B error
    PAMSG_CHAR_CREATE              = 0x0020, // S name, B hair style, B hair color, B gender, W*6 stats
    APMSG_CHAR_CREATE_RESPONSE     = 0x0021, // B error
    PAMSG_CHAR_DELETE              = 0x0022, // B index
    APMSG_CHAR_DELETE_RESPONSE     = 0x0023, // B error
    PAMSG_CHAR_LIST                = 0x0024, // -
    APMSG_CHAR_LIST_RESPONSE       = 0x0025, // B number, { B index, S name, B gender, B hair style, B hair color, B level, W money, W*6 stats, S mapname, W*2 position }*
    PAMSG_CHAR_SELECT              = 0x0026, // B index
    APMSG_CHAR_SELECT_RESPONSE     = 0x0027, // B error, S mapname, W*2 position
    PAMSG_EMAIL_CHANGE             = 0x0030, // S email
    APMSG_EMAIL_CHANGE_RESPONSE    = 0x0031, // B error
    PAMSG_EMAIL_GET                = 0x0032, // -
    APMSG_EMAIL_GET_RESPONSE       = 0x0033, // B error, S email
    PAMSG_PASSWORD_CHANGE          = 0x0034, // S old password, S new password
    APMSG_PASSWORD_CHANGE_RESPONSE = 0x0035, // B error
    PAMSG_ENTER_WORLD              = 0x0040, // -
    APMSG_ENTER_WORLD_RESPONSE     = 0x0041, // B error, S address, W port, B*32 token
    PAMSG_ENTER_CHAT               = 0x0042, // -
    APMSG_ENTER_CHAT_RESPONSE      = 0x0043, // B error, S address, W port, B*32 token
    PGMSG_CONNECT                  = 0x0050, // B*32 token
    GPMSG_CONNECT_RESPONSE         = 0x0051, // B error
    PCMSG_CONNECT                  = 0x0053, // B*32 token
    CPMSG_CONNECT_RESPONSE         = 0x0054, // B error

    // Game
    PGMSG_PICKUP                   = 0x0110,
    GPMSG_PICKUP_RESPONSE          = 0x0111,
    GPMSG_BEING_ENTER              = 0x0200, // B type, L being id
                                             // player: S name, B hair style, B hair color, B gender
    GPMSG_BEING_LEAVE              = 0x0201, // B type, L being id
    PGMSG_WALK                     = 0x0260, // L*2 destination
    PGMSG_SAY                      = 0x02A0, // S text
    GPMSG_SAY                      = 0x02A1, // S being, S text
    PGMSG_USE_ITEM                 = 0x0300, // L item id
    GPMSG_USE_RESPONSE             = 0x0301, // B error
    PGMSG_EQUIP                    = 0x0302, // L item id, B slot
    GPMSG_EQUIP_RESPONSE           = 0x0303, // B error

    // Chat
    CPMSG_ERROR                    = 0x0401, // B error
    CPMSG_ANNOUNCEMENT             = 0x0402, // S text
    CPMSG_PRIVMSG                  = 0x0403, // S user, S text
    CPMSG_PUBMSG                   = 0x0404, // W channel, S user, S text
    PCMSG_CHAT                     = 0x0410, // S text, W channel
    PCMSG_ANNOUNCE                 = 0x0411, // S text
    PCMSG_PRIVMSG                  = 0x0412, // S user, S text
    // -- Channeling
    PCMSG_REGISTER_CHANNEL            = 0x0413, // B pub/priv, S name, S announcement, S password
    CPMSG_REGISTER_CHANNEL_RESPONSE   = 0x0414, // B error
    PCMSG_UNREGISTER_CHANNEL          = 0x0415, // W channel
    CPMSG_UNREGISTER_CHANNEL_RESPONSE = 0x0416, // B error
    CPMSG_CHANNEL_EVENT               = 0x0418, // W channel, B event, S user
    PCMSG_ENTER_CHANNEL               = 0x0419, // W channel, S password
    CPMSG_ENTER_CHANNEL_RESPONSE      = 0x0420, // B error
    PCMSG_QUIT_CHANNEL                = 0x0421, // W channel
    CPMSG_QUIT_CHANNEL_RESPONSE       = 0x0422, // B error

    XXMSG_INVALID = 0x7FFF
};

// Generic return values

enum {
    ERRMSG_OK = 0,                      // everything is fine
    ERRMSG_FAILURE,                     // the action failed
    ERRMSG_NO_LOGIN,                    // the user is not yet logged
    ERRMSG_NO_CHARACTER_SELECTED,       // the user needs a character
    ERRMSG_INSUFFICIENT_RIGHTS,         // the user is not privileged
    ERRMSG_INVALID_ARGUMENT,            // part of the received message was invalid
};

// Login specific return values
enum {
    LOGIN_INVALID_VERSION = 0x40,       // the user is using an incompatible protocol
    LOGIN_SERVER_FULL                   // the server is overloaded
};


/** Encodes coords and direction in 3 bytes data */
void set_coordinates(char *data, unsigned short x, unsigned short y, unsigned char direction);

#endif
