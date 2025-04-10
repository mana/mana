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

#include "utils/stringutils.h"

#include <cstring>
#include <algorithm>
#include <cstdarg>
#include <cstdio>

std::string &trim(std::string &str)
{
    std::string::size_type pos = str.find_last_not_of(' ');
    if (pos != std::string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');

        if (pos != std::string::npos)
            str.erase(0, pos);
    }
    else
    {
        // There is nothing else but whitespace in the string
        str.clear();
    }
    return str;
}

std::string &toLower(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

std::string &toUpper(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}

unsigned int atox(const std::string &str)
{
    unsigned int value;
    sscanf(str.c_str(), "0x%06x", &value);

    return value;
}

const char *ipToString(int address)
{
    static char asciiIP[16];

    snprintf(asciiIP, 16, "%i.%i.%i.%i",
             (unsigned char)(address),
             (unsigned char)(address >> 8),
             (unsigned char)(address >> 16),
             (unsigned char)(address >> 24));

    return asciiIP;
}

std::string strprintf(char const *format, ...)
{
    char buf[256];
    va_list args;
    va_start(args, format);
    int nb = vsnprintf(buf, 256, format, args);
    va_end(args);
    if (nb < 256)
    {
        return buf;
    }
    // The static size was not big enough, try again with a dynamic allocation.
    ++nb;   // Add 1 for the null terminator.

    std::string res(nb, char());
    va_start(args, format);
    vsnprintf(res.data(), nb, format, args);
    va_end(args);
    return res;
}

std::string &replaceCharacters(std::string &str,
                               std::string_view chars,
                               char replacement)
{
    for (auto &c : str)
    {
        if (chars.find(c) != std::string::npos)
            c = replacement;
    }
    return str;
}

std::string &removeColors(std::string &msg)
{
    auto pos = msg.find("##");
    while (pos != std::string::npos && msg.length() - pos >= 3)
    {
        msg.erase(pos, 3);
        pos = msg.find("##", pos);
    }
    return msg;
}

bool isWordSeparator(char chr)
{
    return (chr == ' ' || chr == ',' || chr == '.' || chr == '"');
}

std::string findSameSubstring(const std::string &str1,
                              const std::string &str2)
{
    int minLength = str1.length() > str2.length() ? str2.length() : str1.length();
    for (int f = 0; f < minLength; f ++)
    {
        if (str1.at(f) != str2.at(f))
        {
            return str1.substr(0, f);
        }
    }
    return str1.substr(0, minLength);
}

bool getBoolFromString(std::string text, bool def)
{
    toLower(trim(text));
    if (text == "true" || text == "1" || text == "on" || text == "yes" || text == "y")
        return true;
    if (text == "false" || text == "0" || text == "off" || text == "no" || text == "n")
        return false;

    return def;
}

std::string autocomplete(const std::vector<std::string> &candidates,
                         std::string base)
{
    auto i = candidates.begin();
    toLower(base);
    std::string newName;

    while (i != candidates.end())
    {
        if (!i->empty())
        {
            std::string name = *i;
            toLower(name);

            std::string::size_type pos = name.find(base, 0);
            if (pos == 0)
            {
                if (!newName.empty())
                {
                    toLower(newName);
                    newName = findSameSubstring(name, newName);
                }
                else
                {
                    newName = *i;
                }
            }
        }

        ++i;
    }

    return newName;
}

std::string normalize(const std::string &name)
{
    std::string normalized = name;
    return toLower(trim(normalized));
}

std::string getDirectoryFromURL(const std::string &url)
{
    std::string directory = url;

    // Parse out any "http://", "ftp://", etc...
    size_t pos = directory.find("://");
    if (pos != std::string::npos)
        directory.erase(0, pos + 3);

    // Replace characters which are not valid or difficult in file system paths
    replaceCharacters(directory, ":*?\"<>| ", '_');

    // Replace ".." (double dots) with "_" to avoid directory traversal.
    pos = directory.find("..");
    while (pos != std::string::npos)
    {
        directory.replace(pos, 2, "_");
        pos = directory.find("..");
    }

    return directory;
}

std::string join(const std::vector<std::string> &strings, const char *separator)
{
    std::string result;
    if (auto i = strings.begin(), e = strings.end(); i != e)
    {
        result += *i++;
        for (; i != e; ++i)
            result.append(separator).append(*i);
    }
    return result;
}
