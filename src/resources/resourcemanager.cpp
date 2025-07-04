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

#include "utils/filesystem.h"

#include <SDL_image.h>

#include <cassert>
#include <sstream>
#include <memory>

#include <sys/time.h>

ResourceManager *ResourceManager::instance = nullptr;

ResourceManager::ResourceManager()
{
    Log::info("Initializing resource manager...");
}

ResourceManager::~ResourceManager()
{
    auto cleanupResources = [&](auto match)
    {
        // Include any orphaned resources into the main list for cleanup
        mResources.insert(mOrphanedResources.begin(), mOrphanedResources.end());
        mOrphanedResources.clear();

        for (auto iter = mResources.begin(); iter != mResources.end(); )
        {
            if (match(iter->second))
            {
                cleanUp(iter->second);
                iter = mResources.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    };

    // SpriteDef references ImageSet
    cleanupResources([](Resource *res) { return dynamic_cast<SpriteDef *>(res); });

    // ImageSet references Image
    cleanupResources([](Resource *res) { return dynamic_cast<ImageSet *>(res); });

    // Release remaining resources
    cleanupResources([](Resource *res) { return true; });

    assert(mOrphanedResources.empty());
}

void ResourceManager::cleanUp(Resource *res)
{
    if (res->mRefCount > 0)
    {
        Log::info("ResourceManager::~ResourceManager() cleaning up %d "
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
    const time_t threshold = oldest - 30;

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
            Log::info("ResourceManager::release(%s)", res->mIdPath.c_str());
            iter = mOrphanedResources.erase(iter);
            delete res; // delete only after removal from list, to avoid issues in recursion
        }
    }

    mOldestOrphan = oldest;
}

bool ResourceManager::addToSearchPath(const std::string &path, bool append)
{
    if (!FS::addToSearchPath(path, append))
    {
        Log::error("Couldn't add search path: %s (%s)", path.c_str(), FS::getLastError());
        return false;
    }
    Log::info("Added search path: %s", path.c_str());
    return true;
}

void ResourceManager::searchAndAddArchives(const std::string &path,
                                           const std::string &ext,
                                           bool append)
{
    const char *dirSep = FS::getDirSeparator();

    for (auto fileName : FS::enumerateFiles(path))
    {
        const size_t len = strlen(fileName);

        if (len > ext.length() && ext != (fileName + (len - ext.length())))
        {
            std::string file = path + fileName;
            if (auto realDir = FS::getRealDir(file))
            {
                std::string archive = std::string(*realDir) + dirSep + file;
                addToSearchPath(archive, append);
            }
        }
    }
}

std::string ResourceManager::getPath(const std::string &file)
{
    // Get the real directory of the file
    auto realDir = FS::getRealDir(file);
    std::string path;

    if (realDir)
    {
        path = std::string(*realDir) + "/" + file;
    }
    else
    {
        // if not found in search path return the default path
        path = Client::getPackageDirectory() + "/" + file;
    }

    return path;
}

Resource *ResourceManager::get(const std::string &idPath,
                               const std::function<Resource *()> &generator)
{
    // Check if the id exists, and return the value if it does.
    auto resIter = mResources.find(idPath);
    if (resIter != mResources.end())
    {
        return resIter->second;
    }

    resIter = mOrphanedResources.find(idPath);
    if (resIter != mOrphanedResources.end())
    {
        Resource *res = resIter->second;
        mResources.insert(*resIter);
        mOrphanedResources.erase(resIter);
        return res;
    }

    Resource *resource = generator();
    if (resource)
    {
        resource->mIdPath = idPath;
        mResources[idPath] = resource;
        cleanOrphans();
    }

    return resource;
}

ResourceRef<Music> ResourceManager::getMusic(const std::string &path)
{
    return static_cast<Music*>(get(path, [&] () -> Resource * {
        if (SDL_RWops *rw = FS::openBufferedRWops(path))
            return Music::load(rw);

        return nullptr;
    }));
}

ResourceRef<SoundEffect> ResourceManager::getSoundEffect(const std::string &path)
{
    return static_cast<SoundEffect*>(get(path, [&] () -> Resource * {
        if (SDL_RWops *rw = FS::openBufferedRWops(path))
            return SoundEffect::load(rw);

        return nullptr;
    }));
}

ResourceRef<Image> ResourceManager::getImage(const std::string &idPath)
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
        SDL_RWops *rw = FS::openRWops(path);
        if (!rw)
            return nullptr;

        Resource *res = d ? Image::load(rw, *d)
                          : Image::load(rw);
        return res;
    }));
}

ResourceRef<ImageSet> ResourceManager::getImageSet(const std::string &imagePath,
                                                   int w, int h)
{
    std::stringstream ss;
    ss << imagePath << "[" << w << "x" << h << "]";

    return static_cast<ImageSet*>(get(ss.str(), [&] () -> Resource * {
        auto img = getImage(imagePath);
        if (!img)
            return nullptr;

        return new ImageSet(img, w, h);
    }));
}

ResourceRef<SpriteDef> ResourceManager::getSprite(const std::string &path, int variant)
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
