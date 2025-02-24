/*
 *  Emote database
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2009-2013  The Mana Developers
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

#include "resources/emotedb.h"

#include "log.h"

#include "resources/imageset.h"
#include "resources/resourcemanager.h"

#include <algorithm>
#include <vector>

namespace
{
    std::vector<Emote> mEmotes;
    Emote mUnknown;
    bool mLoaded = false;
}

void EmoteDB::init()
{
    if (mLoaded)
        unload();

    mUnknown.name = "unknown";
    mUnknown.effectId = -1;
    mUnknown.image = ResourceManager::getInstance()->getImage("graphics/sprites/error.png");
}

void EmoteDB::readEmoteNode(XML::Node node, const std::string &filename)
{
    const int id = node.getProperty("id", -1);
    if (id == -1)
    {
        logger->log("Emote Database: Emote with missing ID in %s!", filename.c_str());
        return;
    }

    Emote emote;

    emote.id = id;
    emote.name = node.getProperty("name", "unknown");
    emote.effectId = node.getProperty("effectid", -1);

    if (emote.effectId == -1)
    {
        logger->log("Emote Database: Warning: Emote %s has no attached effect in %s!",
                    emote.name.c_str(), filename.c_str());
        return;
    }

    const std::string imageName = node.getProperty("image", "");
    const int width = node.getProperty("width", 0);
    const int height = node.getProperty("height", 0);

    if (imageName.empty() || width <= 0 || height <= 0)
    {
        logger->log("Emote Database: Warning: Emote %s has bad imageset values in %s",
                    emote.name.c_str(), filename.c_str());
        return;
    }

    emote.is = ResourceManager::getInstance()->getImageSet(imageName,
                                                           width,
                                                           height);

    if (!emote.is || emote.is->size() == 0)
    {
        logger->log("Emote Database: Error loading imageset for emote %s in %s",
                    emote.name.c_str(), filename.c_str());
        return;
    }

    // For now we just use the first image in the animation
    emote.image = emote.is->get(0);

    mEmotes.push_back(std::move(emote));
}

void EmoteDB::checkStatus()
{
    mLoaded = true;
}

void EmoteDB::unload()
{
    // These images are owned by the ImageSet
    for (auto &emote : mEmotes)
        emote.image.release();

    mEmotes.clear();
    mUnknown.image = nullptr;
    mLoaded = false;
}

const Emote &EmoteDB::get(int id)
{
    auto i = std::find_if(mEmotes.begin(), mEmotes.end(),
                          [id](const Emote &e) { return e.id == id; });

    if (i == mEmotes.end())
    {
        logger->log("EmoteDB: Warning, unknown emote ID %d requested", id);
        return mUnknown;
    }

    return *i;
}

const Emote &EmoteDB::getByIndex(int index)
{
    return mEmotes.at(index);
}

int EmoteDB::getEmoteCount()
{
    return mEmotes.size();
}
