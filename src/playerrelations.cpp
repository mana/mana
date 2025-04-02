/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2024  The Mana Developers
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

#include <algorithm>

#include "actorspritemanager.h"
#include "being.h"
#include "configuration.h"
#include "playerrelations.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

PlayerRelationsManager::~PlayerRelationsManager()
{
    delete_all(mIgnoreStrategies);
}

int PlayerRelationsManager::getPlayerIgnoreStrategyIndex(const std::string &name)
{
    std::vector<PlayerIgnoreStrategy *> &strategies = getPlayerIgnoreStrategies();
    for (unsigned int i = 0; i < strategies.size(); i++)
        if (strategies[i]->mShortName == name)
            return i;

    return -1;
}

void PlayerRelationsManager::init()
{
    int index = getPlayerIgnoreStrategyIndex(config.playerIgnoreStrategy);
    if (index >= 0)
        setPlayerIgnoreStrategy(getPlayerIgnoreStrategies()[index]);

    // Ignores are always saved to the config file, but might not be loaded
    if (config.persistentPlayerList)
        mRelations = config.players;

    signalUpdate();
}

void PlayerRelationsManager::store()
{
    config.playerIgnoreStrategy = mIgnoreStrategy ? mIgnoreStrategy->mShortName
                                                  : DEFAULT_IGNORE_STRATEGY;
    config.players = mRelations;
}

void PlayerRelationsManager::signalUpdate()
{
    store();

    for (auto listener : mListeners)
        listener->playerRelationsUpdated();
}

unsigned int PlayerRelationsManager::checkPermissionSilently(
                                                  const std::string &playerName,
                                                  unsigned int flags)
{
    unsigned int permissions = config.defaultPlayerPermissions;

    switch (getRelation(playerName))
    {
    case PlayerRelation::Neutral:
        break;

        // widen permissions for friends
    case PlayerRelation::Friend:
        permissions |=
                PlayerPermissions::EMOTE |
                PlayerPermissions::SPEECH_FLOAT |
                PlayerPermissions::SPEECH_LOG |
                PlayerPermissions::WHISPER |
                PlayerPermissions::TRADE;
        break;

        // narrow permissions for disregarded and ignored players
    case PlayerRelation::Disregarded:
        permissions &=
                PlayerPermissions::EMOTE |
                PlayerPermissions::SPEECH_FLOAT;
        break;
    case PlayerRelation::Ignored:
        permissions &= 0;
        break;
    }

    return permissions & flags;
}

bool PlayerRelationsManager::hasPermission(Being *being, unsigned int flags)
{
    if (being->getType() == ActorSprite::PLAYER)
        return hasPermission(being->getName(), flags) == flags;
    return true;
}

bool PlayerRelationsManager::hasPermission(const std::string &name,
                                           unsigned int flags)
{
    unsigned int rejections = flags & ~checkPermissionSilently(name, flags);
    bool permitted = rejections == 0;

    // execute `ignore' strategy, if possible
    if (!permitted && mIgnoreStrategy)
    {
        if (Being *b = actorSpriteManager->findBeingByName(name, ActorSprite::PLAYER))
            mIgnoreStrategy->ignore(b, rejections);
    }

    return permitted;
}

void PlayerRelationsManager::setRelation(const std::string &playerName,
                                         PlayerRelation relation)
{
    mRelations[playerName] = relation;
    signalUpdate();
}

std::vector<std::string> PlayerRelationsManager::getPlayers() const
{
    std::vector<std::string> retval;

    for (const auto &[name, _] : mRelations)
        retval.push_back(name);

    sort(retval.begin(), retval.end());

    return retval;
}

void PlayerRelationsManager::removePlayer(const std::string &name)
{
    auto it = mRelations.find(name);
    if (it != mRelations.end())
    {
        mRelations.erase(it);
        signalUpdate();
    }
}


PlayerRelation PlayerRelationsManager::getRelation(const std::string &name) const
{
    auto it = mRelations.find(name);
    return it != mRelations.end() ? it->second : PlayerRelation::Neutral;
}

////////////////////////////////////////
// defaults

unsigned int PlayerRelationsManager::getDefault() const
{
    return config.defaultPlayerPermissions;
}

void PlayerRelationsManager::setDefault(unsigned int permissions)
{
    config.defaultPlayerPermissions = permissions;
    signalUpdate();
}


////////////////////////////////////////
// ignore strategies


class PIS_nothing : public PlayerIgnoreStrategy
{
public:
    PIS_nothing()
    {
        mDescription = _("Completely ignore");
        mShortName = "nop";
    }

    void ignore(Being *being, unsigned int flags) override
    {
    }
};

class PIS_dotdotdot : public PlayerIgnoreStrategy
{
public:
    PIS_dotdotdot()
    {
        mDescription = _("Print '...'");
        mShortName = "dotdotdot";
    }

    void ignore(Being *being, unsigned int flags) override
    {
        being->setSpeech("...", 2000);
    }
};


class PIS_blinkname : public PlayerIgnoreStrategy
{
public:
    PIS_blinkname()
    {
        mDescription = _("Blink name");
        mShortName = "blinkname";
    }

    void ignore(Being *being, unsigned int flags) override
    {
        being->flashName(1500);
    }
};

std::vector<PlayerIgnoreStrategy *> &
PlayerRelationsManager::getPlayerIgnoreStrategies()
{
    if (mIgnoreStrategies.empty())
    {
        mIgnoreStrategies.push_back(new PIS_nothing());
        mIgnoreStrategies.push_back(new PIS_dotdotdot());
        mIgnoreStrategies.push_back(new PIS_blinkname());
    }
    return mIgnoreStrategies;
}


PlayerRelationsManager player_relations;
