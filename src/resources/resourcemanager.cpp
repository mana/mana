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

#include "../main.h"
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

    logger.log("ResourceManager::~ResourceManager() cleaned up %d references to %d"
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
            logger.log("Warning: Map resource not supported.");
            break;
        case MUSIC:
            logger.log("Warning: Music resource not supported.");
            break;
        case IMAGE:
            // Attempt to create and load our image object.
            resource =
                reinterpret_cast<Resource*>(Image::load(filePath, flags));

            // If the object is invalid, try using PhysicsFS
            if (resource == NULL) {
                std::cout << "Check if exists: " << filePath << std::endl;

                // Load the image resource file
                void* buffer = NULL;
                int fileSize = loadFile(filePath, buffer);

                // Let the image class load it
                resource = reinterpret_cast<Resource*>(Image::load(buffer,
                                fileSize));

                // Cleanup
                if (buffer != NULL) {
                    delete[] buffer;
                }
            }

            break;
        case SCRIPT:
            logger.log("Warning: Script resource not supported.");
            break;
        case TILESET:
            logger.log("Warning: Tileset resource not supported.");
            break;
        case SOUND_EFFECT:
            logger.log("Warning: Sound FX resource not supported.");
            break;
        default: 
            logger.log("Warning: Unknown resource type");
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

            logger.log("Adding to PhysicsFS: %s", filePath.c_str());

            // Add the zip file to our PhysicsFS search path
            PHYSFS_addToSearchPath(filePath.c_str(), 1);
        }
    }

    closedir(dir);
#endif
}

int ResourceManager::loadFile(const std::string& fileName, void* buffer)
{
    // If the file doesn't exist indicate failure
    if (PHYSFS_exists(fileName.c_str()) != 0) return -1;

    // Initialize the buffer value
    buffer = NULL;

    // Attempt to open the specified file using PhysicsFS
    PHYSFS_file* file = PHYSFS_openRead(fileName.c_str());

    // If the handler is an invalid pointer indicate failure
    if (file == NULL) return -1;

    // Print file information message
    int fileLength = PHYSFS_fileLength(file);

    std::cout << fileName   << " - "
              << fileLength << " bytes" << std::endl;

    // Allocate memory in the buffer and load the file
    buffer = (void*)new char[fileLength];
    PHYSFS_read(file, buffer, 1, fileLength);

    // Close the file and let the user deallocate the memory (safe?)
    PHYSFS_close(file);

    return fileLength;
}
