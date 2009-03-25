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

#ifndef NET_GAMESERVER_PLAYER_H
#define NET_GAMESERVER_PLAYER_H

#include "../../../being.h"

#include <guichan/actionlistener.hpp>

#include <iosfwd>


struct RespawnRequestListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event);
};

namespace Net
{
    namespace GameServer
    {
        namespace Player
        {
            void say(const std::string &text);
            void walk(int x, int y);
            void pickUp(int x, int y);
            void moveItem(int oldSlot, int newSlot, int amount);
            void drop(int slot, int amount);
            void equip(int slot);
            void unequip(int slot);
            void useItem(int slot);
            void attack(int direction);
            void useSpecial(int special);
            void changeAction(Being::Action action);
            void talkToNPC(int id, bool restart);
            void selectFromNPC(int id, int choice);
            void requestTrade(int id);
            void acceptTrade(bool accept);
            void tradeItem(int slot, int amount);
            void tradeMoney(int amount);
            void tradeWithNPC(int item, int amount);
            void sendLetter(const std::string &player, const std::string &text);
            void raiseAttribute(int attribute);
            void lowerAttribute(int attribute);
            void respawn();
            static RespawnRequestListener respawnListener;
            void changeDir(unsigned char dir);
        }
    }
}

#endif
