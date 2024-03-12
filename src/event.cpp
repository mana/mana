/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#include "eventlistener.h"
#include "variabledata.h"

Event::ListenMap Event::mBindings;

Event::~Event()
{
    auto it = mData.begin();
    while (it != mData.end())
    {
        delete it->second;
        it++;
    }
}

// Integers

void Event::setInt(const std::string &key, int value)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new IntData(value);
}

int Event::getInt(const std::string &key) const
{
    auto it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_INT)
        throw BAD_VALUE;

    return static_cast<IntData *>(it->second)->getData();
}

// Strings

void Event::setString(const std::string &key, const std::string &value)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new StringData(value);
}

const std::string &Event::getString(const std::string &key) const
{
    auto it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_STRING)
        throw BAD_VALUE;

    return static_cast<StringData *>(it->second)->getData();
}


// Floats

void Event::setFloat(const std::string &key, double value)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new FloatData(value);
}

double Event::getFloat(const std::string &key) const
{
    auto it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_FLOAT)
        throw BAD_VALUE;

    return static_cast<FloatData *>(it->second)->getData();
}

// Booleans

void Event::setBool(const std::string &key, bool value)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new BoolData(value);
}

bool Event::getBool(const std::string &key) const
{
    auto it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_BOOL)
        throw BAD_VALUE;

    return static_cast<BoolData *>(it->second)->getData();
}

// Items

void Event::setItem(const std::string &key, Item *value)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new ItemData(value);
}

Item *Event::getItem(const std::string &key) const
{
    auto it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_ITEM)
        throw BAD_VALUE;

    return static_cast<ItemData *>(it->second)->getData();
}

// Actors

void Event::setActor(const std::string &key, ActorSprite *value)
{
    if (mData.find(key) != mData.end())
        throw KEY_ALREADY_EXISTS;

    mData[key] = new ActorData(value);
}

ActorSprite *Event::getActor(const std::string &key) const
{
    auto it = mData.find(key);
    if (it == mData.end())
        throw BAD_KEY;

    if (it->second->getType() != VariableData::DATA_ACTOR)
        throw BAD_VALUE;

    return static_cast<ActorData *>(it->second)->getData();
}

// Triggers

void Event::trigger(Channel channel, const Event &event)
{
    auto it = mBindings.find(channel);

    // Make sure something is listening
    if (it == mBindings.end())
        return;

    // Loop though all listeners
    auto lit = it->second.begin();
    while (lit != it->second.end())
    {
        (*lit)->event(channel, event);
        lit++;
    }
}

void Event::bind(EventListener *listener, Channel channel)
{
    mBindings[channel].insert(listener);
}

void Event::unbind(EventListener *listener, Channel channel)
{
    mBindings[channel].erase(listener);
}

void Event::remove(EventListener *listener)
{
    auto it = mBindings.begin();
    while (it != mBindings.end())
    {
        it->second.erase(listener);
        it++;
    }
}
