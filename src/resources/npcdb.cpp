/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "resources/npcdb.h"

#include "log.h"

#include "resources/beinginfo.h"

#include "utils/dtor.h"
#include "utils/xml.h"

namespace
{
    std::map<int, BeingInfo *> mNPCInfos;
    bool mLoaded = false;
}


void NPCDB::init()
{
    if (mLoaded)
        unload();
}

void NPCDB::readNPCNode(XML::Node node, const std::string &filename)
{
    int id = node.getProperty("id", 0);
    if (id == 0)
    {
        Log::info("NPC Database: NPC with missing ID in %s", filename.c_str());
        return;
    }

    auto *currentInfo = new BeingInfo;

    currentInfo->setTargetCursorSize(node.getProperty("targetCursor", "medium"));
    currentInfo->setHoverCursor(node.getProperty("hoverCursor", "talk"));

    currentInfo->targetSelection = node.getProperty("targetSelection", true);

    SpriteDisplay &display = currentInfo->display;
    for (auto spriteNode : node.children())
    {
        if (spriteNode.name() == "sprite")
        {
            SpriteReference &currentSprite = display.sprites.emplace_back();
            currentSprite.sprite = spriteNode.textContent();
            currentSprite.variant = spriteNode.getProperty("variant", 0);
        }
        else if (spriteNode.name() == "particlefx")
        {
            display.particles.emplace_back(spriteNode.textContent());
        }
    }

    mNPCInfos[id] = currentInfo;
}

void NPCDB::checkStatus()
{
    mLoaded = true;
}

void NPCDB::unload()
{
    delete_all(mNPCInfos);
    mNPCInfos.clear();

    mLoaded = false;
}

BeingInfo *NPCDB::get(int id)
{
    auto i = mNPCInfos.find(id);

    if (i == mNPCInfos.end())
    {
        Log::info("NPCDB: Warning, unknown NPC ID %d requested", id);
        return BeingInfo::Unknown;
    }

    return i->second;
}
