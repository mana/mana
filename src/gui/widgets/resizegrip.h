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

#include <guichan/widget.hpp>

/**
 * Resize grip. The resize grip is part of a resizable Window. It relies on the
 * fact that uncaught mouse events are automatically routed to the parent
 * window.
 *
 * \ingroup GUI
 */
class ResizeGrip : public gcn::Widget
{
    public:
        ResizeGrip();

        /**
         * Draws the resize grip.
         */
        void draw(gcn::Graphics *graphics) override;
};
