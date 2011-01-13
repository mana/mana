/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef UTILS_STRINGUTILS_H
#define UTILS_STRINGUTILS_H

#include <string>
#include <sstream>
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
    std::stringstream ss;
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
 * Removes bad characters from a string
 *
 * @param str the string to remove the bad chars from
 * @return a reference to the string without bad chars
 */
std::string &removeBadChars(std::string &str);

/**
 * Removes colors from a string
 *
 * @param msg the string to remove the colors from
 * @return string without colors
 */
std::string removeColors(std::string msg);

/**
 * Compares the two strings case-insensitively.
 *
 * @param a the first string in the comparison
 * @param b the second string in the comparison
 * @return 0 if the strings are equal, positive if the first is greater,
 *           negative if the second is greater
 */
int compareStrI(const std::string &a, const std::string &b);

/**
 * Tells wether the character is a word separator.
 */
bool isWordSeparator(char chr);

const std::string findSameSubstring(const std::string &str1,
                                    const std::string &str2);

const char* getSafeUtf8String(std::string text);

/**
 * Returns a bool value depending on the given string value.
 *
 * @param text the string used to get the bool value
 * @return a boolean value..
 */
bool getBoolFromString(const std::string &text, bool def = false);

/**
 * Returns the most approaching string of base from candidates.
 */
std::string autocomplete(std::vector<std::string> &candidates,
                         std::string base);

/**
 * Normalize a string, which means lowercase and trim it.
 */
std::string normalize(const std::string &name);

#endif // UTILS_STRINGUTILS_H
