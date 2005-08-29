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

#include <cassert>
#include <physfs.h>

#include "image.h"
#include "music.h"
#include "soundeffect.h"

#include "../log.h"


ResourceManager *ResourceManager::instance = NULL;

ResourceManager::ResourceManager()
{
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

bool ResourceManager::setWriteDir(const std::string &path)
{
    return (bool)PHYSFS_setWriteDir(path.c_str());
}

void ResourceManager::addToSearchPath(const std::string &path, bool append)
{
    PHYSFS_addToSearchPath(path.c_str(), append ? 1 : 0);
}

void ResourceManager::searchAndAddArchives(
        const std::string &path, const std::string &ext, bool append)
{
    const char *dirSep = PHYSFS_getDirSeparator();
    char **list = PHYSFS_enumerateFiles(path.c_str());

    for (char **i = list; *i != NULL; i++)
    {
        size_t len = strlen(*i);

        if (len > ext.length() && !ext.compare((*i)+(len - ext.length())))
        {
            std::string file, realPath, archive;

            file = path + "/" + (*i);
            realPath = std::string(PHYSFS_getRealDir(file.c_str()));
            archive = realPath + path + dirSep + (*i);

            logger->log("Adding to PhysicsFS: %s", archive.c_str());
            addToSearchPath(archive, append);
        }
    }

    PHYSFS_freeList(list);
}

bool ResourceManager::mkdir(const std::string &path)
{
    return (bool)PHYSFS_mkdir(path.c_str());
}

bool ResourceManager::exists(const std::string &path)
{
    return PHYSFS_exists(path.c_str());
}

bool ResourceManager::isDirectory(const std::string &path)
{
    return PHYSFS_isDirectory(path.c_str());
}

std::string ResourceManager::getRealPath(const std::string &path)
{
    const char *dirSep = PHYSFS_getDirSeparator();

    return std::string(PHYSFS_getRealDir(path.c_str())) + dirSep + path;
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

    int fileSize;
    void *buffer = loadFile(idPath, fileSize);

    if (!buffer) {
        logger->log("Warning: resource doesn't exist!");
        return NULL;
    }

    Resource *resource = NULL;

    // Create an object of the specified type.
    switch (type)
    {
        case MUSIC:
            {
                // Let the music class load it
                resource = Music::load(buffer, fileSize, idPath);
            }
            break;
        case IMAGE:
            {
                // Let the image class load it
                resource = Image::load(buffer, fileSize, idPath);
            }
            break;
        case SOUND_EFFECT:
            {
                // Let the sound effect class load it
                resource = SoundEffect::load(buffer, fileSize, idPath);
            }
            break;
        default:
            /* Nothing to do here, just avoid compiler warnings... */
            break;
    }

    free(buffer);

    if (resource) {
        resource->incRef();

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

void*
ResourceManager::loadFile(const std::string &fileName, int &fileSize)
{
    // If the file doesn't exist indicate failure
    if (!PHYSFS_exists(fileName.c_str())) {
        logger->log("Warning: %s not found!", fileName.c_str());
        return NULL;
    }

    // Attempt to open the specified file using PhysicsFS
    PHYSFS_file *file = PHYSFS_openRead(fileName.c_str());

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

std::vector<std::string>
ResourceManager::loadTextFile(const std::string &fileName)
{
    int contentsLength;
    char *fileContents = (char*)loadFile(fileName, contentsLength);
    std::vector<std::string> lines;

    if (!fileContents)
    {
        logger->log("Couldn't load text file: %s", fileName.c_str());
        return lines;
    }

    // Reallocate and include terminating 0 character
    fileContents = (char*)realloc(fileContents, contentsLength + 1);
    fileContents[contentsLength] = '\0';

    // Tokenize and add each line separately
    char *line = strtok(fileContents, "\n");
    while (line != NULL)
    {
        lines.push_back(line);
        line = strtok(NULL, "\n");
    }

    free(fileContents);
    return lines;
}
