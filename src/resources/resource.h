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

#ifndef _TMW_RESOURCE_H
#define _TMW_RESOURCE_H

#include <string>

/**
 * A generic reference counted resource object.
 */
class Resource
{
    public:
        /**
         * Constructor
         */
        Resource(const std::string &idPath);

        /**
         * Increments the internal reference count.
         */
        void
        incRef();

        /**
         * Decrements the reference count and deletes the object
         * if no references are left.
         *
         * @return <code>true</code> if the object was deleted
         *         <code>false</code> otherwise.
         */
        bool
        decRef();

    protected:
        /**
         * Destructor.
         */
        virtual
        ~Resource();

    private:
        unsigned int mRefCount;    /**< Reference count */
        std::string mIdPath;       /**< Path identifying this resource */
};

#endif
