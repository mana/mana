/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#ifdef _WIN32
#include "specialfolder.h"
#include <windows.h>

#ifdef SPECIALFOLDERLOCATION_TEST
// compile with -DSPECIALFOLDERLOCATION_TEST to get a standalone
// binary for testing
#include <iostream>
#endif

/*
 * Retrieve the pathname of special folders on win32, or an empty string
 * on error / if the folder does not exist.
 * See http://msdn.microsoft.com/en-us/library/bb762494(VS.85).aspx for
 * a list of folder ids
 */
std::string getSpecialFolderLocation(int folderId)
{
    std::string ret;
    LPITEMIDLIST pItemIdList;
    LPMALLOC pMalloc;
    char szPath[MAX_PATH];

    // get the item ID list for folderId
    HRESULT hr = SHGetSpecialFolderLocation(NULL, folderId, &pItemIdList);
    if (hr != S_OK)
        return ret;

    // convert the ID list into a file system path
    if (SHGetPathFromIDList(pItemIdList, szPath) == FALSE)
        return ret;

    // get the IMalloc pointer and free all resources we used
    hr = SHGetMalloc(&pMalloc);
    pMalloc->Free(pItemIdList);
    pMalloc->Release();

    ret = szPath;
    return ret;
}

#ifdef SPECIALFOLDERLOCATION_TEST
int main()
{
    std::cout << "APPDATA " << getSpecialFolderLocation(CSIDL_APPDATA)
              << std::endl;
    std::cout << "DESKTOP " << getSpecialFolderLocation(CSIDL_DESKTOP)
              << std::endl;
    std::cout << "LOCAL_APPDATA "
              << getSpecialFolderLocation(CSIDL_LOCAL_APPDATA)
              << std::endl;
    std::cout << "MYPICTURES " << getSpecialFolderLocation(CSIDL_MYPICTURES)
              << std::endl;
    std::cout << "PERSONAL " << getSpecialFolderLocation(CSIDL_PERSONAL)
              << std::endl;
}
#endif
#endif
