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

#include "eventmanager.h"

#include "listener.h"

namespace Mana
{

ListenMap EventManager::mBindings;

void EventManager::trigger(const std::string &channel, const Event &event)
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

void EventManager::remove(Listener *listener)
{
    ListenMap::iterator it = mBindings.begin();
    while (it != mBindings.end())
    {
        it->second.erase(listener);
        it++;
    }
}

void EventManager::bind(Listener *listener, const std::string &channel)
{
    mBindings[channel].insert(listener);
}

void EventManager::unbind(Listener *listener, const std::string &channel)
{
    mBindings[channel].erase(listener);
}

}; // namespace Mana
