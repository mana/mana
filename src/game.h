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

#ifndef _TMW_GAME_
#define _TMW_GAME_

#include <iosfwd>

#define SPEECH_TIME 80
#define SPEECH_MAX_TIME 100

class MessageHandler;
class Network;

extern std::string map_path;
extern volatile int fps;
extern volatile int tick_time;

class Game
{
    public:
        Game(Network *network);
        ~Game();

        void logic();

        void handleInput();

    protected:
        Network *mNetwork;

        MessageHandler *mBeingHandler;
        MessageHandler *mBuySellHandler;
        MessageHandler *mChatHandler;
        MessageHandler *mEquipmentHandler;
        MessageHandler *mInventoryHandler;
        MessageHandler *mItemHandler;
        MessageHandler *mNpcHandler;
        MessageHandler *mPlayerHandler;
        MessageHandler *mSkillHandler;
        MessageHandler *mTradeHandler;
};

/**
 * Returns elapsed time. (Warning: very unsafe function, it supposes the delay
 * is always < 10 seconds)
 */
int get_elapsed_time(int start_time);

#endif
