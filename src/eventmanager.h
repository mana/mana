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

#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <string>
#include <map>
#include <set>

namespace Mana
{

class Event;
class Listener;

typedef std::set<Listener *> ListenerSet;
typedef std::map<std::string, ListenerSet > ListenMap;

class EventManager
{
    public:
    // Sends event to all listener on the channel
    static void trigger(const std::string &channel, const Event &event);

    // Removes a listener from all channels
    static void remove(Listener *listener);

    // Adds or removes a listener to a channel.
    static void bind(Listener *listener, const std::string &channel);
    static void unbind(Listener *listener, const std::string &channel);

    private:
    static ListenMap mBindings;
};

}; // namespace Mana

#endif
