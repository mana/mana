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

#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <map>

namespace Mana
{

// Possible exception that can be thrown
enum BadEvent {
    BAD_KEY,
    BAD_VALUE,
    KEY_ALREADY_EXISTS
};

class VariableData;
typedef std::map<std::string, VariableData *> VariableMap;

class Event
{
public:
    // String passed can be retivered with getName()
    // and is to used to identify what type of event
    // this is.
    Event(const std::string &name)
    { mEventName = name; }

    ~Event();

    const std::string &getName() const
    { return mEventName; }

    // Sets or gets a interger with a key to identify
    void setInt(const std::string &key, int value) throw (BadEvent);
    int getInt(const std::string &key) const throw (BadEvent);

    // Sets or gets a string with a key to identify
    void setString(const std::string &key, const std::string &value) throw (BadEvent);
    const std::string &getString(const std::string &key) const throw (BadEvent);

    // Sets or gets a floating point number with key to identify
    void setFloat(const std::string &key, double value) throw (BadEvent);
    double getFloat(const std::string &key) const throw (BadEvent);

    // Sets or gets a boolean with key to identify
    void setBool(const std::string &key, bool value) throw (BadEvent);
    bool getBool(const std::string &key) const throw (BadEvent);

private:
    std::string mEventName;

    VariableMap mData;
};

} // namespace Mana

#endif
