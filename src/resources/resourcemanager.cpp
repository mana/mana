/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "resources/resourcemanager.h"

#include "client.h"
#include "log.h"

#include "resources/dye.h"
#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/music.h"
#include "resources/soundeffect.h"
#include "resources/spritedef.h"

#include "utils/zlib.h"
#include "utils/physfsrwops.h"

#include <physfs.h>

#include <SDL_image.h>

#include <cassert>
#include <sstream>
#include <memory>

#include <sys/time.h>

ResourceManager *ResourceManager::instance = nullptr;

ResourceManager::ResourceManager()
  : mOldestOrphan(0)
{
    logger->log("Initializing resource manager...");
}

ResourceManager::~ResourceManager()
{
    mResources.insert(mOrphanedResources.begin(), mOrphanedResources.end());

    // Release any remaining spritedefs first because they depend on image sets
    auto iter = mResources.begin();
    while (iter != mResources.end())
    {
        if (dynamic_cast<SpriteDef*>(iter->second) != nullptr)
        {
            cleanUp(iter->second);
            auto toErase = iter;
            ++iter;
            mResources.erase(toErase);
        }
        else
        {
            ++iter;
        }
    }

    // Release any remaining image sets first because they depend on images
    iter = mResources.begin();
    while (iter != mResources.end())
    {
        if (dynamic_cast<ImageSet*>(iter->second) != nullptr)
        {
            cleanUp(iter->second);
            auto toErase = iter;
            ++iter;
            mResources.erase(toErase);
        }
        else
        {
            ++iter;
        }
    }

    // Release remaining resources, logging the number of dangling references.
    iter = mResources.begin();
    while (iter != mResources.end())
    {
        cleanUp(iter->second);
        ++iter;
    }
}

void ResourceManager::cleanUp(Resource *res)
{
    if (res->mRefCount > 0)
    {
        logger->log("ResourceManager::~ResourceManager() cleaning up %d "
                "reference%s to %s",
                res->mRefCount,
                (res->mRefCount == 1) ? "" : "s",
                res->mIdPath.c_str());
    }

    delete res;
}

void ResourceManager::cleanOrphans()
{
    // Delete orphaned resources after 30 seconds.
    time_t oldest = time(nullptr);
    time_t threshold = oldest - 30;

    if (mOrphanedResources.empty() || mOldestOrphan >= threshold)
        return;

    auto iter = mOrphanedResources.begin();
    while (iter != mOrphanedResources.end())
    {
        Resource *res = iter->second;
        const time_t t = res->mTimeStamp;
        if (t >= threshold)
        {
            if (t < oldest)
                oldest = t;
            ++iter;
        }
        else
        {
            logger->log("ResourceManager::release(%s)", res->mIdPath.c_str());
            iter = mOrphanedResources.erase(iter);
            delete res; // delete only after removal from list, to avoid issues in recursion
        }
    }

    mOldestOrphan = oldest;
}

bool ResourceManager::setWriteDir(const std::string &path)
{
    return (bool) PHYSFS_setWriteDir(path.c_str());
}

bool ResourceManager::addToSearchPath(const std::string &path, bool append)
{
    logger->log("Adding to PhysicsFS: %s", path.c_str());
    if (!PHYSFS_mount(path.c_str(), nullptr, append ? 1 : 0))
    {
        logger->log("Error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return false;
    }
    return true;
}

void ResourceManager::searchAndAddArchives(const std::string &path,
                                           const std::string &ext,
                                           bool append)
{
    const char *dirSep = PHYSFS_getDirSeparator();
    char **list = PHYSFS_enumerateFiles(path.c_str());

    for (char **i = list; *i; i++)
    {
        size_t len = strlen(*i);

        if (len > ext.length() && !ext.compare((*i)+(len - ext.length())))
        {
            std::string file, realPath, archive;

            file = path + (*i);
            realPath = std::string(PHYSFS_getRealDir(file.c_str()));
            archive = realPath + dirSep + file;

            addToSearchPath(archive, append);
        }
    }

    PHYSFS_freeList(list);
}

bool ResourceManager::mkdir(const std::string &path)
{
    return (bool) PHYSFS_mkdir(path.c_str());
}

bool ResourceManager::exists(const std::string &path)
{
    return PHYSFS_exists(path.c_str());
}

bool ResourceManager::isDirectory(const std::string &path)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(path.c_str(), &stat) != 0)
    {
        return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
    }
    return false;
}

std::string ResourceManager::getPath(const std::string &file)
{
    // get the real path to the file
    const char* tmp = PHYSFS_getRealDir(file.c_str());
    std::string path;

    // if the file is not in the search path, then its NULL
    if (tmp)
    {
        path = std::string(tmp) + "/" + file;
    }
    else
    {
        // if not found in search path return the default path
        path = Client::getPackageDirectory() + "/" + file;
    }

    return path;
}

SDL_RWops *ResourceManager::open(const std::string &path)
{
    return PHYSFSRWOPS_openRead(path.c_str());
}

Resource *ResourceManager::get(const std::string &idPath,
                               const std::function<Resource *()> &generator)
{
    // Check if the id exists, and return the value if it does.
    auto resIter = mResources.find(idPath);
    if (resIter != mResources.end())
    {
        resIter->second->incRef();
        return resIter->second;
    }

    resIter = mOrphanedResources.find(idPath);
    if (resIter != mOrphanedResources.end())
    {
        Resource *res = resIter->second;
        mResources.insert(*resIter);
        mOrphanedResources.erase(resIter);
        res->incRef();
        return res;
    }

    Resource *resource = generator();

    if (resource)
    {
        resource->incRef();
        resource->mIdPath = idPath;
        mResources[idPath] = resource;
        cleanOrphans();
    }

    // Returns NULL if the object could not be created.
    return resource;
}

