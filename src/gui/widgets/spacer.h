/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef SPACER_H
#define SPACER_H

#include "guichan/graphics.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"

/**
 * A space.
 *
 * \ingroup GUI
 */
class Spacer : public gcn::Widget
{
    public:
        /**
         * Constructor.
         *
         * @note Can be called empty, will default to a 5x5 px space
         *
         * @param w - width in px.
         * @param h - height in px.
         */
        Spacer(int x = 5, int y = 5);

        /**
         * Draws the Space.
         */
        void draw(gcn::Graphics *g){}
};

#endif // SPACER_H
