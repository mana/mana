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
#define SMSG_BEING_REMOVE            0x0080
#define SMSG_BEING_CHANGE_LOOKS      0x00c3
#define SMSG_BEING_CHANGE_LOOKS2     0x01d7 /**< Same as 0x00c3, but 16 bit ID */
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

// Packets from client to server
#define CMSG_TRADE_RESPONSE          0x00e6
#define CMSG_ITEM_PICKUP             0x009f
#define CMSG_MAP_LOADED              0x007d
#define CMSG_NPC_BUY_REQUEST         0x00c8
#define CMSG_NPC_BUY_SELL_REQUEST    0x00c5
#define CMSG_CHAT_MESSAGE            0x008c
#define CMSG_CHAT_WHISPER            0x0096
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

/** Encodes coords and direction in 3 bytes data */
void set_coordinates(char *data, unsigned short x, unsigned short y, unsigned char direction);

#endif
