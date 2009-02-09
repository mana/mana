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

#ifndef GAME_
#define GAME_

#include <memory>

#include "configlistener.h"

class MessageHandler;
class Network;

extern std::string map_path;
extern volatile int fps;
extern volatile int tick_time;

class Game : public ConfigListener
{
    public:
	Game(Network *network);
	~Game();

	void logic();

	void handleInput();

	void optionChanged(const std::string &name);

    private:
	Network *mNetwork;

	/** Used to determine whether to draw the next frame. */
	int mDrawTime;

	/** The minimum frame time (used for frame limiting). */
	int mMinFrameTime;

	typedef const std::auto_ptr<MessageHandler> MessageHandlerPtr;
	MessageHandlerPtr mBeingHandler;
	MessageHandlerPtr mBuySellHandler;
	MessageHandlerPtr mChatHandler;
	MessageHandlerPtr mEquipmentHandler;
	MessageHandlerPtr mInventoryHandler;
	MessageHandlerPtr mItemHandler;
	MessageHandlerPtr mNpcHandler;
	MessageHandlerPtr mPlayerHandler;
	MessageHandlerPtr mSkillHandler;
	MessageHandlerPtr mTradeHandler;
};

/**
 * Returns elapsed time. (Warning: supposes the delay is always < 100 seconds)
 */
int get_elapsed_time(int start_time);

#endif
