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

#ifndef TA_PROTOCOL_H
#define TA_PROTOCOL_H

#include <cstdint>

namespace TmwAthena {

enum {
    JOB = 0xa,

    STRENGTH = 0xd,
    AGILITY,
    VITALITY,
    INTELLIGENCE,
    DEXTERITY,
    LUCK,

    ATK,
    DEF,
    MATK,
    MDEF,
    HIT,
    FLEE,
    CRIT
};

enum
{
    SPRITE_BASE = 0,
    SPRITE_SHOE,
    SPRITE_BOTTOMCLOTHES,
    SPRITE_TOPCLOTHES,
    SPRITE_MISC1,
    SPRITE_MISC2,
    SPRITE_HAIR,
    SPRITE_HAT,
    SPRITE_CAPE,
    SPRITE_GLOVES,
    SPRITE_WEAPON,
    SPRITE_SHIELD,
    SPRITE_VECTOREND
};

enum class LOOK : uint8_t
{
    BASE            = 0,
    HAIR            = 1,
    WEAPON          = 2,
    HEAD_BOTTOM     = 3,
    HEAD_TOP        = 4,
    HEAD_MID        = 5,
    HAIR_COLOR      = 6,
    CLOTHES_COLOR   = 7,
    SHIELD          = 8,
    SHOES           = 9,
    GLOVES          = 10,
    CAPE            = 11,
    MISC1           = 12,
    MISC2           = 13,
};

enum NpcCommand
{
    NPC_REQUEST_LANG = 0,
    NPC_CAMERA_ACTOR = 1,
    NPC_CAMERA_POS = 2,
    NPC_CAMERA_RESTORE = 3,
    NPC_CAMERA_RELATIVE = 4,
    NPC_CLOSE_DIALOG = 5,
    NPC_SHOW_AVATAR = 6,
    NPC_SET_AVATAR_DIRECTION = 7,
    NPC_SET_AVATAR_ACTION = 8,
    NPC_CLEAR_DIALOG = 9,
    NPC_REQUEST_ITEM = 10,
    NPC_REQUEST_ITEM_INDEX = 11,
    NPC_REQUEST_ITEMS = 12,
};

static const int INVENTORY_OFFSET = 2;
static const int STORAGE_OFFSET = 1;

enum {
    // login server messages
    SMSG_UPDATE_HOST               = 0x0063, // update host notify
    CMSG_LOGIN_REGISTER            = 0x0064, // account login
    SMSG_LOGIN_DATA                = 0x0069, // account login success
    SMSG_LOGIN_ERROR               = 0x006a, // account login error

    // char server messages
    CMSG_CHAR_PASSWORD_CHANGE      = 0x0061, // change password
    SMSG_CHAR_PASSWORD_RESPONSE    = 0x0062, // change password result
    CMSG_CHAR_SERVER_CONNECT       = 0x0065, // connect char
    CMSG_CHAR_SELECT               = 0x0066, // select character
    CMSG_CHAR_CREATE               = 0x0067, // create character
    CMSG_CHAR_DELETE               = 0x0068, // delete character
    SMSG_CHAR_LOGIN                = 0x006b, // connect char success
    SMSG_CHAR_LOGIN_ERROR          = 0x006c, // connect char error
    SMSG_CHAR_CREATE_SUCCEEDED     = 0x006d, // create character success
    SMSG_CHAR_CREATE_FAILED        = 0x006e, // create character error
    SMSG_CHAR_DELETE_SUCCEEDED     = 0x006f, // delete character success
    SMSG_CHAR_DELETE_FAILED        = 0x0070, // delete character error
    SMSG_CHAR_MAP_INFO             = 0x0071, // select character success

