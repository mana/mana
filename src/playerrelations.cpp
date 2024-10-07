/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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
    ConfigurationObject *writeConfigItem(std::pair<std::string, PlayerRelation> value,
                                         ConfigurationObject *cobj) override
    {
        cobj->setValue(NAME, value.first);
        cobj->setValue(RELATION, toString(value.second.mRelation));

        return cobj;
    }

    std::map<std::string, PlayerRelation> *
    readConfigItem(ConfigurationObject *cobj,
                   std::map<std::string, PlayerRelation> *container) override
    {
        std::string name = cobj->getValue(NAME, "");
        if (name.empty())
            return container;

        auto it = (*container).find(name);
        if (it != (*container).end())
        {
            int v = (int)cobj->getValue(RELATION, PlayerRelation::NEUTRAL);
            (*container)[name] = PlayerRelation(static_cast<PlayerRelation::Relation>(v));
        }
        // otherwise ignore the duplicate entry

        return container;
    }
};

static PlayerConfSerialiser player_conf_serialiser; // stateless singleton

const unsigned int PlayerRelation::RELATION_PERMISSIONS[RELATIONS_NR] = {
    /* NEUTRAL */    0, // we always fall back to the defaults anyway
    /* FRIEND  */    EMOTE | SPEECH_FLOAT | SPEECH_LOG | WHISPER | TRADE,
    /* DISREGARDED*/ EMOTE | SPEECH_FLOAT,
    /* IGNORED */    0
};

PlayerRelation::PlayerRelation(Relation relation)
{
    mRelation = relation;
}

PlayerRelationsManager::PlayerRelationsManager() :
    mPersistIgnores(false),
    mDefaultPermissions(PlayerRelation::DEFAULT),
    mIgnoreStrategy(nullptr)
{
}

PlayerRelationsManager::~PlayerRelationsManager()
{
    delete_all(mIgnoreStrategies);
}

void PlayerRelationsManager::clear()
{
    for (const auto &name : getPlayers())
        removePlayer(name);
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

void PlayerRelationsManager::load()
{
    clear();

    mPersistIgnores = config.getValue(PERSIST_IGNORE_LIST, 1);
    mDefaultPermissions = (int) config.getValue(DEFAULT_PERMISSIONS, mDefaultPermissions);
    std::string ignore_strategy_name = config.getValue(PLAYER_IGNORE_STRATEGY, DEFAULT_IGNORE_STRATEGY);
    int ignore_strategy_index = getPlayerIgnoreStrategyIndex(ignore_strategy_name);
    if (ignore_strategy_index >= 0)
        setPlayerIgnoreStrategy(getPlayerIgnoreStrategies()[ignore_strategy_index]);

    config.getList<std::pair<std::string, PlayerRelation>,
                   std::map<std::string, PlayerRelation> *>
        ("player",  &mRelations, &player_conf_serialiser);
}


void PlayerRelationsManager::init()
{
    load();

    if (!mPersistIgnores)
        clear(); // Yes, we still keep them around in the config file until the next update.
}

void PlayerRelationsManager::store()
{
    config.setList<std::map<std::string, PlayerRelation>::const_iterator,
                   std::pair<std::string, PlayerRelation>,
                   std::map<std::string, PlayerRelation> *>
        ("player",
         mRelations.begin(), mRelations.end(),
         &player_conf_serialiser);

    config.setValue(DEFAULT_PERMISSIONS, mDefaultPermissions);
    config.setValue(PERSIST_IGNORE_LIST, mPersistIgnores);
    config.setValue(PLAYER_IGNORE_STRATEGY,
                    (mIgnoreStrategy)? mIgnoreStrategy->mShortName : DEFAULT_IGNORE_STRATEGY);

    config.write();
}

void PlayerRelationsManager::signalUpdate(const std::string &name)
{
    store();

    for (auto listener : mListeners)
        listener->updatedPlayer(name);
}

unsigned int PlayerRelationsManager::checkPermissionSilently(
                                                  const std::string &playerName,
                                                  unsigned int flags)
{
    auto it = mRelations.find(playerName);
    if (it == mRelations.end())
        return mDefaultPermissions & flags;

    PlayerRelation &r = it->second;

    unsigned int permissions =
        PlayerRelation::RELATION_PERMISSIONS[r.mRelation];

    switch (r.mRelation)
    {
    case PlayerRelation::NEUTRAL:
        permissions = mDefaultPermissions;
        break;

    case PlayerRelation::FRIEND:
        permissions |= mDefaultPermissions; // widen
        break;

    default:
        permissions &= mDefaultPermissions; // narrow
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

    if (!permitted)
    {
        // execute `ignore' strategy, if possible
        if (mIgnoreStrategy)
        {
            Being *b = actorSpriteManager->findBeingByName(name,
                                                        ActorSprite::PLAYER);
            if (b && b->getType() == ActorSprite::PLAYER)
                mIgnoreStrategy->ignore(b, rejections);
        }
    }

    return permitted;
}

void PlayerRelationsManager::setRelation(const std::string &playerName,
                                         PlayerRelation::Relation relation)
{
    mRelations[playerName] = PlayerRelation(relation);
    signalUpdate(playerName);
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
        signalUpdate(name);
    }
}


PlayerRelation::Relation PlayerRelationsManager::getRelation(const std::string &name) const
{
    auto it = mRelations.find(name);
    if (it != mRelations.end())
        return it->second.mRelation;

    return PlayerRelation::NEUTRAL;
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

    store();
    signalUpdate(std::string());
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
