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
    std::map<int, BeingInfo *> mMonsterInfos;
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

    // This can be overridden by an 'offset' attribute on a 'monsters' root tag.
    mMonsterIdOffset = Net::getNetworkType() == ServerType::TMWATHENA ? OLD_TMWATHENA_OFFSET : 0;
}

void MonsterDB::setMonsterIdOffset(int offset)
{
    mMonsterIdOffset = offset;
}

/**
 * Read <monster> node from settings.
 */
void MonsterDB::readMonsterNode(xmlNodePtr node, const std::string &filename)
{
    auto *currentInfo = new BeingInfo;

    currentInfo->blockType = Map::BLOCKTYPE_MONSTER;

    currentInfo->name = XML::getProperty(node, "name", _("unnamed"));

    currentInfo->setTargetCursorSize(XML::getProperty(node,
                                     "targetCursor", "medium"));

    currentInfo->setHoverCursor(XML::getProperty(node, "hoverCursor", "attack"));

    currentInfo->targetSelection = XML::getProperty(
        node, "targetSelection", true);

    SpriteDisplay &display = currentInfo->display;

    for (auto spriteNode : XML::Children(node))
    {
        if (xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
        {
            SpriteReference &currentSprite = display.sprites.emplace_back();
            currentSprite.sprite = (const char*)spriteNode->children->content;
            currentSprite.variant = XML::getProperty(spriteNode, "variant", 0);
        }
        else if (xmlStrEqual(spriteNode->name, BAD_CAST "sound"))
        {
            std::string event = XML::getProperty(spriteNode, "event", std::string());
            const char *soundFile = (const char*) spriteNode->children->content;

            if (event == "hit")
            {
                currentInfo->addSound(SoundEvent::HIT, soundFile);
            }
            else if (event == "miss")
            {
                currentInfo->addSound(SoundEvent::MISS, soundFile);
            }
            else if (event == "hurt")
            {
                currentInfo->addSound(SoundEvent::HURT, soundFile);
            }
            else if (event == "die")
            {
                currentInfo->addSound(SoundEvent::DIE, soundFile);
            }
            else
            {
                logger->log("MonsterDB: Warning, sound effect %s for "
                            "unknown event %s of monster %s in %s",
                            soundFile, event.c_str(),
                            currentInfo->name.c_str(),
                            filename.c_str());
            }
        }
        else if (xmlStrEqual(spriteNode->name, BAD_CAST "attack"))
        {
            Attack attack;
            const int id = XML::getProperty(spriteNode, "id", 0);

            attack.effectId = XML::getProperty(spriteNode, "effect-id", -1);
            attack.hitEffectId =
                XML::getProperty(spriteNode, "hit-effect-id",
                                 paths.getIntValue("hitEffectId"));
            attack.criticalHitEffectId =
                XML::getProperty(spriteNode, "critical-hit-effect-id",
                                 paths.getIntValue("criticalHitEffectId"));
            attack.missileParticleFilename =
                XML::getProperty(spriteNode, "missile-particle", "");

            attack.action = XML::getProperty(spriteNode, "action", "attack");

            currentInfo->addAttack(id, std::move(attack));
        }
        else if (xmlStrEqual(spriteNode->name, BAD_CAST "particlefx"))
        {
            display.particles.emplace_back(
                (const char*) spriteNode->children->content);
        }
    }

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
    auto i = mMonsterInfos.find(id);

    if (i == mMonsterInfos.end())
    {
        logger->log("MonsterDB: Warning, unknown monster ID %d requested", id);
        return BeingInfo::Unknown;
    }

    return i->second;
}
