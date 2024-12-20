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

#define PLAYER_IGNORE_STRATEGY_NOP "nop"
#define DEFAULT_IGNORE_STRATEGY PLAYER_IGNORE_STRATEGY_NOP

#define NAME "name" // constant for xml serialisation
#define RELATION "relation" // constant for xml serialisation

// (De)serialisation class
class PlayerConfSerialiser : public ConfigurationListManager<std::pair<std::string, PlayerRelation>,
                                                             std::map<std::string, PlayerRelation> *>
{
    ConfigurationObject *writeConfigItem(const std::pair<std::string, PlayerRelation> &value,
                                         ConfigurationObject *cobj) override
    {
        cobj->setValue(NAME, value.first);
        cobj->setValue(RELATION, toString(static_cast<int>(value.second)));

        return cobj;
    }

    std::map<std::string, PlayerRelation> *
    readConfigItem(ConfigurationObject *cobj,
                   std::map<std::string, PlayerRelation> *container) override
    {
        std::string name = cobj->getValue(NAME, std::string());
        if (name.empty())
            return container;

        int v = cobj->getValue(RELATION, static_cast<int>(PlayerRelation::NEUTRAL));
        (*container)[name] = static_cast<PlayerRelation>(v);

        return container;
    }
};



PlayerRelationsManager::~PlayerRelationsManager()
{
    delete_all(mIgnoreStrategies);
}

void PlayerRelationsManager::clear()
{
    mRelations.clear();
    signalUpdate();
}

#define PERSIST_IGNORE_LIST "persistent-player-list"
#define PLAYER_IGNORE_STRATEGY "player-ignore-strategy"
#define DEFAULT_PERMISSIONS "default-player-permissions"

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
    mPersistIgnores = config.getValue(PERSIST_IGNORE_LIST, 1);
    mDefaultPermissions = config.getValue(DEFAULT_PERMISSIONS, mDefaultPermissions);
    std::string ignore_strategy_name = config.getValue(PLAYER_IGNORE_STRATEGY, DEFAULT_IGNORE_STRATEGY);
    int ignore_strategy_index = getPlayerIgnoreStrategyIndex(ignore_strategy_name);
    if (ignore_strategy_index >= 0)
        setPlayerIgnoreStrategy(getPlayerIgnoreStrategies()[ignore_strategy_index]);

    mRelations.clear();

    // Ignores are always saved to the config file, but might not be loaded
    if (mPersistIgnores)
    {
        PlayerConfSerialiser player_conf_serialiser;
        config.getList<std::pair<std::string, PlayerRelation>,
                       std::map<std::string, PlayerRelation> *>
                ("player", &mRelations, player_conf_serialiser);
    }

    signalUpdate();
}

void PlayerRelationsManager::store()
{
    PlayerConfSerialiser player_conf_serialiser;
    config.setList<std::map<std::string, PlayerRelation>::const_iterator,
                   std::pair<std::string, PlayerRelation>,
                   std::map<std::string, PlayerRelation> *>
        ("player",
         mRelations.begin(), mRelations.end(),
         player_conf_serialiser);

    config.setValue(DEFAULT_PERMISSIONS, mDefaultPermissions);
    config.setValue(PERSIST_IGNORE_LIST, mPersistIgnores);
    config.setValue(PLAYER_IGNORE_STRATEGY,
                    (mIgnoreStrategy)? mIgnoreStrategy->mShortName : DEFAULT_IGNORE_STRATEGY);

    config.write();
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
    unsigned int permissions = mDefaultPermissions;

    switch (getRelation(playerName))
    {
    case PlayerRelation::NEUTRAL:
        break;

        // widen permissions for friends
    case PlayerRelation::FRIEND:
        permissions |=
                PlayerPermissions::EMOTE |
                PlayerPermissions::SPEECH_FLOAT |
                PlayerPermissions::SPEECH_LOG |
                PlayerPermissions::WHISPER |
                PlayerPermissions::TRADE;
        break;

        // narrow permissions for disregarded and ignored players
    case PlayerRelation::DISREGARDED:
        permissions &=
                PlayerPermissions::EMOTE |
                PlayerPermissions::SPEECH_FLOAT;
        break;
    case PlayerRelation::IGNORED:
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
    return it != mRelations.end() ? it->second : PlayerRelation::NEUTRAL;
}

////////////////////////////////////////
// defaults

unsigned int PlayerRelationsManager::getDefault() const
{
    return mDefaultPermissions;
}

void PlayerRelationsManager::setDefault(unsigned int permissions)
{
    mDefaultPermissions = permissions;
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
        mShortName = PLAYER_IGNORE_STRATEGY_NOP;
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
