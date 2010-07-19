/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <map>
#include <string>

/**
 * Standard attributes for players.
 */
enum Attribute
{
    LEVEL,
    HP, MAX_HP,
    MP, MAX_MP,
    EXP, EXP_NEEDED,
    MONEY,
    TOTAL_WEIGHT, MAX_WEIGHT,
    SKILL_POINTS,
    CHAR_POINTS, CORR_POINTS
};

/**
 * Stat information storage structure.
 */
struct Stat
{
    int base;
    int mod;
    int exp;
    int expNeed;
};

typedef std::map<int, int> IntMap;
typedef std::map<int, Stat> StatMap;

/**
 * Backend for core player information.
 */
struct PlayerInfoBackend
{
    IntMap mAttributes;
    StatMap mStats;
};

class Equipment;
class Inventory;
class Item;

/**
 * Special information storage structure.
 */
struct Special
{
    int currentMana;
    int neededMana;
    int recharge;
};

typedef std::map<int, Special> SpecialsMap;

/**
 * A database like namespace which holds global info about the localplayer
 *
 * NOTE: 'bool notify' is used to determine if a event is to be triggered.
 */
namespace PlayerInfo
{

// --- Attributes -------------------------------------------------------------

    /**
     * Returns the value of the given attribute.
     */
    int getAttribute(int id);

    /**
     * Changes the value of the given attribute.
     */
    void setAttribute(int id, int value, bool notify = true);

// --- Stats ------------------------------------------------------------------

    /**
     * Returns the base value of the given stat.
     */
    int getStatBase(int id);

    /**
     * Changes the base value of the given stat.
     */
    void setStatBase(int id, int value, bool notify = true);

    /**
     * Returns the modifier for the given stat.
     */
    int getStatMod(int id);

    /**
     * Changes the modifier for the given stat.
     */
    void setStatMod(int id, int value, bool notify = true);

    /**
     * Returns the current effective value of the given stat. Effective is base
     * + mod
     */
    int getStatEffective(int id);

    /**
     * Changes the level of the given stat.
     */
    void setStatLevel(int id, int value, bool notify = true);

    /**
     * Returns the experience of the given stat.
     */
    std::pair<int, int> getStatExperience(int id);

    /**
     * Changes the experience of the given stat.
     */
    void setStatExperience(int id, int have, int need, bool notify = true);

// --- Inventory / Equipment --------------------------------------------------

    /**
     * Returns the player's inventory.
     */
    Inventory *getInventory();

    /**
     * Clears the player's inventory and equipment.
     */
    void clearInventory();

    /**
     * Changes the inventory item at the given slot.
     */
    void setInventoryItem(int index, int id, int amount);

    /**
     * Returns the player's equipment.
     */
    Equipment *getEquipment();

    /**
     * Returns the player's equipment at the given slot.
     */
    Item *getEquipment(unsigned int slot);

// --- Specials ---------------------------------------------------------------

    /**
     * Changes the status of the given special.
     */
    void setSpecialStatus(int id, int current, int max, int recharge);

    /**
     * Returns the status of the given special.
     */
    const SpecialsMap &getSpecialStatus();

// --- Misc -------------------------------------------------------------------

    /**
     * Changes the internal PlayerInfoBackend reference;
     */
    void setBackend(const PlayerInfoBackend &backend);

    /**
     * Does necessary updates every tick.
     */
    void logic();

    /**
     * Returns true if the player is involved in a trade at the moment, false
     * otherwise.
     */
    bool isTrading();

    /**
     * Sets whether the player is currently involved in trade or not.
     */
    void setTrading(bool trading);

    /**
     * Initializes some internals.
     */
    void init();

} // namespace PlayerInfo

#endif
