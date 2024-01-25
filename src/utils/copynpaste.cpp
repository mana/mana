/*
 *  Clipboard Interaction.
 *  Copyright (C) 2010-2024  The Mana Developers
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

#include "utils/copynpaste.h"

#include <SDL.h>

bool insertFromClipboard(std::string &text, std::string::size_type &pos)
{
    char *buf = SDL_GetClipboardText();
    const size_t len = strlen(buf);
    if (len > 0) {
        text.insert(pos, buf);
        pos += len;
    }
    SDL_free(buf);
    return len > 0;
}
