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

#ifndef PLAYER_RELATIONS_H
#define PLAYER_RELATIONS_H

#include <list>
#include <map>
#include <string>
#include <vector>

class Being;
class Player;

struct PlayerRelation
{
    static const unsigned int EMOTE        = (1 << 0);
    static const unsigned int SPEECH_FLOAT = (1 << 1);
    static const unsigned int SPEECH_LOG   = (1 << 2);
    static const unsigned int WHISPER      = (1 << 3);
    static const unsigned int TRADE        = (1 << 4);

    static const unsigned int RELATIONS_NR = 4;
    static const unsigned int RELATION_PERMISSIONS[RELATIONS_NR];

    static const unsigned int DEFAULT = EMOTE
                                      | SPEECH_FLOAT
                                      | SPEECH_LOG
                                      | WHISPER
                                      | TRADE;
    enum Relation {
        NEUTRAL     = 0,
        FRIEND      = 1,
        DISREGARDED = 2,
        IGNORED     = 3
    };

    PlayerRelation(Relation relation);

    Relation mRelation; // bitmask for all of the above
};


/**
 * Ignore strategy: describes how we should handle ignores.
 */
class PlayerIgnoreStrategy
{
public:
    std::string mDescription;
    std::string mShortName;

    virtual ~PlayerIgnoreStrategy() {}

    /**
     * Handle the ignoring of the indicated action by the indicated player.
     */
    virtual void ignore(Player *player, unsigned int flags) = 0;
};

class PlayerRelationsListener
{
public:
    PlayerRelationsListener() { }
    virtual ~PlayerRelationsListener() { }

    virtual void updatedPlayer(const std::string &name) = 0;
};

/**
 * Player relations class, represents any particular relations and/or
 * preferences the user of the local client has wrt other players (identified
 * by std::string).
 */
class PlayerRelationsManager
{
public:
    PlayerRelationsManager();
    ~PlayerRelationsManager();

    /**
     * Initialise player relations manager (load config file etc.)
     */
    void init();

    /**
     * Load configuration from our config file, or substitute defaults.
     */
    void load();

    /**
     * Save configuration to our config file.
     */
    void store();

    /**
     * Determines whether the player in question is being ignored, filtered by
     * the specified flags.
     */
    unsigned int checkPermissionSilently(const std::string &player_name,
                                         unsigned int flags);

    /**
     * Tests whether the player in question is being ignored for any of the
     * actions in the specified flags. If so, trigger appropriate side effects
     * if requested by the player.
     */
    bool hasPermission(Being *being, unsigned int flags);

    bool hasPermission(const std::string &being, unsigned int flags);

    /**
     * Updates the relationship with this player.
     */
    void setRelation(const std::string &name,
                     PlayerRelation::Relation relation);

    /**
     * Updates the relationship with this player.
     */
    PlayerRelation::Relation getRelation(const std::string &name);

    /**
     * Deletes the information recorded for a player.
     */
    void removePlayer(const std::string &name);

    /**
     * Retrieves the default permissions.
     */
    unsigned int getDefault() const;

    /**
     * Sets the default permissions.
     */
    void setDefault(unsigned int permissions);

    /**
     * Retrieves all known player ignore strategies.
     *
     * The player ignore strategies are allocated statically and must not be
     * deleted.
     */
    std::vector<PlayerIgnoreStrategy *> *getPlayerIgnoreStrategies();

    /**
     * Return the current player ignore strategy.
     *
     * \return A player ignore strategy, or NULL
     */
    PlayerIgnoreStrategy *getPlayerIgnoreStrategy() const
    {
        return mIgnoreStrategy;
    }

    /**
     * Sets the strategy to call when ignoring players.
     */
    void setPlayerIgnoreStrategy(PlayerIgnoreStrategy *strategy)
    {
        mIgnoreStrategy = strategy;
    }

    /**
     * For a given ignore strategy short name, find the appropriate index in
     * the ignore strategies vector.
     *
     * \param The short name of the ignore strategy to look up
     * \return The appropriate index, or -1
     */
    int getPlayerIgnoreStrategyIndex(const std::string &shortname);

    /**
     * Retrieves a sorted vector of all players for which we have any relations
     * recorded.
     */
    std::vector<std::string> *getPlayers();

    /**
     * Removes all recorded player info.
     */
    void clear();

    /**
     * Do we persist our `ignore' setup?
     */
    bool getPersistIgnores() const { return mPersistIgnores; }

    /**
     * Change the `ignore persist' flag.
     *
     * @param value Whether to persist ignores
     */
    void setPersistIgnores(bool value) { mPersistIgnores = value; }

    void addListener(PlayerRelationsListener *listener)
    {
        mListeners.push_back(listener);
    }

    void removeListener(PlayerRelationsListener *listener)
    {
        mListeners.remove(listener);
    }

private:
    void signalUpdate(const std::string &name);

    bool mPersistIgnores; // If NOT set, we delete the ignored data upon reloading
    unsigned int mDefaultPermissions;

    PlayerIgnoreStrategy *mIgnoreStrategy;
    std::map<std::string, PlayerRelation *> mRelations;
    std::list<PlayerRelationsListener *> mListeners;
    std::vector<PlayerIgnoreStrategy *> mIgnoreStrategies;
};


extern PlayerRelationsManager player_relations; // singleton representation of player relations


#endif /* !defined(PLAYER_RELATIONS_H) */
