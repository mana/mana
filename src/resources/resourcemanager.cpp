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

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

ResourceEntry::ResourceEntry():
    resource(NULL)
{
}

ResourceManager *ResourceManager::instance = NULL;

ResourceManager::ResourceManager()
{
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

#ifdef __DEBUG
    std::cout << "ResourceManager::~ResourceManager() cleaned up " <<
        danglingReferences << " references to " << danglingResources <<
        " resources\n";
#endif
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

    // Attempt to load the object if it doesn't exist.

    // The filePath string.
    std::string filePath = "";

    // Retrieve the current path for this program.
    char programPath[260];
    getcwd(programPath, 260);

    // Set the filePath variable to the appropriate value
    // this is only if we're not using a packed file.
    filePath = std::string(programPath) + std::string("/data/") + idPath;

    Resource *resource = NULL;

    // Create an object of the specified type.
    switch (type)
    {
        case MAP:
            warning("Map resource not supported.");
            break;
        case MUSIC:
            warning("Music resource not supported.");
            break;
        case IMAGE:
            // Attempt to create and load our image object.
            resource =
                reinterpret_cast<Resource*>(Image::load(filePath, flags));
            break;
        case SCRIPT:
            warning("Script resource not supported.");
            break;
        case TILESET:
            warning("Tileset resource not supported.");
            break;
        case SOUND_EFFECT:
            warning("Sound FX resource not supported.");
            break;
        default: 
            warning("Unknown resource type");
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
