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

#include <map>
#include <string>
#include "resource.h"
#include "image.h"

/**
 * A resource entry descriptor.
 */
struct ResourceEntry
{
    ResourceEntry();

    Resource *resource;
    std::string filePath;
};

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
            MAP,
            MUSIC,
            IMAGE,
            SCRIPT,
            TILESET,
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
         * Creates a resource and adds it to the resource map. The idPath is
         * converted into the appropriate path for the current operating system
         * and the resource is loaded.
         *
         * @param type   The type of resource to load.
         * @param idPath The resource identifier path.
         * @param flags  Flags to control the loading of certain resources.
         * @return A valid resource or <code>NULL</code> if the resource could
         *         not be loaded.
         */
        Resource *get(
                const E_RESOURCE_TYPE &type,
                const std::string &idPath,
                int flags = 0);

        /**
         * Convenience wrapper around ResourceManager::create.
         */
        Image *getImage(const std::string &idPath, int flags = 0);

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
         * Searches for zip files and adds them to the PhysicsFS search path.
         */
        void searchAndAddZipFiles();

        /**
         * Allocates data into a buffer pointer for raw data loading
         *
         * @param fileName The name of the file to be loaded
         * @param buffer The empty buffer into which the data will be loaded
         *
         * @return The size of the file that was loaded
         */
        void *loadFile(const std::string &fileName, int &fileSize);


        static ResourceManager *instance;
        std::map<std::string, ResourceEntry> resources;
};

#endif
