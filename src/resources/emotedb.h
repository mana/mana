/*
 *  Emote database
 *  Copyright (C) 2009  Aethyra Development Team
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

#ifndef EMOTE_DB_H
#define EMOTE_DB_H

#include <list>
#include <map>
#include <string>

class ImageSprite;

struct Emote
{
    std::string name;
    ImageSprite *sprite;
    int effect;
};

typedef std::map<int, Emote*> Emotes;

/**
 * Emote information database.
 */
namespace EmoteDB
{
    void load();

    void unload();

    const Emote *get(int id);

    int getLast();

    typedef Emotes::iterator EmotesIterator;
}

#endif // EMOTE_DB_H
