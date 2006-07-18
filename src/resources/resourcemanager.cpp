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
#include <sstream>
#include <physfs.h>

#include "image.h"
#include "music.h"
#include "soundeffect.h"

#include "../log.h"

#include "../graphic/spriteset.h"


ResourceManager *ResourceManager::instance = NULL;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    // Create our resource iterator.
    ResourceIterator iter = mResources.begin();

    // Iterate through and release references until objects are deleted.
    while (!mResources.empty())
    {
        Resource *res = mResources.begin()->second;
        std::string id = res->getIdPath();
        int references = 0;

        references += res->mRefCount;
        release(res->mIdPath);
        delete res;

        logger->log("ResourceManager::~ResourceManager() cleaned up %d "
                    "references to %s", references, id.c_str());
    }
}

bool
ResourceManager::setWriteDir(const std::string &path)
{
    return (bool)PHYSFS_setWriteDir(path.c_str());
}

void
ResourceManager::addToSearchPath(const std::string &path, bool append)
{
    logger->log("Adding to PhysicsFS: %s", path.c_str());
    PHYSFS_addToSearchPath(path.c_str(), append ? 1 : 0);
}

bool
ResourceManager::mkdir(const std::string &path)
{
    return (bool)PHYSFS_mkdir(path.c_str());
}

bool
ResourceManager::exists(const std::string &path)
{
    return PHYSFS_exists(path.c_str());
}

bool
ResourceManager::isDirectory(const std::string &path)
{
    return PHYSFS_isDirectory(path.c_str());
}

Resource*
ResourceManager::get(const E_RESOURCE_TYPE &type, const std::string &idPath)
{
    // Check if the id exists, and return the value if it does.
    ResourceIterator resIter = mResources.find(idPath);

    if (resIter != mResources.end() && resIter->second) {
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

        mResources[idPath] = resource;
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

Spriteset* ResourceManager::createSpriteset(const std::string &imagePath,
        int w, int h)
{
    Image *img;

    if (!(img = getImage(imagePath))) {
        return NULL;
    }

    Spriteset *result = new Spriteset(img, w, h);

    img->decRef();

    return result;
}

void
ResourceManager::release(const std::string &idPath)
{
    logger->log("ResourceManager::release(%s)", idPath.c_str());

    ResourceIterator resIter = mResources.find(idPath);

    // The resource has to exist
    assert(resIter != mResources.end() && resIter->second);

    mResources.erase(idPath);
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
    delete instance;
    instance = NULL;
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

    std::istringstream iss(std::string(fileContents, contentsLength));
    std::string line;

    while (getline(iss, line, '\n')) {
        lines.push_back(line);
    }

    free(fileContents);
    return lines;
}
