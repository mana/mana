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

#ifndef _TMW_RESOURCE_MANAGER_H
#define _TMW_RESOURCE_MANAGER_H

#include <iosfwd>
#include <map>
#include <vector>

class Resource;
class Image;
class Music;
class SoundEffect;
class Spriteset;

/**
 * A class for loading and managing resources.
 */
class ResourceManager
{
    public:
        /**
         * An enumeration of resource types.
         */
        enum E_RESOURCE_TYPE
        {
            //MAP,
            MUSIC,
            IMAGE,
            //SCRIPT,
            //TILESET,
            SOUND_EFFECT
        };

        /**
         * Constructor.
         */
        ResourceManager();

        /**
         * Destructor.
         */
        ~ResourceManager();

        /**
         * Sets the write directory.
         *
         * @param path The path of the directory to be added.
         * @return <code>true</code> on success, <code>false</code> otherwise.
         */
        bool
        setWriteDir(const std::string &path);

        /**
         * Adds a directory or archive to the search path. If append is true
         * then the directory is added to the end of the search path, otherwise
         * it is added at the front.
         */
        void
        addToSearchPath(const std::string &path, bool append);

        /**
         * Creates a directory in the write path
         */
        bool
        mkdir(const std::string &path);

        /**
         * Checks whether the given file or directory exists in the search path
         */
        bool
        exists(const std::string &path);

        /**
         * Checks whether the given path is a directory.
         */
        bool
        isDirectory(const std::string &path);

        /**
         * Creates a resource and adds it to the resource map. The idPath is
         * converted into the appropriate path for the current operating system
         * and the resource is loaded.
         *
         * @param type   The type of resource to load.
         * @param idPath The resource identifier path.
         * @return A valid resource or <code>NULL</code> if the resource could
         *         not be loaded.
         */
        Resource*
        get(const E_RESOURCE_TYPE &type, const std::string &idPath);

        /**
         * Convenience wrapper around ResourceManager::create for loading
         * images.
         */
        Image*
        getImage(const std::string &idPath);

        /**
         * Convenience wrapper around ResourceManager::create for loading
         * songs.
         */
        Music*
        getMusic(const std::string &idPath);

        /**
         * Convenience wrapper around ResourceManager::create for loading
         * samples.
         */
        SoundEffect*
        getSoundEffect(const std::string &idPath);

        /**
         * Creates a spriteset based on the image referenced by the given
         * path and the supplied sprite sizes
         */
        Spriteset* createSpriteset(const std::string &imagePath, int w, int h);

        /**
         * Releases a resource, removing it from the set of loaded resources.
         */
        void
        release(const std::string &idPath);

        /**
         * Allocates data into a buffer pointer for raw data loading. The
         * returned data is expected to be freed using <code>free()</code>.
         *
         * @param fileName The name of the file to be loaded.
         * @param fileSize The size of the file that was loaded.
         *
         * @return An allocated byte array containing the data that was loaded,
         *         or <code>NULL</code> on fail.
         */
        void*
        loadFile(const std::string &fileName, int &fileSize);

        /**
         * Retrieves the contents of a text file.
         */
        std::vector<std::string>
        loadTextFile(const std::string &fileName);

        /**
         * Returns an instance of the class, creating one if it does not
         * already exist.
         */
        static ResourceManager*
        getInstance();

        /**
         * Deletes the class instance if it exists.
         */
        static void
        deleteInstance();

    private:
        static ResourceManager *instance;
        typedef std::map<std::string, Resource*> Resources;
        typedef Resources::iterator ResourceIterator;
        Resources mResources;
};

#endif