    // map server messages
    CMSG_MAP_SERVER_CONNECT        = 0x0072, // connect map
    SMSG_MAP_LOGIN_SUCCESS         = 0x0073, // connect map success
    SMSG_BEING_VISIBLE             = 0x0078, // being appear notify
    SMSG_BEING_MOVE                = 0x007b, // being move notify
    SMSG_BEING_SPAWN               = 0x007c, // being spawn notify
    CMSG_MAP_LOADED                = 0x007d, // map loaded
    CMSG_MAP_PING                  = 0x007e, // ping
    SMSG_SERVER_PING               = 0x007f, // pong
    SMSG_BEING_REMOVE              = 0x0080, // remove being notify
    CMSG_PLAYER_CHANGE_DEST        = 0x0085, // walk
    SMSG_WALK_RESPONSE             = 0x0087, // walk success
    SMSG_PLAYER_STOP               = 0x0088, // stop walking notify
    CMSG_PLAYER_CHANGE_ACT         = 0x0089, // player action
    SMSG_BEING_ACTION              = 0x008a, // being action notify
    CMSG_CHAT_MESSAGE              = 0x008c, // global chat
    SMSG_BEING_CHAT                = 0x008d, // global chat notify
    SMSG_PLAYER_CHAT               = 0x008e, // global chat result
    CMSG_NPC_TALK                  = 0x0090, // npc click
    SMSG_PLAYER_WARP               = 0x0091, // change map notify
    SMSG_CHANGE_MAP_SERVER         = 0x0092, // change map server notify
    CMSG_NAME_REQUEST              = 0x0094, // get being name
    SMSG_BEING_NAME_RESPONSE       = 0x0095, // get being name result
    CMSG_CHAT_WHISPER              = 0x0096, // whisper
    SMSG_WHISPER                   = 0x0097, // receive whisper
    SMSG_WHISPER_RESPONSE          = 0x0098, // whisper result
    SMSG_GM_CHAT                   = 0x009a, // gm announcement notify
    CMSG_PLAYER_CHANGE_DIR         = 0x009b, // face direction
    SMSG_BEING_CHANGE_DIRECTION    = 0x009c, // face direction notify
    SMSG_ITEM_VISIBLE              = 0x009d, // item visible notify
    SMSG_ITEM_DROPPED              = 0x009e, // item dropped notify
    CMSG_ITEM_PICKUP               = 0x009f, // item pickup
    SMSG_PLAYER_INVENTORY_ADD      = 0x00a0, // inventory add notify
    SMSG_ITEM_REMOVE               = 0x00a1, // flooritem delete notify
    CMSG_PLAYER_INVENTORY_DROP     = 0x00a2, // drop item
    SMSG_PLAYER_EQUIPMENT          = 0x00a4, // inventory equipment notify
    SMSG_PLAYER_STORAGE_EQUIP      = 0x00a6, // storage equipment notify
    CMSG_PLAYER_INVENTORY_USE      = 0x00a7, // use item
    SMSG_ITEM_USE_RESPONSE         = 0x00a8, // use item result
    CMSG_PLAYER_EQUIP              = 0x00a9, // equip item
    SMSG_PLAYER_EQUIP              = 0x00aa, // equip item result
    CMSG_PLAYER_UNEQUIP            = 0x00ab, // unequip item
    SMSG_PLAYER_UNEQUIP            = 0x00ac, // unequip item result
    SMSG_PLAYER_INVENTORY_REMOVE   = 0x00af, // inventory delete notify
    SMSG_PLAYER_STAT_UPDATE_1      = 0x00b0, // player stat update 1 notify
    SMSG_PLAYER_STAT_UPDATE_2      = 0x00b1, // player stat update 2 notify
    CMSG_PLAYER_REBOOT             = 0x00b2, // respawn or switch character
    SMSG_CHAR_SWITCH_RESPONSE      = 0x00b3, // character switch success
    SMSG_NPC_MESSAGE               = 0x00b4, // script message notify
    SMSG_NPC_NEXT                  = 0x00b5, // (reverse) script next
    SMSG_NPC_CLOSE                 = 0x00b6, // (reverse) script close
    SMSG_NPC_CHOICE                = 0x00b7, // (reverse) script menu
    CMSG_NPC_LIST_CHOICE           = 0x00b8, // (reverse) script menu result
    CMSG_NPC_NEXT_REQUEST          = 0x00b9, // (reverse) script next result
    CMSG_STAT_UPDATE_REQUEST       = 0x00bb, // stat increase
    SMSG_PLAYER_STAT_UPDATE_4      = 0x00bc, // stat increase result
    SMSG_PLAYER_STAT_UPDATE_5      = 0x00bd, // player stat update 5 notify
    SMSG_PLAYER_STAT_UPDATE_6      = 0x00be, // stat price notify
    CMSG_PLAYER_EMOTE              = 0x00bf, // emote
    SMSG_BEING_EMOTION             = 0x00c0, // emote notify
    SMSG_NPC_BUY_SELL_CHOICE       = 0x00c4, // npc click result shop
    CMSG_NPC_BUY_SELL_REQUEST      = 0x00c5, // npc shop buy/sell select
    SMSG_NPC_BUY                   = 0x00c6, // npc shop buy select result
    SMSG_NPC_SELL                  = 0x00c7, // npc shop sell select result
    CMSG_NPC_BUY_REQUEST           = 0x00c8, // npc shop buy
    CMSG_NPC_SELL_REQUEST          = 0x00c9, // npc shop sell
    SMSG_NPC_BUY_RESPONSE          = 0x00ca, // npc shop buy result
    SMSG_NPC_SELL_RESPONSE         = 0x00cb, // npc shop sell result
    SMSG_ADMIN_KICK_ACK            = 0x00cd, // kick result
    CMSG_TRADE_REQUEST             = 0x00e4, // trade please
    SMSG_TRADE_REQUEST             = 0x00e5, // incoming trade request
    CMSG_TRADE_RESPONSE            = 0x00e6, // incoming trade request result
    SMSG_TRADE_RESPONSE            = 0x00e7, // trade please result
    CMSG_TRADE_ITEM_ADD_REQUEST    = 0x00e8, // trade add
    SMSG_TRADE_ITEM_ADD            = 0x00e9, // trade item added notify
    CMSG_TRADE_ADD_COMPLETE        = 0x00eb, // trade lock
    SMSG_TRADE_OK                  = 0x00ec, // trade lock notify
    CMSG_TRADE_CANCEL_REQUEST      = 0x00ed, // trade cancel
    SMSG_TRADE_CANCEL              = 0x00ee, // trade cancel notify
    CMSG_TRADE_OK                  = 0x00ef, // trade commit
    SMSG_TRADE_COMPLETE            = 0x00f0, // trade complete notify
    SMSG_PLAYER_STORAGE_STATUS     = 0x00f2, // storage size notify
    CMSG_MOVE_TO_STORAGE           = 0x00f3, // storage put
    SMSG_PLAYER_STORAGE_ADD        = 0x00f4, // storage added notify
    CMSG_MOVE_FROM_STORAGE         = 0x00f5, // storage take
    SMSG_PLAYER_STORAGE_REMOVE     = 0x00f6, // storage removed notify
    CMSG_CLOSE_STORAGE             = 0x00f7, // storage close
    SMSG_PLAYER_STORAGE_CLOSE      = 0x00f8, // storage closed notify
    CMSG_PARTY_CREATE              = 0x00f9, // party create
    SMSG_PARTY_CREATE              = 0x00fa, // party create result
    SMSG_PARTY_INFO                = 0x00fb, // party info notify
    CMSG_PARTY_INVITE              = 0x00fc, // party invite
    SMSG_PARTY_INVITE_RESPONSE     = 0x00fd, // party invite result
    SMSG_PARTY_INVITED             = 0x00fe, // (reverse) party invitation
    CMSG_PARTY_INVITED             = 0x00ff, // (reverse) party invitation result
    CMSG_PARTY_LEAVE               = 0x0100, // party leave
    SMSG_PARTY_SETTINGS            = 0x0101, // party option notify
    CMSG_PARTY_SETTINGS            = 0x0102, // party option
    CMSG_PARTY_KICK                = 0x0103, // party kick
    SMSG_PARTY_LEAVE               = 0x0105, // party left notify
    SMSG_PARTY_UPDATE_HP           = 0x0106, // party hp notify
    SMSG_PARTY_UPDATE_COORDS       = 0x0107, // party xy notify
    CMSG_PARTY_MESSAGE             = 0x0108, // party message
    SMSG_PARTY_MESSAGE             = 0x0109, // party message notify
    SMSG_PLAYER_SKILL_UP           = 0x010e, // skill raise result
    SMSG_PLAYER_SKILLS             = 0x010f, // skill info notify
    SMSG_SKILL_FAILED              = 0x0110, // skill failed
    CMSG_SKILL_LEVELUP_REQUEST     = 0x0112, // skill raise
    CMSG_PLAYER_STOP_ATTACK        = 0x0118, // attack stop
    SMSG_PLAYER_STATUS_CHANGE      = 0x0119, // player option notify
    SMSG_PLAYER_MOVE_TO_ATTACK     = 0x0139, // player move attack range notify
    SMSG_PLAYER_ATTACK_RANGE       = 0x013a, // player attack range notify
    SMSG_PLAYER_ARROW_MESSAGE      = 0x013b, // player arrow fail notify
    SMSG_PLAYER_ARROW_EQUIP        = 0x013c, // player arrow equip notify
    SMSG_PLAYER_STAT_UPDATE_3      = 0x0141, // player stat update 3
    SMSG_NPC_INT_INPUT             = 0x0142, // (reverse) script input integer
    CMSG_NPC_INT_RESPONSE          = 0x0143, // (reverse) script input integer result
    CMSG_NPC_CLOSE                 = 0x0146, // (reverse) script close response
    SMSG_BEING_RESURRECT           = 0x0148, // being resurrected notify
    CMSG_CLIENT_QUIT               = 0x018a, // client quit
    SMSG_MAP_QUIT_RESPONSE         = 0x018b, // client quit result
    SMSG_PLAYER_GUILD_PARTY_INFO   = 0x0195, // guild party info notify
    SMSG_BEING_STATUS_CHANGE       = 0x0196, // being status change notify
    SMSG_PVP_MAP_MODE              = 0x0199, // map pvp status
    SMSG_PVP_SET                   = 0x019a, // being pvp status
    SMSG_BEING_SELFEFFECT          = 0x019b, // being effect
    SMSG_TRADE_ITEM_ADD_RESPONSE   = 0x01b1, // trade add result
    SMSG_PLAYER_INVENTORY_USE      = 0x01c8, // use item result
    SMSG_NPC_STR_INPUT             = 0x01d4, // (reverse) script input string
    CMSG_NPC_STR_RESPONSE          = 0x01d5, // (reverse) script input string result
    SMSG_BEING_CHANGE_LOOKS2       = 0x01d7, // being change look
    SMSG_PLAYER_UPDATE_1           = 0x01d8, // player appear notify
    SMSG_PLAYER_UPDATE_2           = 0x01d9, // player appear notify
    SMSG_PLAYER_MOVE               = 0x01da, // player move notify
    SMSG_SKILL_DAMAGE              = 0x01de, // deal skill damage
    SMSG_PLAYER_INVENTORY          = 0x01ee, // inventory list notify
    SMSG_PLAYER_STORAGE_ITEMS      = 0x01f0, // storage list notify
    SMSG_BEING_IP_RESPONSE         = 0x020c, // player ip notify
    CMSG_ONLINE_LIST               = 0x0210, // online list request
    SMSG_ONLINE_LIST               = 0x0211, // advanced online list
    SMSG_NPC_COMMAND               = 0x0212, // npc command
    SMSG_QUEST_SET_VAR             = 0x0214, // send quest
    SMSG_QUEST_PLAYER_VARS         = 0x0215, // send all quest
    SMSG_BEING_MOVE3               = 0x0225, // being move 3
    SMSG_MAP_MASK                  = 0x0226, // send map mask
    SMSG_MAP_MUSIC                 = 0x0227, // change map music
    SMSG_NPC_CHANGETITLE           = 0x0228, // npc change title
    SMSG_SCRIPT_MESSAGE            = 0x0229, // script message
    SMSG_PLAYER_CLIENT_COMMAND     = 0x0230, // remote client command
    SMSG_MAP_SET_TILES_TYPE        = 0x0231, // send area collision
    SMSG_PLAYER_HP                 = 0x0232, // send hp update
    SMSG_PLAYER_HP_FULL            = 0x0233, // send full hp

    // any server messages
    SMSG_CONNECTION_PROBLEM        = 0x0081, // connect foo error
    CMSG_SERVER_VERSION_REQUEST    = 0x7530, // version
    SMSG_SERVER_VERSION_RESPONSE   = 0x7531, // version result
    CMSG_CLIENT_DISCONNECT         = 0x7532, // disconnect
};

}

#endif
