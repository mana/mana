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

#pragma once

#include "resources/resource.h"

#include <ctime>
#include <functional>
#include <map>
#include <string>

class Image;
class ImageSet;
class Music;
class SoundEffect;
class SpriteDef;

struct SDL_RWops;

/**
 * A class for loading and managing resources.
 */
class ResourceManager
{
    friend class Resource;

    public:

        using loader = Resource *(*)(SDL_RWops *);

        ResourceManager();

        /**
         * Destructor. Cleans up remaining resources, warning about resources
         * that were still referenced.
         */
        ~ResourceManager();

        /**
         * Adds a directory or archive to the search path. If append is true
         * then the directory is added to the end of the search path, otherwise
         * it is added at the front.
         *
         * @return <code>true</code> on success, <code>false</code> otherwise.
         */
        static bool addToSearchPath(const std::string &path, bool append);

        /**
        * Searches for zip files and adds them to the search path.
        */
        static void searchAndAddArchives(const std::string &path,
                                         const std::string &ext,
                                         bool append);

        /**
         * Returns the real path to a file. Note that this method will always
         * return a path, it does not check whether the file exists.
         *
         * @param file The file to get the real path to.
         * @return The real path.
         */
        static std::string getPath(const std::string &file);

        /**
         * Creates a resource and adds it to the resource map.
         *
         * @param idPath The resource identifier path.
         * @param fun    A function for generating the resource.
         * @param data   Extra parameters for the generator.
         * @return A valid resource or <code>NULL</code> if the resource could
         *         not be generated.
         */
        Resource *get(const std::string &idPath,
                      const std::function<Resource *()> &generator);

        /**
         * Loads a resource from a file and adds it to the resource map.
         *
         * @param path The file name.
         * @param fun  A function for parsing the file.
         * @return A valid resource or <code>NULL</code> if the resource could
         *         not be loaded.
         */
        Resource *get(const std::string &path, loader fun);

        /**
         * Convenience wrapper around ResourceManager::get for loading
         * images.
         */
        Image *getImage(const std::string &idPath);

        /**
         * Convenience wrapper around ResourceManager::get for loading
         * images. Returns an automatically reference-counted resource.
         */
        ResourceRef<Image> getImageRef(const std::string &idPath);

        /**
         * Convenience wrapper around ResourceManager::get for loading
         * songs.
         */
        Music *getMusic(const std::string &idPath);

        /**
         * Convenience wrapper around ResourceManager::get for loading
         * samples.
         */
        SoundEffect *getSoundEffect(const std::string &idPath);

        /**
         * Creates a image set based on the image referenced by the given
         * path and the supplied sprite sizes
         */
        ImageSet *getImageSet(const std::string &imagePath, int w, int h);

        /**
         * Creates a sprite definition based on a given path and the supplied
         * variant.
         */
        SpriteDef *getSprite(const std::string &path, int variant = 0);

        /**
         * Returns an instance of the class, creating one if it does not
         * already exist.
         */
        static ResourceManager *getInstance();

        /**
         * Deletes the class instance if it exists.
         */
        static void deleteInstance();

    private:
        /**
         * Releases a resource, placing it in the set of orphaned resources.
         * Only called from Resource::decRef,
         */
        void release(Resource *);

        /**
         * Removes a resource from the list of resources managed by the
         * resource manager. Only called from Resource::decRef,
         */
        void remove(Resource *);

        /**
         * Deletes the resource after logging a cleanup message.
         */
        static void cleanUp(Resource *resource);

        void cleanOrphans();

        static ResourceManager *instance;
        std::map<std::string, Resource *> mResources;
        std::map<std::string, Resource *> mOrphanedResources;
        time_t mOldestOrphan;
};
