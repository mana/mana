/*
 *  The Mana Client
 *  Copyright (C) 2010-2024  The Mana Developers
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

#include "specialfolder.h"

#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>

#ifdef SPECIALFOLDERLOCATION_TEST
// compile with -DSPECIALFOLDERLOCATION_TEST to get a standalone
// binary for testing
#include <iostream>
#endif

/*
 * Retrieve the pathname of special folders on Windows, or an empty string
 * on error.
 *
 * See https://learn.microsoft.com/en-us/windows/win32/shell/knownfolderid
 * for a list of folder IDs.
 */
std::string getSpecialFolderLocation(const KNOWNFOLDERID &folderId)
{
    std::string ret;
    PWSTR widePath = 0;

    HRESULT hr = SHGetKnownFolderPath(folderId, 0, NULL, &widePath);
    if (hr == S_OK)
    {
        // determine needed bytes
        size_t len;
        if (wcstombs_s(&len, nullptr, 0, widePath, 0) == 0)
        {
            ret.resize(len - 1);    // subtract null character
            if (wcstombs_s(nullptr, ret.data(), len, widePath, len - 1) != 0)
                ret.clear();
        }
    }

    CoTaskMemFree(widePath);

    return ret;
}

#ifdef SPECIALFOLDERLOCATION_TEST
int main()
{
    std::cout << "RoamingAppData " << getSpecialFolderLocation(FOLDERID_RoamingAppData)
              << std::endl;
    std::cout << "Desktop " << getSpecialFolderLocation(FOLDERID_Desktop)
              << std::endl;
    std::cout << "LocalAppData " << getSpecialFolderLocation(FOLDERID_LocalAppData)
              << std::endl;
    std::cout << "Pictures " << getSpecialFolderLocation(FOLDERID_Pictures)
              << std::endl;
    std::cout << "Documents " << getSpecialFolderLocation(FOLDERID_Documents)
              << std::endl;
}
#endif
#endif // _WIN32

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>

std::string getResourcesLocation()
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path,
                                          PATH_MAX))
    {
        fprintf(stderr, "Can't find Resources directory\n");
    }
    CFRelease(resourcesURL);
    return path;
}
#endif // __APPLE__