Resource *ResourceManager::get(const std::string &path, loader fun)
{
    return get(path, [&] () -> Resource * {
        if (SDL_RWops *rw = open(path))
            return fun(rw);
        return nullptr;
    });
}

Music *ResourceManager::getMusic(const std::string &idPath)
{
    return static_cast<Music*>(get(idPath, Music::load));
}

SoundEffect *ResourceManager::getSoundEffect(const std::string &idPath)
{
    return static_cast<SoundEffect*>(get(idPath, SoundEffect::load));
}

Image *ResourceManager::getImage(const std::string &idPath)
{
    return static_cast<Image*>(get(idPath, [&] () -> Resource * {
        std::string path = idPath;
        std::string::size_type p = path.find('|');
        std::unique_ptr<Dye> d;
        if (p != std::string::npos)
        {
            d = std::make_unique<Dye>(path.substr(p + 1));
            path = path.substr(0, p);
        }
        SDL_RWops *rw = open(path);
        if (!rw)
            return nullptr;

        Resource *res = d ? Image::load(rw, *d)
                          : Image::load(rw);
        return res;
    }));
}

ResourceRef<Image> ResourceManager::getImageRef(const std::string &idPath)
{
    ResourceRef<Image> img = getImage(idPath);
    img->decRef();  // remove ref added by ResourceManager::get
    return img;
}

ImageSet *ResourceManager::getImageSet(const std::string &imagePath,
                                       int w, int h)
{
    std::stringstream ss;
    ss << imagePath << "[" << w << "x" << h << "]";

    return static_cast<ImageSet*>(get(ss.str(), [&] () -> Resource * {
        auto img = getImageRef(imagePath);
        if (!img)
            return nullptr;

        return new ImageSet(img, w, h);
    }));
}

SpriteDef *ResourceManager::getSprite(const std::string &path, int variant)
{
    std::stringstream ss;
    ss << path << "[" << variant << "]";

    return static_cast<SpriteDef*>(get(ss.str(), [&] () -> Resource * {
        return SpriteDef::load(path, variant);
    }));
}

void ResourceManager::release(Resource *res)
{
    auto resIter = mResources.find(res->mIdPath);

    // The resource has to exist
    assert(resIter != mResources.end() && resIter->second == res);

    const time_t timestamp = time(nullptr);

    res->mTimeStamp = timestamp;
    if (mOrphanedResources.empty())
        mOldestOrphan = timestamp;

    mOrphanedResources.insert(*resIter);
    mResources.erase(resIter);
}

void ResourceManager::remove(Resource *res)
{
    mResources.erase(res->mIdPath);
}

ResourceManager *ResourceManager::getInstance()
{
    // Create a new instance if necessary.
    if (!instance)
        instance = new ResourceManager;
    return instance;
}

void ResourceManager::deleteInstance()
{
    delete instance;
    instance = nullptr;
}

void *ResourceManager::loadFile(const std::string &filename, int &filesize,
                                bool inflate)
{
    // Attempt to open the specified file using PhysicsFS
    PHYSFS_file *file = PHYSFS_openRead(filename.c_str());

    // If the handler is an invalid pointer indicate failure
    if (file == nullptr)
    {
        logger->log("Warning: Failed to load %s: %s",
                filename.c_str(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return nullptr;
    }

    // Log the real dir of the file
    logger->log("Loaded %s/%s", PHYSFS_getRealDir(filename.c_str()),
            filename.c_str());

    // Get the size of the file
    filesize = PHYSFS_fileLength(file);

    // Allocate memory and load the file
    void *buffer = malloc(filesize);
    PHYSFS_readBytes(file, buffer, filesize);

    // Close the file and let the user deallocate the memory
    PHYSFS_close(file);

    if (inflate && filename.find(".gz", filename.length() - 3)
            != std::string::npos)
    {
        unsigned char *inflated;

        // Inflate the gzipped map data
        filesize = inflateMemory((unsigned char*) buffer, filesize, inflated);
        free(buffer);

        buffer = inflated;

        if (!buffer)
        {
            logger->log("Could not decompress file: %s", filename.c_str());
        }
    }

    return buffer;
}

bool ResourceManager::copyFile(const std::string &src, const std::string &dst)
{
    PHYSFS_file *srcFile = PHYSFS_openRead(src.c_str());
    if (!srcFile)
    {
        logger->log("Read error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return false;
    }
    PHYSFS_file *dstFile = PHYSFS_openWrite(dst.c_str());
    if (!dstFile)
    {
        logger->log("Write error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        PHYSFS_close(srcFile);
        return false;
    }

    int fileSize = PHYSFS_fileLength(srcFile);
    void *buf = malloc(fileSize);
    PHYSFS_readBytes(srcFile, buf, fileSize);
    PHYSFS_writeBytes(dstFile, buf, fileSize);

    PHYSFS_close(srcFile);
    PHYSFS_close(dstFile);
    free(buf);
    return true;
}

std::vector<std::string> ResourceManager::loadTextFile(
        const std::string &fileName)
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

    while (getline(iss, line))
        lines.push_back(line);

    free(fileContents);
    return lines;
}
