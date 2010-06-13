/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "resources/monsterdb.h"

#include "log.h"

#include "net/net.h"

#include "resources/beinginfo.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/xml.h"

#define OLD_TMWATHENA_OFFSET 1002

namespace
{
    BeingInfos mMonsterInfos;
    bool mLoaded = false;
}

void MonsterDB::load()
{
    if (mLoaded)
        unload();

    logger->log("Initializing monster database...");

    XML::Document doc("monsters.xml");
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "monsters"))
    {
        logger->error("Monster Database: Error while loading monster.xml!");
    }

    int offset = XML::getProperty(rootNode, "offset", Net::getNetworkType() ==
                                 ServerInfo::TMWATHENA ? OLD_TMWATHENA_OFFSET : 0);

    //iterate <monster>s
    for_each_xml_child_node(monsterNode, rootNode)
    {
        if (!xmlStrEqual(monsterNode->name, BAD_CAST "monster"))
        {
            continue;
        }

        BeingInfo *currentInfo = new BeingInfo;

        currentInfo->setWalkMask(Map::BLOCKMASK_WALL
                                 | Map::BLOCKMASK_CHARACTER
                                 | Map::BLOCKMASK_MONSTER);
        currentInfo->setBlockType(Map::BLOCKTYPE_MONSTER);

        currentInfo->setName(XML::getProperty(monsterNode, "name", _("unnamed")));

        currentInfo->setTargetCursorSize(XML::getProperty(monsterNode,
                                         "targetCursor", "medium"));

        SpriteDisplay display;

        //iterate <sprite>s and <sound>s
        for_each_xml_child_node(spriteNode, monsterNode)
        {
            if (xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
            {
                SpriteReference *currentSprite = new SpriteReference;
                currentSprite->sprite = (const char*)spriteNode->xmlChildrenNode->content;
                currentSprite->variant = XML::getProperty(spriteNode, "variant", 0);
                display.sprites.push_back(currentSprite);
            }
            else if (xmlStrEqual(spriteNode->name, BAD_CAST "sound"))
            {
                std::string event = XML::getProperty(spriteNode, "event", "");
                const char *filename;
                filename = (const char*) spriteNode->xmlChildrenNode->content;

                if (event == "hit")
                {
                    currentInfo->addSound(SOUND_EVENT_HIT, filename);
                }
                else if (event == "miss")
                {
                    currentInfo->addSound(SOUND_EVENT_MISS, filename);
                }
                else if (event == "hurt")
                {
                    currentInfo->addSound(SOUND_EVENT_HURT, filename);
                }
                else if (event == "die")
                {
                    currentInfo->addSound(SOUND_EVENT_DIE, filename);
                }
                else
                {
                    logger->log("MonsterDB: Warning, sound effect %s for "
                                "unknown event %s of monster %s",
                                filename, event.c_str(),
                                currentInfo->getName().c_str());
                }
            }
            else if (xmlStrEqual(spriteNode->name, BAD_CAST "attack"))
            {
                const int id = XML::getProperty(spriteNode, "id", 0);
                const std::string particleEffect = XML::getProperty(
                        spriteNode, "particle-effect", "");
                SpriteAction spriteAction = SpriteDef::makeSpriteAction(
                        XML::getProperty(spriteNode, "action", "attack"));
                const std::string missileParticle = XML::getProperty(
                        spriteNode, "missile-particle", "");
                currentInfo->addAttack(id, spriteAction,
                                       particleEffect, missileParticle);
            }
            else if (xmlStrEqual(spriteNode->name, BAD_CAST "particlefx"))
            {
                display.particles.push_back(
                    (const char*) spriteNode->xmlChildrenNode->content);
            }
        }
        currentInfo->setDisplay(display);

        mMonsterInfos[XML::getProperty(monsterNode, "id", 0) + offset] = currentInfo;
    }

    mLoaded = true;
}

void MonsterDB::unload()
{
    delete_all(mMonsterInfos);
    mMonsterInfos.clear();

    mLoaded = false;
}


BeingInfo *MonsterDB::get(int id)
{
    BeingInfoIterator i = mMonsterInfos.find(id);

    if (i == mMonsterInfos.end())
    {
        logger->log("MonsterDB: Warning, unknown monster ID %d requested", id);
        return BeingInfo::Unknown;
    }
    else
    {
        return i->second;
    }
}
