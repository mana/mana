/*
 *  Emote database
 *  Copyright (C) 2009  Aethyra Development Team
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

#ifndef EMOTE_DB_H
#define EMOTE_DB_H

#include <list>
#include <map>
#include <string>

class AnimatedSprite;

struct EmoteSprite
{
    const AnimatedSprite *sprite;
    std::string name;
};

struct EmoteInfo
{
    std::list<EmoteSprite*> sprites;
    std::list<std::string> particles;
};

typedef std::map<int, EmoteInfo*> EmoteInfos;

/**
 * Emote information database.
 */
namespace EmoteDB
{
    void load();

    void unload();

    const EmoteInfo *get(int id);

    const AnimatedSprite *getAnimation(int id);

    const int &getLast();

    typedef EmoteInfos::iterator EmoteInfosIterator;
}

#endif
