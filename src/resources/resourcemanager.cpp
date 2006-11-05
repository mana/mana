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
#include <SDL_image.h>

#include "image.h"
#include "music.h"
#include "soundeffect.h"
#include "spriteset.h"

#include "../log.h"


ResourceManager *ResourceManager::instance = NULL;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    // Release any remaining spritesets first because they depend on images
    ResourceIterator iter = mResources.begin();
    while (iter != mResources.end())
    {
        if (dynamic_cast<Spriteset*>(iter->second) != NULL)
        {
            cleanUp(iter->second);
            ResourceIterator toErase = iter;
            ++iter;
            mResources.erase(toErase);
        }
        else
        {
            ++iter;
        }
    }

    // Release remaining resources, logging the number of dangling references.
    while (!mResources.empty())
    {
        cleanUp(mResources.begin()->second);
        mResources.erase(mResources.begin());
    }
}

void
ResourceManager::cleanUp(Resource *res)
{
    logger->log("ResourceManager::~ResourceManager() cleaning up %d "
                "reference%s to %s",
                res->mRefCount,
                (res->mRefCount == 1) ? "" : "s",
                res->mIdPath.c_str());
    delete res;
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

    // Returns NULL if the object could not be created.
    return resource;
}

Image*
ResourceManager::getImage(const std::string &idPath)
{
    return dynamic_cast<Image*>(get(IMAGE, idPath));
}

Music*
ResourceManager::getMusic(const std::string &idPath)
{
    return dynamic_cast<Music*>(get(MUSIC, idPath));
}

SoundEffect*
ResourceManager::getSoundEffect(const std::string &idPath)
{
    return dynamic_cast<SoundEffect*>(get(SOUND_EFFECT, idPath));
}

Spriteset*
ResourceManager::getSpriteset(const std::string &imagePath, int w, int h)
{
    std::stringstream ss;
    ss << imagePath << "[" << w << "x" << h << "]";
    const std::string idPath = ss.str();

    ResourceIterator resIter = mResources.find(idPath);

    if (resIter != mResources.end()) {
        resIter->second->incRef();
        return dynamic_cast<Spriteset*>(resIter->second);
    }

    Image *img = getImage(imagePath);

    if (!img) {
        return NULL;
    }

    Spriteset *spriteset = new Spriteset(idPath, img, w, h);
    spriteset->incRef();
    mResources[idPath] = spriteset;

    img->decRef();

    return spriteset;
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

SDL_Surface*
ResourceManager::loadSDLSurface(const std::string& filename)
{
    int fileSize;
    void *buffer = loadFile(filename, fileSize);
    SDL_Surface *tmp = NULL;

    if (buffer) {
        SDL_RWops *rw = SDL_RWFromMem(buffer, fileSize);
        tmp = IMG_Load_RW(rw, 1);
        ::free(buffer);
    }

    return tmp;
}
