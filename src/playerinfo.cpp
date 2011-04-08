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

#include "playerinfo.h"

#include "client.h"
#include "equipment.h"
#include "event.h"
#include "inventory.h"
#include "eventlistener.h"
#include "log.h"

#include "resources/iteminfo.h"

namespace PlayerInfo {

class PlayerLogic;

PlayerLogic *mListener = 0;

PlayerInfoBackend mData;

Inventory *mInventory = 0;
Equipment *mEquipment = 0;

bool mStorageCount = 0;

bool mNPCCount = 0;
bool mNPCPostCount = 0;

BuySellState mBuySellState = BUYSELL_NONE;
bool mTrading = false;

std::map<int, Special> mSpecials;
char mSpecialRechargeUpdateNeeded = 0;

int mLevelProgress = 0;

// --- Triggers ---------------------------------------------------------------

void triggerAttr(int id, int old)
{
    Event event(Event::UpdateAttribute);
    event.setInt("id", id);
    event.setInt("oldValue", old);
    event.setInt("newValue", mData.mAttributes.find(id)->second);
    event.trigger(Event::AttributesChannel);
}

void triggerStat(int id, const std::string &changed, int old1, int old2 = 0)
{
    StatMap::iterator it = mData.mStats.find(id);
    Event event(Event::UpdateStat);
    event.setInt("id", id);
    event.setInt("base", it->second.base);
    event.setInt("mod", it->second.mod);
    event.setInt("exp", it->second.exp);
    event.setInt("expNeeded", it->second.expNeed);
    event.setString("changed", changed);
    event.setInt("oldValue1", old1);
    event.setInt("oldValue2", old2);
    event.trigger(Event::AttributesChannel);
}

// --- Attributes -------------------------------------------------------------

int getAttribute(int id)
{
    IntMap::const_iterator it = mData.mAttributes.find(id);
    if (it != mData.mAttributes.end())
        return it->second;
    else
        return 0;
}

void setAttribute(int id, int value, bool notify)
{
    int old = mData.mAttributes[id];
    mData.mAttributes[id] = value;
    if (notify)
        triggerAttr(id, old);
}

// --- Stats ------------------------------------------------------------------

int getStatBase(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.base;
    else
        return 0;
}

void setStatBase(int id, int value, bool notify)
{
    int old = mData.mStats[id].base;
    mData.mStats[id].base = value;
    if (notify)
        triggerStat(id, "base", old);
}

int getStatMod(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.mod;
    else
        return 0;
}

void setStatMod(int id, int value, bool notify)
{
    int old = mData.mStats[id].mod;
    mData.mStats[id].mod = value;
    if (notify)
        triggerStat(id, "mod", old);
}

int getStatEffective(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.base + it->second.mod;
    else
        return 0;
}

std::pair<int, int> getStatExperience(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    int a, b;
    if (it != mData.mStats.end())
    {
        a = it->second.exp;
        b = it->second.expNeed;
    }
    else
    {
        a = 0;
        b = 0;
    }
    return std::pair<int, int>(a, b);
}

void setStatExperience(int id, int have, int need, bool notify)
{
    int oldExp = mData.mStats[id].exp;
    int oldExpNeed = mData.mStats[id].expNeed;
    mData.mStats[id].exp = have;
    mData.mStats[id].expNeed = need;
    if (notify)
        triggerStat(id, "exp", oldExp, oldExpNeed);
}

// --- Inventory / Equipment --------------------------------------------------

Inventory *getInventory()
{
    return mInventory;
}

void clearInventory()
{
    mEquipment->clear();
    mInventory->clear();
}

void setInventoryItem(int index, int id, int amount)
{
    mInventory->setItem(index, id, amount);
}

Equipment *getEquipment()
{
    return mEquipment;
}

Item *getEquipment(unsigned int slot)
{
    return mEquipment->getEquipment(slot);
}

void setEquipmentBackend(Equipment::Backend *backend)
{
    mEquipment->setBackend(backend);
}

int getStorageCount()
{
    return mStorageCount;
}

void setStorageCount(int count)
{
    int old = mStorageCount;
    mStorageCount = count;

    if (count != old)
    {
        Event event(Event::StorageCount);
        event.setInt("oldCount", old);
        event.setInt("newCount", count);
        event.trigger(Event::StorageChannel);
    }
}

// -- NPC ---------------------------------------------------------------------

int getNPCInteractionCount()
{
    return mNPCCount;
}

void setNPCInteractionCount(int count)
{
    int old = mNPCCount;
    mNPCCount = count;

    if (count != old)
    {
        Event event(Event::NpcCount);
        event.setInt("oldCount", old);
        event.setInt("newCount", count);
        event.trigger(Event::NpcChannel);
    }
}

int getNPCPostCount()
{
    return mNPCPostCount;
}

void setNPCPostCount(int count)
{
    int old = mNPCPostCount;
    mNPCPostCount = count;

    if (count != old)
    {
        Event event(Event::PostCount);
        event.setInt("oldCount", old);
        event.setInt("newCount", count);
        event.trigger(Event::NpcChannel);
    }
}

// -- Buy/Sell/Trade ----------------------------------------------------------

BuySellState getBuySellState()
{
    return mBuySellState;
}

void setBuySellState(BuySellState buySellState)
{
    BuySellState old = mBuySellState;
    mBuySellState = buySellState;

    if (buySellState != old)
    {
        Event event(Event::StateChange);
        event.setInt("oldState", old);
        event.setInt("newState", buySellState);
        event.trigger(Event::BuySellChannel);
    }
}

bool isTrading()
{
    return mTrading;
}

void setTrading(bool trading)
{
    bool notify = mTrading != trading;
    mTrading = trading;

    if (notify)
    {
        Event event(Event::Trading);
        event.setBool("trading", trading);
        event.trigger(Event::StatusChannel);
    }
}

// --- Specials ---------------------------------------------------------------

void setSpecialStatus(int id, int current, int max, int recharge)
{
    logger->log("SpecialUpdate Skill #%d -- (%d/%d) -> %d", id, current, max,
                recharge);
    mSpecials[id].currentMana = current;
    mSpecials[id].neededMana = max;
    mSpecials[id].recharge = recharge;
}

const SpecialsMap &getSpecialStatus()
{
    return mSpecials;
}

// --- Misc -------------------------------------------------------------------

void setBackend(const PlayerInfoBackend &backend)
{
    mData = backend;
}

bool isTalking()
{
    return getNPCInteractionCount() || getNPCPostCount()
            || getBuySellState() != BUYSELL_NONE;
}

void logic()
{
    if ((mSpecialRechargeUpdateNeeded%11) == 0)
    {
        mSpecialRechargeUpdateNeeded = 0;
        for (SpecialsMap::iterator it = mSpecials.begin(),
             it_end = mSpecials.end(); it != it_end; it++)
        {
            it->second.currentMana += it->second.recharge;
            if (it->second.currentMana > it->second.neededMana)
            {
                it->second.currentMana = it->second.neededMana;
            }
        }
    }
    mSpecialRechargeUpdateNeeded++;
}

class PlayerLogic : EventListener
{
public:
    PlayerLogic()
    {
        listen(Event::ClientChannel);
        listen(Event::GameChannel);
    }

    void event(Event::Channel channel, const Event &event)
    {
        if (channel == Event::ClientChannel)
        {
            if (event.getType() == Event::StateChange)
            {
                int newState = event.getInt("newState");

                if (newState == STATE_GAME)
                {
                    if (mInventory == 0)
                    {
                        mInventory = new Inventory(Inventory::INVENTORY);
                        mEquipment = new Equipment();
                    }
                }
            }
        }
        else if (channel == Event::GameChannel)
        {
            if (event.getType() == Event::Destructed)
            {
                delete mInventory;
                delete mEquipment;

                mInventory = 0;
                mEquipment = 0;
            }
        }
    }
};

void init()
{
    if (mListener)
        return;

    mListener = new PlayerLogic();
}

} // namespace PlayerInfo
