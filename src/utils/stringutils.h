/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include <charconv>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
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
 * Returns the number of bytes of the UTF-8 character at the start of the
 * given null-terminated string. Only as many continuation bytes as the lead
 * byte declares are included, so that invalid sequences are counted the same
 * way as by SDL_ttf.
 */
inline size_t utf8CharLength(const char *str)
{
    const unsigned char c = str[0];
    int left = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
    size_t length = 1;
    while (left-- > 0 && (str[length] & 0xC0) == 0x80)
        ++length;
    return length;
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
bool fromString(const char *str, T &value)
{
    return FromString<T>()(str, value);
}

inline bool fromString(const char *str, std::string &value)
{
    value = str;
    return true;
}

inline bool fromString(const char *str, std::string_view &value)
{
    value = str;
    return true;
}

inline bool fromString(const char *str, int &value)
{
    const char *end = str + std::strlen(str);
    return std::from_chars(str, end, value).ec == std::errc();
}

inline bool fromString(const char *str, unsigned &value)
{
    const char *end = str + std::strlen(str);
    return std::from_chars(str, end, value).ec == std::errc();
}

inline bool fromString(const char *str, unsigned short &value)
{
    const char *end = str + std::strlen(str);
    return std::from_chars(str, end, value).ec == std::errc();
}

#if __cpp_lib_to_chars >= 201611L
inline bool fromString(const char *str, float &value)
{
    const char *end = str + std::strlen(str);
    return std::from_chars(str, end, value, std::chars_format::general).ec == std::errc();
}

inline bool fromString(const char *str, double &value)
{
    const char *end = str + std::strlen(str);
    return std::from_chars(str, end, value, std::chars_format::general).ec == std::errc();
}
#else
inline bool fromString(const char *str, float &value)
{
    char *end;
    value = std::strtof(str, &end);
    return end != str && *end == '\0';
}

inline bool fromString(const char *str, double &value)
{
    char *end;
    value = std::strtod(str, &end);
    return end != str && *end == '\0';
}
#endif

inline bool fromString(const char *str, bool &value)
{
    value = getBoolFromString(str);
    return true;
}

bool fromString(const char *str, std::vector<int> &value);

template<typename T>
struct FromString<T, std::enable_if_t<std::is_enum_v<T>>>
{
    bool operator() (const char *str, T &value)
    {
        return fromString(str, reinterpret_cast<std::underlying_type_t<T>&>(value));
    }
};

template<typename T>
struct FromString<std::optional<T>>
{
    bool operator() (const char *str, std::optional<T> &value)
    {
        T tmp{};
        if (fromString(str, tmp)) {
            value = std::move(tmp);
            return true;
        }
        return false;
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
