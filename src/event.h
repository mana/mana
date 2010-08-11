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
    /**
     * Makes an event with the given name.
     */
    Event(const std::string &name)
    { mEventName = name; }

    ~Event();

    /**
     * Returns the name of the event.
     */
    const std::string &getName() const
    { return mEventName; }

    /**
     * Sets the given variable to the given integer, if it isn't already set.
     */
    void setInt(const std::string &key, int value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and an integer.
     */
    int getInt(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and an integer, returning the
     * given default otherwise.
     */
    inline int getInt(const std::string &key, int defaultValue) const
    { try { return getInt(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is an integer.
     */
    bool hasInt(const std::string &key);

    /**
     * Sets the given variable to the given string, if it isn't already set.
     */
    void setString(const std::string &key, const std::string &value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and a string.
     */
    const std::string &getString(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and a string, returning the
     * given default otherwise.
     */
    inline const std::string &getString(const std::string &key,
                                        const std::string &defaultValue) const
    { try { return getString(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is a string.
     */
    bool hasString(const std::string &key);

    /**
     * Sets the given variable to the given floating-point, if it isn't already
     * set.
     */
    void setFloat(const std::string &key, double value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and a floating-point.
     */
    double getFloat(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and a floating-point, returning
     * the given default otherwise.
     */
    inline double getFloat(const std::string &key, float defaultValue) const
    { try { return getFloat(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is a floating-point.
     */
    bool hasFloat(const std::string &key);

    /**
     * Sets the given variable to the given boolean, if it isn't already set.
     */
    void setBool(const std::string &key, bool value) throw (BadEvent);

    /**
     * Returns the given variable if it is set and a boolean.
     */
    bool getBool(const std::string &key) const throw (BadEvent);

    /**
     * Returns the given variable if it is set and a boolean, returning the
     * given default otherwise.
     */
    inline bool getBool(const std::string &key, bool defaultValue) const
    { try { return getBool(key); } catch (BadEvent) { return defaultValue; }}

    /**
     * Returns true if the given variable exists and is a boolean.
     */
    bool hasBool(const std::string &key);

private:
    std::string mEventName;

    VariableMap mData;
};

} // namespace Mana

#endif
