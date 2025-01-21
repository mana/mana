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

#include <ctime>
#include <string>

/**
 * A generic reference counted resource object.
 */
class Resource
{
    friend class ResourceManager;

    public:
        enum OrphanPolicy {
            DeleteLater,
            DeleteImmediately
        };

        Resource() = default;

        /**
         * Increments the internal reference count.
         */
        void incRef() { ++mRefCount; }

        /**
         * Decrements the reference count. When no references are left, either
         * schedules the object for deletion or deletes it immediately,
         * depending on the \a orphanPolicy.
         */
        void decRef(OrphanPolicy orphanPolicy = DeleteLater);

        /**
         * Return the path identifying this resource.
         */
        const std::string &getIdPath() const
        { return mIdPath; }

    protected:
        virtual ~Resource() = default;

    private:
        std::string mIdPath;    /**< Path identifying this resource. */
        time_t mTimeStamp;      /**< Time at which the resource was orphaned. */
        unsigned mRefCount = 0; /**< Reference count. */
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
            mResource->decRef();
    }

    // Assignment operator
    ResourceRef &operator=(const ResourceRef &other)
    {
        if (this != &other)
        {
            if (mResource)
                mResource->decRef();

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
                mResource->decRef();

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

    /**
     * Releases the resource without decrementing the reference count!
     *
     * This is currently necessary to avoid calls to decRef on instances of
     * SubImage, which are not reference counted resources.
     */
    void release()
    { mResource = nullptr; }

private:
    RESOURCE *mResource;
};
