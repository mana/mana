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

#include <cassert>

#ifdef WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

ResourceManager *ResourceManager::instance = NULL;

ResourceManager::ResourceManager()
{
    // Add zip files to PhysicsFS
    searchAndAddZipFiles();
}

ResourceManager::~ResourceManager()
{
    // Create our resource iterator.
    std::map<std::string, Resource*>::iterator iter = resources.begin();
    int danglingResources = 0;
    int danglingReferences = 0;

    // Iterate through and release references until objects are deleted.
    while (!resources.empty())
    {
        Resource *res = resources.begin()->second;
        danglingResources++;

        do {
            danglingReferences++;
        }
        while (!res->decRef());
    }

    logger->log("ResourceManager::~ResourceManager() cleaned up %d references "
            "to %d resources", danglingReferences, danglingResources);
}

Resource*
ResourceManager::get(const E_RESOURCE_TYPE &type, const std::string &idPath)
{
    // Check if the id exists, and return the value if it does.
    std::map<std::string, Resource*>::iterator resIter =
        resources.find(idPath);

    if (resIter != resources.end() && resIter->second) {
        resIter->second->incRef();
        return resIter->second;
    }

    logger->log("ResourceManager::get(%s)", idPath.c_str());

    Resource *resource = NULL;

    // Create an object of the specified type.
    switch (type)
    {
        case MAP:
            logger->log("Warning: Map resource not supported.");
            break;
        case MUSIC:
            {
                // Load the music resource file
                int fileSize;
                void *buffer = loadFile(idPath, fileSize);

                if (buffer != NULL)
                {
                    // Let the music class load it
                    resource = Music::load(buffer, fileSize);

                    // Cleanup
                    free(buffer);
                }
                else {
                    logger->log("Warning: resource doesn't exist!");
                }
            }
            break;
        case IMAGE:
            {
                // Load the image resource file
                int fileSize;
                void *buffer = loadFile(idPath, fileSize);

                if (buffer != NULL)
                {
                    // Let the image class load it
                    resource = Image::load(buffer, fileSize);

                    // Cleanup
                    free(buffer);
                }
                else {
                    logger->log("Warning: resource doesn't exist!");
                }
            }
            break;
        case SCRIPT:
            logger->log("Warning: Script resource not supported.");
            break;
        case TILESET:
            logger->log("Warning: Tileset resource not supported.");
            break;
        case SOUND_EFFECT:
            {
                // Load the sample resource file
                int fileSize;
                void *buffer = loadFile(idPath, fileSize);

                if (buffer != NULL)
                {
                    // Let the sound effect class load it
                    resource = SoundEffect::load(buffer, fileSize);

                    // Cleanup
                    free(buffer);
                }
                else {
                    logger->log("Warning: resource doesn't exist!");
                }
            }
            break;
        default: 
            logger->log("Warning: Unknown resource type");
            break;
    }

    if (resource) {
        resource->incRef();
        resource->setIdPath(idPath);

        resources[idPath] = resource;
    }

    // Return NULL if the object could not be created.
    return resource;
}

Image*
ResourceManager::getImage(const std::string &idPath)
{
    return (Image*)get(IMAGE, idPath);
}

Music*
ResourceManager::getMusic(const std::string &idPath)
{
    return (Music*)get(MUSIC, idPath);
}

SoundEffect*
ResourceManager::getSoundEffect(const std::string &idPath)
{
    return (SoundEffect*)get(SOUND_EFFECT, idPath);
}

void
ResourceManager::release(const std::string &idPath)
{
    logger->log("ResourceManager::release(%s)", idPath.c_str());

    std::map<std::string, Resource*>::iterator resIter =
        resources.find(idPath);

    // The resource has to exist
    assert(resIter != resources.end() && resIter->second);

    resources.erase(idPath);
}

ResourceManager*
ResourceManager::getInstance()
{
    // Create a new instance if necessary.
    if (instance == NULL) instance = new ResourceManager();
    return instance;
}

void
ResourceManager::deleteInstance()
{
    if (instance != NULL) {
        delete instance;
        instance = NULL;
    }
}

void
ResourceManager::searchAndAddZipFiles()
{
    // Add the main data directory to our PhysicsFS search path
    PHYSFS_addToSearchPath("data", 1);
    PHYSFS_addToSearchPath(TMW_DATADIR "data", 1);

#ifdef _WIN32
    // Define the path in which to search
    std::string searchString = std::string("data/*.zip");

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

        logger->log("Adding to PhysicsFS: %s", findFileInfo.name);

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

            logger->log("Adding to PhysicsFS: %s", filePath.c_str());

            // Add the zip file to our PhysicsFS search path
            PHYSFS_addToSearchPath(filePath.c_str(), 1);
        }
    }

    closedir(dir);
#endif
}

void*
ResourceManager::loadFile(const std::string &fileName, int &fileSize)
{
    // If the file doesn't exist indicate failure
    if (!PHYSFS_exists(fileName.c_str())) {
        logger->log("Warning: %s not found!", fileName.c_str());
        return NULL;
    }

    // Attempt to open the specified file using PhysicsFS
    PHYSFS_file* file = PHYSFS_openRead(fileName.c_str());

    // If the handler is an invalid pointer indicate failure
    if (file == NULL) {
        logger->log("Warning: %s failed to load!", fileName.c_str());
        return NULL;
    }

    // Get the size of the file
    fileSize = PHYSFS_fileLength(file);

    // Allocate memory and load the file
    void *buffer = malloc(fileSize);
    PHYSFS_read(file, buffer, 1, fileSize);

    // Close the file and let the user deallocate the memory
    PHYSFS_close(file);

    return buffer;
}
