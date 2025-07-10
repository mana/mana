/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <vector>

/**
 * Trims spaces off the end and the beginning of the given string.
 *
 * @param str the string to trim spaces off
 * @return a reference to the trimmed string
 */
std::string &trim(std::string &str);

/**
 * Converts the given string to lower case.
 *
 * @param str the string to convert to lower case
 * @return a reference to the given string converted to lower case
 */
std::string &toLower(std::string &str);

/**
 * Converts the given string to upper case.
 *
 * @param str the string to convert to upper case
 * @return a reference to the given string converted to upper case
 */
std::string &toUpper(std::string &str);


/**
 * Converts an ascii hexidecimal string to an integer
 *
 * @param str the hex string to convert to an int
 * @return the integer representation of the hex string
 */
unsigned int atox(const std::string &str);

/**
 * Converts the given value to a string using std::stringstream.
 *
 * @param arg the value to convert to a string
 * @return the string representation of arg
 */
template<typename T> std::string toString(const T &arg)
{
    std::ostringstream ss;
    ss << arg;
    return ss.str();
}

/**
 * Converts the given IP address to a string.
 *
 * The returned string is statically allocated, and shouldn't be freed. It is
 * changed upon the next use of this method.
 *
 * @param address the address to convert to a string
 * @return the string representation of the address
 */
const char *ipToString(int address);

/**
 * A safe version of sprintf that returns a std::string of the result.
 */
std::string strprintf(char const *, ...)
#ifdef __GNUC__
    /* This attribute is nice: it even works through gettext invokation. For
       example, gcc will complain that strprintf(_("%s"), 42) is ill-formed. */
    __attribute__((__format__(__printf__, 1, 2)))
#endif
;

/**
 * Replaces a set of characters with another character.
 *
 * @param str the string to remove the bad chars from
 * @param chars the bad characters to remove
 * @param replacement the character to replace the bad chars with
 * @return a reference to the string without bad chars
 */
std::string &replaceCharacters(std::string &str,
                               std::string_view chars,
                               char replacement = '_');

/**
 * Removes colors from a string.
 *
 * @param msg the string to remove the colors from
 * @return reference to the modified string
 */
std::string &removeColors(std::string &msg);

/**
 * Returns whether a string starts with a given prefix.
 */
inline bool startsWith(std::string_view str, std::string_view prefix)
{
    return str.substr(0, prefix.size()) == prefix;
}

/**
 * Returns whether a string ends with a given suffix.
 */
inline bool endsWith(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * Tells wether the character is a word separator.
 */
bool isWordSeparator(char chr);

std::string findSameSubstring(const std::string &str1,
                              const std::string &str2);

/**
 * Returns a bool value depending on the given string value.
 *
 * @param text the string used to get the bool value
 * @return a boolean value..
 */
bool getBoolFromString(std::string text, bool def = false);

/**
 * This class can be partially specialized to provide custom string conversion.
 *
 * This is done instead of overloading the base function template to avoid
 * ambiguity.
 */
template<typename T, typename Enable = void>
struct FromString;

template<typename T>
void fromString(const char *str, T &value)
{
    FromString<T>()(str, value);
}

inline void fromString(const char *str, std::string &value)
{
    value = str;
}

inline void fromString(const char *str, std::string_view &value)
{
    value = str;
}

inline void fromString(const char *str, int &value)
{
    value = atoi(str);
}

inline void fromString(const char *str, unsigned &value)
{
    value = strtoul(str, nullptr, 10);
}

inline void fromString(const char *str, unsigned short &value)
{
    value = static_cast<unsigned short>(strtoul(str, nullptr, 10));
}

inline void fromString(const char *str, float &value)
{
    value = strtof(str, nullptr);
}

inline void fromString(const char *str, double &value)
{
    value = atof(str);
}

inline void fromString(const char *str, bool &value)
{
    value = getBoolFromString(str);
}

void fromString(const char *str, std::vector<int> &value);

template<typename T>
struct FromString<T, std::enable_if_t<std::is_enum_v<T>>>
{
    void operator() (const char *str, T &value)
    {
        fromString(str, reinterpret_cast<std::underlying_type_t<T>&>(value));
    }
};

template<typename T>
struct FromString<std::optional<T>>
{
    void operator() (const char *str, std::optional<T> &value)
    {
        fromString(str, value.emplace());
    }
};

/**
 * Returns the most approaching string of base from candidates.
 */
std::string autocomplete(const std::vector<std::string> &candidates,
                         std::string base);

/**
 * Normalize a string, which means lowercase and trim it.
 */
std::string normalize(const std::string &name);

/**
 * Derives a directory from the given URL, stripping the schema and replacing
 * certain invalid characters.
 *
 * i.e.: http://www.manasource.org:9601/updates/ -> www.manasource.org_9601/updates/
 */
std::string getDirectoryFromURL(const std::string &url);

/**
 * Joins a vector of strings into one string, separated by the given
 * separator.
 */
std::string join(const std::vector<std::string> &strings, const char *separator);
