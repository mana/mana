/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "resourcemanager.h"
#include "../log.h"
#include <iostream>
#include <sstream>
#include <physfs.h>

#ifdef WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

ResourceEntry::ResourceEntry():
    resource(NULL)
{
}

ResourceManager *ResourceManager::instance = NULL;

ResourceManager::ResourceManager()
{
    // Add zip files to PhysicsFS
    searchAndAddZipFiles();
}

ResourceManager::~ResourceManager()
{
    // Create our resource iterator.
    std::map<std::string, ResourceEntry>::iterator iter = resources.begin();
    int danglingResources = 0;
    int danglingReferences = 0;

    // Iterate through and release references until objects are deleted.
    while (iter != resources.end()) {
        Resource *res = iter->second.resource;
        danglingResources++;
        danglingReferences++;
        while (!res->decRef()) {
            danglingReferences++;
        }
        iter++;
    }
    resources.clear();

    log("ResourceManager::~ResourceManager() cleaned up %d references to %d"
            " resources", danglingReferences, danglingResources);
}

Resource* ResourceManager::get(const E_RESOURCE_TYPE &type,
        const std::string &idPath, int flags)
{
    // Check if the id exists, and return the value if it does.
    std::map<std::string, ResourceEntry>::iterator resIter =
        resources.find(idPath);

    if (resIter != resources.end() && resIter->second.resource) {
        resIter->second.resource->incRef();
        return resIter->second.resource;
    }

    // The filePath string.
    std::string filePath = "";

    // Set the filePath variable to the appropriate value
    // this is only if we're not using a packed file.
    filePath = std::string("data/") + idPath;

    Resource *resource = NULL;

    // Create an object of the specified type.
    switch (type)
    {
        case MAP:
            log("Warning: Map resource not supported.");
            break;
        case MUSIC:
            log("Warning: Music resource not supported.");
            break;
        case IMAGE:
            // Attempt to create and load our image object.
            resource =
                reinterpret_cast<Resource*>(Image::load(filePath, flags));

            // If the object is invalid, try using PhysicsFS
            if (resource == NULL) {
                std::cout << "Check if exists: " << filePath << std::endl;
                // If the file is in the PhysicsFS search path
                if (PHYSFS_exists(filePath.c_str()) != 0) {                    
                    // Open the file for read
                    PHYSFS_file* imageFile = PHYSFS_openRead(filePath.c_str());
                    std::cout << filePath << " - " <<
                        PHYSFS_fileLength(imageFile) << " bytes" << std::endl;

                    // Read the file data into a buffer
                    char* buffer = new char[PHYSFS_fileLength(imageFile)];
                    PHYSFS_read(imageFile, buffer, 1,
                            PHYSFS_fileLength(imageFile));

                    // Cast the file to a valid resource
                    resource = reinterpret_cast<Resource*>(Image::load(buffer,
                                PHYSFS_fileLength(imageFile)));

                    // Close the file
                    PHYSFS_close(imageFile);
                    delete[] buffer;
                }
            }

            break;
        case SCRIPT:
            log("Warning: Script resource not supported.");
            break;
        case TILESET:
            log("Warning: Tileset resource not supported.");
            break;
        case SOUND_EFFECT:
            log("Warning: Sound FX resource not supported.");
            break;
        default: 
            log("Warning: Unknown resource type");
            break;
    }

    if (resource) {
        resource->incRef();

        // Create the resource entry for this object.
        ResourceEntry entry;
        entry.filePath = filePath;
        entry.resource = resource;

        resources[idPath] = entry;
    }

    // Return NULL if the object could not be created.
    return resource;
}

Image *ResourceManager::getImage(const std::string &idPath, int flags)
{
    return (Image*)get(IMAGE, idPath, flags);
}

ResourceManager* ResourceManager::getInstance()
{
    // Create a new instance if necessary.
    if (instance == NULL) instance = new ResourceManager();
    return instance;
}

void ResourceManager::deleteInstance()
{
    if (instance != NULL) {
        delete instance;
        instance = NULL;
    }
}

void ResourceManager::searchAndAddZipFiles()
{
    // Define the path in which to search
    std::string searchString = std::string("data/*.zip");

#ifdef _WIN32
    // Create our find file data structure
    struct _finddata_t findFileInfo;

    // Find the first zipped file
    long handle =
        static_cast<long>(::_findfirst(searchString.c_str(), &findFileInfo));
    long file = handle;

    // Loop until all files we're searching for are found
    while (file >= 0) {
        // Define the file path string
        std::string filePath = std::string("data/") +
            std::string(findFileInfo.name);

        log("Adding to PhysicsFS: %s", findFileInfo.name);

        // Add the zip file to our PhysicsFS search path
        PHYSFS_addToSearchPath(filePath.c_str(), 1);

        // Find the next file
        file = ::_findnext(handle, &findFileInfo);
    }

    // Shutdown findfile stuff
    ::_findclose(handle);
#else
    // Retrieve the current path
    char programPath[256];
    getcwd(programPath, 256);
    strncat(programPath, "/data", 256 - strlen(programPath) - 1);

    // Create our directory structure
    DIR *dir = opendir(programPath);

    // Return if the directory is invalid
    if (dir == NULL) {
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(dir)) != NULL) {
        char *ext = strstr(direntry->d_name, ".zip");
        if (ext != NULL && strcmp(ext, ".zip") == 0) {
            // Define the file path string
            std::string filePath = std::string(programPath) +
                std::string("/") + std::string(direntry->d_name);

            log("Adding to PhysicsFS: %s", filePath.c_str());

            // Add the zip file to our PhysicsFS search path
            PHYSFS_addToSearchPath(filePath.c_str(), 1);
        }
    }

    closedir(dir);
#endif
}
