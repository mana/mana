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

#ifndef EA_PROTOCOL_H
#define EA_PROTOCOL_H

static const int INVENTORY_OFFSET = 2;
static const int STORAGE_OFFSET = 1;

/*********************************
 * Packets from server to client *
 *********************************/
#define SMSG_SERVER_PING             0x007f /**< Contains server tick */
#define SMSG_CONNECTION_PROBLEM      0x0081

#define SMSG_UPDATE_HOST             0x0063 /**< Custom update host packet */
#define SMSG_LOGIN_DATA              0x0069
#define SMSG_LOGIN_ERROR             0x006a

#define SMSG_CHAR_LOGIN              0x006b
#define SMSG_CHAR_LOGIN_ERROR        0x006c
#define SMSG_CHAR_CREATE_SUCCEEDED   0x006d
#define SMSG_CHAR_CREATE_FAILED      0x007e
#define SMSG_CHAR_DELETE_SUCCEEDED   0x006f
#define SMSG_CHAR_DELETE_FAILED      0x0070
#define SMSG_CHAR_MAP_INFO           0x0071
#define SMSG_CHAR_PASSWORD_RESPONSE  0x0062 /**< Custom packet reply to password change request */

#define SMSG_MAP_LOGIN_SUCCESS       0x0073 /**< Contains starting location */
#define SMSG_PLAYER_UPDATE_1         0x01d8
#define SMSG_PLAYER_UPDATE_2         0x01d9
#define SMSG_PLAYER_MOVE             0x01da /**< A nearby player moves */
#define SMSG_PLAYER_STOP             0x0088 /**< Stop walking, set position */
#define SMSG_PLAYER_MOVE_TO_ATTACK   0x0139 /**< Move to within attack range */
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
#define SMSG_PLAYER_ATTACK_RANGE     0x013a
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
#define SMSG_BEING_SPAWN             0x007c /**< A being spawns nearby */
#define SMSG_BEING_MOVE2             0x0086 /**< New eAthena being moves */
#define SMSG_BEING_REMOVE            0x0080
#define SMSG_BEING_CHANGE_LOOKS      0x00c3
#define SMSG_BEING_CHANGE_LOOKS2     0x01d7 /**< Same as 0x00c3, but 16 bit ID */
#define SMSG_BEING_SELFEFFECT        0x019b
#define SMSG_BEING_EMOTION           0x00c0
#define SMSG_BEING_ACTION            0x008a /**< Attack, sit, stand up, ... */
#define SMSG_BEING_CHAT              0x008d /**< A being talks */
#define SMSG_BEING_NAME_RESPONSE     0x0095 /**< Has to be requested */
#define SMSG_BEING_CHANGE_DIRECTION  0x009c

#define SMSG_PLAYER_STATUS_CHANGE    0x0119
#define SMSG_BEING_STATUS_CHANGE     0x0196

#define SMSG_NPC_MESSAGE             0x00b4
#define SMSG_NPC_NEXT                0x00b5
#define SMSG_NPC_CLOSE               0x00b6
#define SMSG_NPC_CHOICE              0x00b7 /**< Display a choice */
#define SMSG_NPC_BUY_SELL_CHOICE     0x00c4
#define SMSG_NPC_BUY                 0x00c6
#define SMSG_NPC_SELL                0x00c7
#define SMSG_NPC_BUY_RESPONSE        0x00ca
#define SMSG_NPC_SELL_RESPONSE       0x00cb
#define SMSG_NPC_INT_INPUT           0x0142 /**< Integer input */
#define SMSG_NPC_STR_INPUT           0x01d4 /**< String input */
#define SMSG_PLAYER_CHAT             0x008e /**< Player talks */
#define SMSG_WHISPER                 0x0097 /**< Whisper Recieved */
#define SMSG_WHISPER_RESPONSE        0x0098
#define SMSG_GM_CHAT                 0x009a /**< GM announce */
#define SMSG_WALK_RESPONSE           0x0087

#define SMSG_TRADE_REQUEST           0x00e5 /**< Receiving a request to trade */
#define SMSG_TRADE_RESPONSE          0x00e7
#define SMSG_TRADE_ITEM_ADD          0x00e9
#define SMSG_TRADE_ITEM_ADD_RESPONSE 0x01b1 /**< Not standard eAthena! */
#define SMSG_TRADE_OK                0x00ec
#define SMSG_TRADE_CANCEL            0x00ee
#define SMSG_TRADE_COMPLETE          0x00f0

#define SMSG_PARTY_CREATE            0x00fa
#define SMSG_PARTY_INFO              0x00fb
#define SMSG_PARTY_INVITE_RESPONSE   0x00fd
#define SMSG_PARTY_INVITED           0x00fe
#define SMSG_PARTY_SETTINGS          0x0101
#define SMSG_PARTY_MOVE              0x0104
#define SMSG_PARTY_LEAVE             0x0105
#define SMSG_PARTY_UPDATE_HP         0x0106
#define SMSG_PARTY_UPDATE_COORDS     0x0107
#define SMSG_PARTY_MESSAGE           0x0109

