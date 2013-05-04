/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "resources/monsterdb.h"

#include "log.h"

#include "net/net.h"

#include "resources/beinginfo.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

#include "configuration.h"

#define OLD_TMWATHENA_OFFSET 1002


namespace
{
    BeingInfos mMonsterInfos;
    bool mLoaded = false;
    int mMonsterIdOffset;
}


/**
 * Initialize MonsterDB
 *
 * If it was initialized before, unload() will be called first.
 */
void MonsterDB::init()
{
    if (mLoaded)
        unload();

    // This used to be read from offset attribute of monsters root tag, however
    // I couldn't find any place it was used, so for now the default values are set
    mMonsterIdOffset = Net::getNetworkType() == ServerInfo::TMWATHENA ? OLD_TMWATHENA_OFFSET : 0;
}

/**
 * Read <monster> node from settings.
 */
void MonsterDB::readMonsterNode(xmlNodePtr node, const std::string &filename)
{
    BeingInfo *currentInfo = new BeingInfo;

    currentInfo->setWalkMask(Map::BLOCKMASK_WALL
                             | Map::BLOCKMASK_CHARACTER
                             | Map::BLOCKMASK_MONSTER);
    currentInfo->setBlockType(Map::BLOCKTYPE_MONSTER);

    currentInfo->setName(XML::getProperty(node, "name", _("unnamed")));

    currentInfo->setTargetCursorSize(XML::getProperty(node,
                                     "targetCursor", "medium"));

    SpriteDisplay display;

    //iterate <sprite>s and <sound>s
    for_each_xml_child_node(spriteNode, node)
    {
        if (xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
        {
            SpriteReference currentSprite;
            currentSprite.sprite = (const char*)spriteNode->xmlChildrenNode->content;
            currentSprite.variant = XML::getProperty(spriteNode, "variant", 0);
            display.sprites.push_back(currentSprite);
        }
        else if (xmlStrEqual(spriteNode->name, BAD_CAST "sound"))
        {
            std::string event = XML::getProperty(spriteNode, "event", "");
            const char *soundFile;
            soundFile = (const char*) spriteNode->xmlChildrenNode->content;

            if (event == "hit")
            {
                currentInfo->addSound(SOUND_EVENT_HIT, soundFile);
            }
            else if (event == "miss")
            {
                currentInfo->addSound(SOUND_EVENT_MISS, soundFile);
            }
            else if (event == "hurt")
            {
                currentInfo->addSound(SOUND_EVENT_HURT, soundFile);
            }
            else if (event == "die")
            {
                currentInfo->addSound(SOUND_EVENT_DIE, soundFile);
            }
            else
            {
                logger->log("MonsterDB: Warning, sound effect %s for "
                            "unknown event %s of monster %s in %s",
                            soundFile, event.c_str(),
                            currentInfo->getName().c_str(),
                            filename.c_str());
            }
        }
        else if (xmlStrEqual(spriteNode->name, BAD_CAST "attack"))
        {
            const int id = XML::getProperty(spriteNode, "id", 0);
            int effectId = XML::getProperty(spriteNode, "effect-id", -1);
            int hitEffectId =
                XML::getProperty(spriteNode, "hit-effect-id",
                                 paths.getIntValue("hitEffectId"));
            int criticalHitEffectId =
                XML::getProperty(spriteNode, "critical-hit-effect-id",
                                 paths.getIntValue("criticalHitEffectId"));
            const std::string missileParticleFilename =
                XML::getProperty(spriteNode, "missile-particle", "");

            const std::string spriteAction = XML::getProperty(spriteNode,
                                                              "action",
                                                              "attack");

            currentInfo->addAttack(id, spriteAction, effectId,
                                   hitEffectId, criticalHitEffectId,
                                   missileParticleFilename);
        }
        else if (xmlStrEqual(spriteNode->name, BAD_CAST "particlefx"))
        {
            display.particles.push_back(
                (const char*) spriteNode->xmlChildrenNode->content);
        }
    }
    currentInfo->setDisplay(display);

    mMonsterInfos[XML::getProperty(node, "id", 0) + mMonsterIdOffset] = currentInfo;


}

/**
 * Check if everything was loaded correctly
 */
void MonsterDB::checkStatus()
{
    // there is nothing to check for now
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
