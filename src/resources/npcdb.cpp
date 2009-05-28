/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "resources/npcdb.h"

#include "log.h"

#include "utils/xml.h"

namespace
{
    NPCInfos mNPCInfos;
    NPCInfo mUnknown;
    bool mLoaded = false;
}

void NPCDB::load()
{
    if (mLoaded)
        return;

    NPCsprite *unknownSprite = new NPCsprite;
    unknownSprite->sprite = "error.xml";
    unknownSprite->variant = 0;
    mUnknown.sprites.push_back(unknownSprite);

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

        NPCInfo *currentInfo = new NPCInfo;

        for_each_xml_child_node(spriteNode, npcNode)
        {
            if (xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
            {
                NPCsprite *currentSprite = new NPCsprite;
                currentSprite->sprite = (const char*)spriteNode->xmlChildrenNode->content;
                currentSprite->variant = XML::getProperty(spriteNode, "variant", 0);
                currentInfo->sprites.push_back(currentSprite);
            }
            else if (xmlStrEqual(spriteNode->name, BAD_CAST "particlefx"))
            {
                std::string particlefx = (const char*)spriteNode->xmlChildrenNode->content;
                currentInfo->particles.push_back(particlefx);
            }
        }
        mNPCInfos[id] = currentInfo;
    }

    mLoaded = true;
}

void NPCDB::unload()
{
    for (   NPCInfosIterator i = mNPCInfos.begin();
            i != mNPCInfos.end();
            i++)
    {
        while (!i->second->sprites.empty())
        {
            delete i->second->sprites.front();
            i->second->sprites.pop_front();
        }
        delete i->second;
    }

    mNPCInfos.clear();

    while (!mUnknown.sprites.empty())
    {
        delete mUnknown.sprites.front();
        mUnknown.sprites.pop_front();
    }

    mLoaded = false;
}

const NPCInfo& NPCDB::get(int id)
{
    NPCInfosIterator i = mNPCInfos.find(id);

    if (i == mNPCInfos.end())
    {
        logger->log("NPCDB: Warning, unknown NPC ID %d requested", id);
        return mUnknown;
    }
    else
    {
        return *(i->second);
    }
}
