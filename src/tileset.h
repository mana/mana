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

#ifndef TILESET_H
#define TILESET_H

#include "resources/imageset.h"

/**
 * A tileset, which is basically just an image set but it stores a firstgid.
 */
class Tileset : public ImageSet
{
    public:
        Tileset(Image *img, int w, int h, int firstGid, int margin, int spacing):
            ImageSet(img, w, h, margin, spacing),
            mFirstGid(firstGid)
        {
        }

        /**
         * Returns the first gid.
         */
        int getFirstGid() const
        {
            return mFirstGid;
        }

    private:
        int mFirstGid;
};

#endif // TILESET_H
