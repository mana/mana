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

#include "configuration.h"
#include "log.h"
#include "imagesprite.h"

#include "resources/resourcemanager.h"
#include "resources/image.h"
#include "resources/imageset.h"

namespace
{
    Emotes mEmotes;
    Emote mUnknown;
    bool mLoaded = false;
    int mLastEmote = 0;
}

void EmoteDB::init()
{
    if (mLoaded)
        unload();

    mUnknown.name = "unknown";
    mUnknown.effect = -1;
    mUnknown.sprite = new ImageSprite(
        ResourceManager::getInstance()->getImage("graphics/sprites/error.png"));

    mLastEmote = 0;

}

void EmoteDB::readEmoteNode(xmlNodePtr node, const std::string &filename)
{
    int id = XML::getProperty(node, "id", -1);
    if (id == -1)
    {
        logger->log("Emote Database: Emote with missing ID in %s!", filename.c_str());
        return;
    }

    auto *currentEmote = new Emote;

    currentEmote->name = XML::getProperty(node, "name", "unknown");
    currentEmote->effect = XML::getProperty(node, "effectid", -1);

    if (currentEmote->effect == -1)
    {
        logger->log("Emote Database: Warning: Emote %s has no attached effect in %s!",
                    currentEmote->name.c_str(), filename.c_str());
        delete currentEmote;
        return;
    }

    const std::string imageName = XML::getProperty(node, "image", "");
    const int width = XML::getProperty(node, "width", 0);
    const int height = XML::getProperty(node, "height", 0);

    if (imageName.empty() || width <= 0 || height <= 0)
    {
        logger->log("Emote Database: Warning: Emote %s has bad imageset values in %s",
                    currentEmote->name.c_str(), filename.c_str());
        delete currentEmote;
        return;
    }

    ImageSet *is = ResourceManager::getInstance()->getImageSet(imageName,
                                                               width,
                                                               height);
    if (!is || !(is->size() > 0))
    {
        logger->log("Emote Database: Error loading imageset for emote %s in %s",
                    currentEmote->name.c_str(), filename.c_str());
        delete is;
        delete currentEmote;
        return;
    }
    else
    {
        // For now we just use the first image in the animation
        currentEmote->sprite = new ImageSprite(is->get(0));
    }

    mEmotes[id] = currentEmote;
    if (id > mLastEmote)
        mLastEmote = id;
}

void EmoteDB::checkStatus()
{
    mLoaded = true;
}

void EmoteDB::unload()
{
    Emotes::iterator i;
    for (i = mEmotes.begin(); i != mEmotes.end(); i++)
    {
        delete i->second;
    }

    mEmotes.clear();

    mLoaded = false;
}

const Emote *EmoteDB::get(int id)
{
    Emotes::const_iterator i = mEmotes.find(id);

    if (i == mEmotes.end())
    {
        logger->log("EmoteDB: Warning, unknown emote ID %d requested", id);
        return &mUnknown;
    }
    else
    {
        return i->second;
    }
}

int EmoteDB::getLast()
{
    return mLastEmote;
}
