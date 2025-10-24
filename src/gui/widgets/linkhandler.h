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

#include <string>

/**
 * A simple interface to windows that need to handle links from BrowserBox
 * widget.
 */
class LinkHandler
{
    public:
        virtual ~LinkHandler() = default;

        /**
         * Called when a link has an empty caption. Returns the caption
         * to use for the link.
         *
         * Default implementation just returns the link itself.
         */
        virtual std::string captionForLink(const std::string &link)
        {
            return link;
        }

        /**
         * Called when a link is clicked.
         */
        virtual void handleLink(const std::string &link) = 0;
};
