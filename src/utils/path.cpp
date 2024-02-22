/*
 *  The Mana Server
 *  Copyright (C) 2013  The Mana World Development Team
 *
 *  This file is part of The Mana Server.
 *
 *  The Mana Server is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana Server is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana Server.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils/path.h"
#include <vector>

namespace utils
{

    /**
     * Returns the filePath sub-part corresponding to the filename only.
     * @return splittedPath: the file path ending with '/' or '\'
     *                       and the file name alone.
     */
    splittedPath splitFileNameAndPath(const std::string &fullFilePath)
    {
        // We'll reversed-search for '/' or'\' and extract the substrings
        // corresponding to the filename and the path separately.
        size_t slashPos = fullFilePath.find_last_of("/\\");

        splittedPath splittedFilePath;
        // Note the last slash is kept in the path name.
        splittedFilePath.path = fullFilePath.substr(0, slashPos + 1);
        splittedFilePath.file = fullFilePath.substr(slashPos + 1);

        return splittedFilePath;
    }

    /**
     * Join two path elements into one.
     *
     * This function helps build relative paths.
     *
     * Examples:
     *
     *     /foo + bar = /foo/bar
     *     /foo/ + bar = /foo/bar
     *     /foo + /bar = /bar
     *
     * This will work for PhysFS paths. Windows style paths (prefixed with drive letters) won't work.
     *
     * @return Joined paths or path2 if path2 was an absolute path.
     */
    std::string joinPaths(const std::string &path1, const std::string &path2)
    {
        if (path2.empty())
            return path1;

        if (path1.empty())
            return path2;

        // check if path2 is an absolute path that cannot be joined
        if (path2[0] == '/' || path2[0] == '\\')
            return path2;

        char p1end = path1[path1.size()-1];
        if (p1end == '/' || p1end == '\\')
        {
            return path1 + path2;
        }

        return path1 + "/" + path2;
    }

    /**
     * Removes relative elements from the path.
     */
    std::string cleanPath(const std::string &path)
    {
        std::string part;
        std::string result;
        std::vector<std::string> pathStack;

        size_t prev = 0;
        while (true)
        {
            size_t cur = path.find_first_of("/\\", prev);
            if (cur == std::string::npos)
            {
                // FIXME add everything from prev to the end
                pathStack.push_back(path.substr(prev));
                break;
            }

            part = path.substr(prev, cur - prev);
            if (part == "..")
            {
                // go back one level
                if (!pathStack.empty())
                {
                    pathStack.pop_back();
                }
            }
            else if (part == ".")
            {
                // do nothing
            }
            else if (part.empty())
            {
                if (pathStack.empty() && cur == 0)
                {
                    // handle first empty match before the root slash
                    pathStack.emplace_back();
                }
                else
                {
                    // empty match in the middle of the path should be ignored
                }
            }
            else
            {
                // normal path element
                pathStack.push_back(part);
            }

            cur++;
            prev = cur;
        }

        // join the pathStack into a normal path
        unsigned int i = 0;
        for (i = 0; i < pathStack.size(); i++)
        {
            result += pathStack[i];
            if (i < pathStack.size() - 1) {
                result += "/";
            }
        }

        return result;
    }


}
