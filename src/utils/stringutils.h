/*
 *  The Mana World
 *  Copyright (C) 2007  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef UTILS_STRINGUTILS_H
#define UTILS_STRINGUTILS_H

#include <string>
#include <sstream>

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
 * Removes bad characters from a string
 *
 * @param str the string to remove the bad chars from
 * @return a reference to the string without bad chars
 */
std::string &removeBadChars(std::string &str);

#endif // UTILS_STRINGUTILS_H