#define SMSG_PLAYER_STORAGE_ITEMS    0x01f0 /**< Item list for storage */
#define SMSG_PLAYER_STORAGE_EQUIP    0x00a6 /**< Equipment list for storage */
#define SMSG_PLAYER_STORAGE_STATUS   0x00f2 /**< Slots used and total slots */
#define SMSG_PLAYER_STORAGE_ADD      0x00f4 /**< Add item/equip to storage */
#define SMSG_PLAYER_STORAGE_REMOVE   0x00f6 /**< Remove item/equip from storage */
#define SMSG_PLAYER_STORAGE_CLOSE    0x00f8 /**< Storage access closed */

#define SMSG_ADMIN_KICK_ACK          0x00cd

#define SMSG_MVP                     0x010c

/**********************************
 *  Packets from client to server *
 **********************************/
#define CMSG_CHAR_PASSWORD_CHANGE    0x0061 /**< Custom change password packet */
#define CMSG_CHAR_SERVER_CONNECT     0x0065
#define CMSG_CHAR_SELECT             0x0066
#define CMSG_CHAR_CREATE             0x0067
#define CMSG_CHAR_DELETE             0x0068

#define CMSG_MAP_SERVER_CONNECT      0x0072
#define CMSG_CLIENT_PING             0x007e /**< Send to server with tick */
#define CMSG_MAP_LOADED              0x007d
#define CMSG_CLIENT_QUIT             0x018A

#define CMSG_CHAT_MESSAGE            0x008c
#define CMSG_CHAT_WHISPER            0x0096
#define CMSG_CHAT_ANNOUNCE           0x0099
#define CMSG_CHAT_WHO                0x00c1

#define CMSG_SKILL_LEVELUP_REQUEST   0x0112
#define CMSG_STAT_UPDATE_REQUEST     0x00bb

#define CMSG_PLAYER_INVENTORY_USE    0x00a7
#define CMSG_PLAYER_INVENTORY_DROP   0x00a2
#define CMSG_PLAYER_EQUIP            0x00a9
#define CMSG_PLAYER_UNEQUIP          0x00ab

#define CMSG_ITEM_PICKUP             0x009f
#define CMSG_PLAYER_CHANGE_DIR       0x009b
#define CMSG_PLAYER_CHANGE_DEST      0x0085
#define CMSG_PLAYER_CHANGE_ACT       0x0089
#define CMSG_PLAYER_RESPAWN          0x00b2
#define CMSG_PLAYER_EMOTE            0x00bf
#define CMSG_PLAYER_ATTACK           0x0089
#define CMSG_WHO_REQUEST             0x00c1

#define CMSG_NPC_TALK                0x0090
#define CMSG_NPC_NEXT_REQUEST        0x00b9
#define CMSG_NPC_CLOSE               0x0146
#define CMSG_NPC_LIST_CHOICE         0x00b8
#define CMSG_NPC_INT_RESPONSE        0x0143
#define CMSG_NPC_STR_RESPONSE        0x01d5
#define CMSG_NPC_BUY_SELL_REQUEST    0x00c5
#define CMSG_NPC_BUY_REQUEST         0x00c8
#define CMSG_NPC_SELL_REQUEST        0x00c9

#define CMSG_TRADE_REQUEST           0x00e4
#define CMSG_TRADE_RESPONSE          0x00e6
#define CMSG_TRADE_ITEM_ADD_REQUEST  0x00e8
#define CMSG_TRADE_CANCEL_REQUEST    0x00ed
#define CMSG_TRADE_ADD_COMPLETE      0x00eb
#define CMSG_TRADE_OK                0x00ef

#define CMSG_PARTY_CREATE            0x00f9
#define CMSG_PARTY_INVITE            0x00fc
#define CMSG_PARTY_INVITED           0x00ff
#define CMSG_PARTY_LEAVE             0x0100
#define CMSG_PARTY_SETTINGS          0x0102
#define CMSG_PARTY_KICK              0x0103
#define CMSG_PARTY_MESSAGE           0x0108

#define CMSG_MOVE_TO_STORAGE         0x00f3 /** Move item to storage */
#define CSMG_MOVE_FROM_STORAGE       0x00f5 /** Remove item from storage */
#define CMSG_CLOSE_STORAGE           0x00f7 /** Request storage close */

#define CMSG_ADMIN_ANNOUNCE          0x0099
#define CMSG_ADMIN_LOCAL_ANNOUNCE    0x019C
#define CMSG_ADMIN_HIDE              0x019D
#define CMSG_ADMIN_KICK              0x00CC
#define CMSG_ADMIN_MUTE              0x0149

#endif
