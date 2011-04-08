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

#include "event.h"

#include "listener.h"
#include "variabledata.h"

Event::ListenMap Event::mBindings;

Event::~Event()
{
    VariableMap::iterator it = mData.begin();
    while (it != mData.end())
    {
        delete it->second;
        it++;
    }
}

// Integers

void Event::setInt(const std::string &key, int value) throw (BadEvent)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new IntData(value);
}

int Event::getInt(const std::string &key) const throw (BadEvent)
{
    VariableMap::const_iterator it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_INT)
        throw BAD_VALUE;

    return static_cast<IntData *>(it->second)->getData();
}

bool Event::hasInt(const std::string &key) const
{
    VariableMap::const_iterator it = mData.find(key);
    return !(it == mData.end()
             || it->second->getType() != VariableData::DATA_INT);
}

// Strings

void Event::setString(const std::string &key, const std::string &value) throw (BadEvent)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new StringData(value);
}

const std::string &Event::getString(const std::string &key) const throw (BadEvent)
{
    VariableMap::const_iterator it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_STRING)
        throw BAD_VALUE;

    return static_cast<StringData *>(it->second)->getData();
}


bool Event::hasString(const std::string &key) const
{
    VariableMap::const_iterator it = mData.find(key);
    return !(it == mData.end()
             || it->second->getType() != VariableData::DATA_STRING);
}

// Floats

void Event::setFloat(const std::string &key, double value) throw (BadEvent)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new FloatData(value);
}

double Event::getFloat(const std::string &key) const throw (BadEvent)
{
    VariableMap::const_iterator it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_FLOAT)
        throw BAD_VALUE;

    return static_cast<FloatData *>(it->second)->getData();
}

bool Event::hasFloat(const std::string &key) const
{
    VariableMap::const_iterator it = mData.find(key);
    return !(it == mData.end()
             || it->second->getType() != VariableData::DATA_FLOAT);
}

// Booleans

void Event::setBool(const std::string &key, bool value) throw (BadEvent)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new BoolData(value);
}

bool Event::getBool(const std::string &key) const throw (BadEvent)
{
    VariableMap::const_iterator it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_BOOL)
        throw BAD_VALUE;

    return static_cast<BoolData *>(it->second)->getData();
}

bool Event::hasBool(const std::string &key) const
{
    VariableMap::const_iterator it = mData.find(key);
    return !(it == mData.end()
             || it->second->getType() != VariableData::DATA_BOOL);
}

// Items

void Event::setItem(const std::string &key, Item *value) throw (BadEvent)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new ItemData(value);
}

Item *Event::getItem(const std::string &key) const throw (BadEvent)
{
    VariableMap::const_iterator it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_ITEM)
        throw BAD_VALUE;

    return static_cast<ItemData *>(it->second)->getData();
}

bool Event::hasItem(const std::string &key) const
{
    VariableMap::const_iterator it = mData.find(key);
    return !(it == mData.end()
             || it->second->getType() != VariableData::DATA_ITEM);
}

// Actors

void Event::setActor(const std::string &key, ActorSprite *value) throw (BadEvent)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new ActorData(value);
}

ActorSprite *Event::getActor(const std::string &key) const throw (BadEvent)
{
    VariableMap::const_iterator it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_ACTOR)
        throw BAD_VALUE;

    return static_cast<ActorData *>(it->second)->getData();
}

bool Event::hasActor(const std::string &key) const
{
    VariableMap::const_iterator it = mData.find(key);
    return !(it == mData.end()
             || it->second->getType() != VariableData::DATA_ACTOR);
}

// Triggers

void Event::trigger(Channel channel, const Event &event)
{
    ListenMap::iterator it = mBindings.find(channel);

    // Make sure something is listening
    if (it == mBindings.end())
        return;

    // Loop though all listeners
    ListenerSet::iterator lit = it->second.begin();
    while (lit != it->second.end())
    {
        (*lit)->event(channel, event);
        lit++;
    }
}

void Event::bind(Listener *listener, Channel channel)
{
    mBindings[channel].insert(listener);
}

void Event::unbind(Listener *listener, Channel channel)
{
    mBindings[channel].erase(listener);
}

void Event::remove(Listener *listener)
{
    ListenMap::iterator it = mBindings.begin();
    while (it != mBindings.end())
    {
        it->second.erase(listener);
        it++;
    }
}
