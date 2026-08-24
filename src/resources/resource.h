/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <string>

class SubImage;

template<typename T> class ResourceRef;

/**
 * A generic reference counted resource object.
 */
class Resource
{
    friend class ResourceManager;
    template<typename T> friend class ResourceRef;

    public:
        Resource() = default;

        /**
         * Return the path identifying this resource.
         */
        const std::string &getIdPath() const
        { return mIdPath; }

    protected:
        virtual ~Resource() = default;

    private:
        enum OrphanPolicy {
            DeleteLater,
            DeleteImmediately
        };

        void incRef() { ++mRefCount; }

        /** Decrements the reference count, releasing the resource at zero. */
        static void decRef(Resource *resource,
                           OrphanPolicy orphanPolicy = DeleteLater);

        std::string mIdPath;    /**< Path identifying this resource. */
        time_t mTimeStamp = 0;  /**< Time at which the resource was orphaned. */
        unsigned mRefCount = 0; /**< Reference count. */

        /** When this resource was inserted, wide enough not to wrap around. */
        uint64_t mInsertionOrder = 0;
};

/**
 * Automatically counting Resource reference.
 */
template<typename RESOURCE>
class ResourceRef
{
public:
    // Allow implicit construction from RESOURCE *
    ResourceRef(RESOURCE *resource = nullptr)
        : mResource(resource)
    {
        if (mResource)
            mResource->incRef();
    }

    ResourceRef(std::nullptr_t)
        : mResource(nullptr)
    {}

    /**
     * Sub-images are not managed by the ResourceManager and should not
     * be reference counted by the ResourceRef.
     */
    ResourceRef(SubImage *) = delete;

    // Copy constructor
    ResourceRef(const ResourceRef &other)
        : mResource(other.mResource)
    {
        if (mResource)
            mResource->incRef();
    }

    // Move constructor
    ResourceRef(ResourceRef &&other)
        : mResource(other.mResource)
    {
        other.mResource = nullptr;
    }

    // Destructor
    ~ResourceRef()
    {
        if (mResource)
            Resource::decRef(mResource);
    }

    // Assignment operator
    ResourceRef &operator=(const ResourceRef &other)
    {
        if (this != &other)
        {
            if (mResource)
                Resource::decRef(mResource);

            mResource = other.mResource;

            if (mResource)
                mResource->incRef();
        }
        return *this;
    }

    // Move assignment operator
    ResourceRef &operator=(ResourceRef &&other)
    {
        if (this != &other)
        {
            if (mResource)
                Resource::decRef(mResource);

            mResource = other.mResource;
            other.mResource = nullptr;
        }
        return *this;
    }

    // Allow dereferencing
    RESOURCE *operator->() const
    { return mResource; }

    RESOURCE *get() const
    { return mResource; }

    // Allow implicit conversion to RESOURCE *
    operator RESOURCE *() const
    { return mResource; }

private:
    RESOURCE *mResource;
};
