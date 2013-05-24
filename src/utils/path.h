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

#ifndef UTILS_PATH_H
#define UTILS_PATH_H

#include <string>

namespace utils
{

    struct splittedPath
    {
        std::string path;
        std::string file;
    };

    splittedPath splitFileNameAndPath(const std::string &fullFilePath);

    std::string joinPaths(const std::string &path1, const std::string &path2);

    std::string cleanPath(const std::string &path);

}
#endif // UTILS_PATH_H
