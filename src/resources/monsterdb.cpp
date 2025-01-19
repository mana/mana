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
void MonsterDB::readMonsterNode(XML::Node node, const std::string &filename)
{
    auto *currentInfo = new BeingInfo;

    currentInfo->blockType = Map::BLOCKTYPE_MONSTER;

    currentInfo->name = node.getProperty("name", _("unnamed"));

    currentInfo->setTargetCursorSize(node.getProperty("targetCursor", "medium"));
    currentInfo->setHoverCursor(node.getProperty("hoverCursor", "attack"));

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
        else if (spriteNode.name() == "sound")
        {
            std::string event = spriteNode.getProperty("event", std::string());
            const std::string soundFile { spriteNode.textContent() };

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
                            soundFile.c_str(), event.c_str(),
                            currentInfo->name.c_str(),
                            filename.c_str());
            }
        }
        else if (spriteNode.name() == "attack")
        {
            Attack attack;
            const int id = spriteNode.getProperty("id", 0);

            attack.effectId = spriteNode.getProperty("effect-id", -1);
            attack.hitEffectId =
                spriteNode.getProperty("hit-effect-id",
                                 paths.getIntValue("hitEffectId"));
            attack.criticalHitEffectId =
                spriteNode.getProperty("critical-hit-effect-id",
                                 paths.getIntValue("criticalHitEffectId"));
            attack.missileParticleFilename =
                spriteNode.getProperty("missile-particle", "");

            attack.action = spriteNode.getProperty("action", "attack");

            currentInfo->addAttack(id, std::move(attack));
        }
        else if (spriteNode.name() == "particlefx")
        {
            display.particles.emplace_back(spriteNode.textContent());
        }
    }

    mMonsterInfos[node.getProperty("id", 0) + mMonsterIdOffset] = currentInfo;
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
