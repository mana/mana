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

#include <algorithm>

#include "being.h"
#include "beingmanager.h"
#include "configuration.h"
#include "graphics.h"
#include "player.h"
#include "playerrelations.h"

#include "utils/dtor.h"

#define PLAYER_IGNORE_STRATEGY_NOP "nop"
#define PLAYER_IGNORE_STRATEGY_EMOTE0 "emote0"
#define DEFAULT_IGNORE_STRATEGY PLAYER_IGNORE_STRATEGY_EMOTE0

#define NAME "name" // constant for xml serialisation
#define RELATION "relation" // constant for xml serialisation

#define IGNORE_EMOTE_TIME 100

// (De)serialisation class
class PlayerConfSerialiser : public ConfigurationListManager<std::pair<std::string, PlayerRelation *>,
                                                             std::map<std::string, PlayerRelation *> *>
{
    virtual ConfigurationObject *writeConfigItem(std::pair<std::string, PlayerRelation *> value,
                                                 ConfigurationObject *cobj)
    {
        if (!value.second)
            return NULL;
        cobj->setValue(NAME, value.first);
        cobj->setValue(RELATION, toString(value.second->mRelation));

        return cobj;
    }

    virtual std::map<std::string, PlayerRelation *> *
    readConfigItem(ConfigurationObject *cobj,
                   std::map<std::string, PlayerRelation *> *container)
    {
        std::string name = cobj->getValue(NAME, "");
        if (name.empty())
            return container;

        if (!(*container)[name]) {
            int v = (int)cobj->getValue(RELATION, PlayerRelation::NEUTRAL);
            (*container)[name] = new PlayerRelation(static_cast<PlayerRelation::Relation>(v));
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
    mIgnoreStrategy(NULL)
{
}

PlayerRelationsManager::~PlayerRelationsManager()
{
    delete_all(mIgnoreStrategies);
}

void PlayerRelationsManager::clear()
{
    std::vector<std::string> *names = getPlayers();
    for (std::vector<std::string>::const_iterator
             it = names->begin(); it != names->end(); it++)
        removePlayer(*it);
    delete names;
}

#define PERSIST_IGNORE_LIST "persistent-player-list"
#define PLAYER_IGNORE_STRATEGY "player-ignore-strategy"
#define DEFAULT_PERMISSIONS "default-player-permissions"

int PlayerRelationsManager::getPlayerIgnoreStrategyIndex(const std::string &name)
{
    std::vector<PlayerIgnoreStrategy *> *strategies = getPlayerIgnoreStrategies();
    for (unsigned int i = 0; i < strategies->size(); i++)
        if ((*strategies)[i]->mShortName == name)
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
        setPlayerIgnoreStrategy((*getPlayerIgnoreStrategies())[ignore_strategy_index]);

    config.getList<std::pair<std::string, PlayerRelation *>,
                   std::map<std::string, PlayerRelation *> *>
        ("player",  &(mRelations), &player_conf_serialiser);
}


void PlayerRelationsManager::init()
{
    load();

    if (!mPersistIgnores)
        clear(); // Yes, we still keep them around in the config file until the next update.
}

void PlayerRelationsManager::store()
{
    config.setList<std::map<std::string, PlayerRelation *>::const_iterator,
                   std::pair<std::string, PlayerRelation *>,
                   std::map<std::string, PlayerRelation *> *>
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

    for (std::list<PlayerRelationsListener *>::const_iterator it = mListeners.begin(); it != mListeners.end(); it++)
        (*it)->updatedPlayer(name);
}

unsigned int PlayerRelationsManager::checkPermissionSilently(const std::string &player_name, unsigned int flags)
{
    PlayerRelation *r = mRelations[player_name];
    if (!r)
        return mDefaultPermissions & flags;
    else {
        unsigned int permissions = PlayerRelation::RELATION_PERMISSIONS[r->mRelation];

        switch (r->mRelation) {
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
}

bool PlayerRelationsManager::hasPermission(Being *being, unsigned int flags)
{
    if (being->getType() == Being::PLAYER)
        return hasPermission(being->getName(), flags) == flags;
    return true;
}

bool PlayerRelationsManager::hasPermission(const std::string &name,
                                           unsigned int flags)
{
    unsigned int rejections = flags & ~checkPermissionSilently(name, flags);
    bool permitted = rejections == 0;

    if (!permitted) {
        // execute `ignore' strategy, if possible
        if (mIgnoreStrategy) {
            Player *to_ignore = dynamic_cast<Player *>(beingManager->findBeingByName(name, Being::PLAYER));

            if (to_ignore)
                mIgnoreStrategy->ignore(to_ignore, rejections);
        }
    }

    return permitted;
}

void PlayerRelationsManager::setRelation(const std::string &player_name,
                                         PlayerRelation::Relation relation)
{
    PlayerRelation *r = mRelations[player_name];
    if (r == NULL)
        mRelations[player_name] = new PlayerRelation(relation);
    else
        r->mRelation = relation;

    signalUpdate(player_name);
}

std::vector<std::string> * PlayerRelationsManager::getPlayers()
{
    std::vector<std::string> *retval = new std::vector<std::string>();

    for (std::map<std::string, PlayerRelation *>::const_iterator it = mRelations.begin(); it != mRelations.end(); it++)
        if (it->second)
            retval->push_back(it->first);

    sort(retval->begin(), retval->end());

    return retval;
}

void PlayerRelationsManager::removePlayer(const std::string &name)
{
    if (mRelations[name])
        delete mRelations[name];

    mRelations.erase(name);

    signalUpdate(name);
}


PlayerRelation::Relation PlayerRelationsManager::getRelation(const std::string &name)
{
    if (mRelations[name])
        return mRelations[name]->mRelation;

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
    signalUpdate("");
}


////////////////////////////////////////
// ignore strategies


class PIS_nothing : public PlayerIgnoreStrategy
{
public:
    PIS_nothing()
    {
        mDescription = "completely ignore";
        mShortName = PLAYER_IGNORE_STRATEGY_NOP;
    }

    virtual void ignore(Player *player, unsigned int flags)
    {
    }
};

class PIS_dotdotdot : public PlayerIgnoreStrategy
{
public:
    PIS_dotdotdot()
    {
        mDescription = "print '...'";
        mShortName = "dotdotdot";
    }

    virtual void ignore(Player *player, unsigned int flags)
     {
         player->setSpeech("...", 500);
     }
};


class PIS_blinkname : public PlayerIgnoreStrategy
{
public:
    PIS_blinkname()
    {
        mDescription = "blink name";
        mShortName = "blinkname";
    }

    virtual void ignore(Player *player, unsigned int flags)
    {
        player->flash(200);
    }
};

class PIS_emote : public PlayerIgnoreStrategy
{
public:
    PIS_emote(int emote_nr, const std::string &description, const std::string &shortname) :
        mEmotion(emote_nr)
    {
        mDescription = description;
        mShortName = shortname;
    }

    virtual void ignore(Player *player, unsigned int flags)
     {
         player->setEmote(mEmotion, IGNORE_EMOTE_TIME);
     }
private:
    int mEmotion;
};



std::vector<PlayerIgnoreStrategy *> *
PlayerRelationsManager::getPlayerIgnoreStrategies()
{
    if (mIgnoreStrategies.size() == 0)
    {
        // not initialised yet?
        mIgnoreStrategies.push_back(new PIS_emote(FIRST_IGNORE_EMOTE,
                                                  "floating '...' bubble",
                                                   PLAYER_IGNORE_STRATEGY_EMOTE0));
        mIgnoreStrategies.push_back(new PIS_emote(FIRST_IGNORE_EMOTE + 1,
                                                  "floating bubble",
                                                  "emote1"));
        mIgnoreStrategies.push_back(new PIS_nothing());
        mIgnoreStrategies.push_back(new PIS_dotdotdot());
        mIgnoreStrategies.push_back(new PIS_blinkname());
    }
    return &mIgnoreStrategies;
}


PlayerRelationsManager player_relations;
