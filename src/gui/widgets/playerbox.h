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

#pragma once

#include "scrollarea.h"

class Being;

/**
 * A box showing a player character.
 *
 * \ingroup GUI
 */
class PlayerBox : public ScrollArea
{
    public:
        /**
         * Constructor. Takes the initial player character that this box should
         * display, which defaults to <code>NULL</code>.
         */
        PlayerBox(const Being *being = nullptr);

        /**
         * Sets a new player character to be displayed by this box. Setting the
         * player to <code>NULL</code> causes the box not to draw any
         * character.
         */
        void setPlayer(const Being *being)
        { mBeing = being; }

        /**
         * Draws the scroll area and the player.
         */
        void draw(gcn::Graphics *graphics) override;

    private:
        const Being *mBeing; /**< The character used for display */
};
