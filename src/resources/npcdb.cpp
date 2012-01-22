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

#include "resources/npcdb.h"

#include "log.h"

#include "resources/beinginfo.h"

#include "utils/dtor.h"
#include "utils/xml.h"
#include "configuration.h"

namespace
{
    BeingInfos mNPCInfos;
    bool mLoaded = false;
}

void NPCDB::load()
{
    if (mLoaded)
        unload();

    logger->log("Initializing NPC database...");

    XML::Document doc("npcs.xml");
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "npcs"))
    {
        logger->error("NPC Database: Error while loading npcs.xml!");
    }

    //iterate <npc>s
    for_each_xml_child_node(npcNode, rootNode)
    {
        if (!xmlStrEqual(npcNode->name, BAD_CAST "npc"))
            continue;

        int id = XML::getProperty(npcNode, "id", 0);
        if (id == 0)
        {
            logger->log("NPC Database: NPC with missing ID in npcs.xml!");
            continue;
        }

        BeingInfo *currentInfo = new BeingInfo;

        currentInfo->setTargetCursorSize(XML::getProperty(npcNode,
                                         "targetCursor", "medium"));

        SpriteDisplay display;
        for_each_xml_child_node(spriteNode, npcNode)
        {
            if (xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
            {
                SpriteReference currentSprite;
                currentSprite.sprite = (const char*)spriteNode->xmlChildrenNode->content;
                currentSprite.variant = XML::getProperty(spriteNode, "variant", 0);
                display.sprites.push_back(currentSprite);
            }
            else if (xmlStrEqual(spriteNode->name, BAD_CAST "particlefx"))
            {
                std::string particlefx = (const char*)spriteNode->xmlChildrenNode->content;
                display.particles.push_back(particlefx);
            }
        }

        currentInfo->setDisplay(display);

        mNPCInfos[id] = currentInfo;
    }

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
    BeingInfoIterator i = mNPCInfos.find(id);

    if (i == mNPCInfos.end())
    {
        logger->log("NPCDB: Warning, unknown NPC ID %d requested", id);
        return BeingInfo::Unknown;
    }
    else
    {
        return i->second;
    }
}
