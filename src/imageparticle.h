/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
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

#include "particle.h"

class Image;
class Map;

/**
 * A particle that uses an image for its visualization.
 */
class ImageParticle : public Particle
{
    public:
        /**
         * Constructor. The image is reference counted by this particle.
         *
         * @param map   the map this particle appears on
         * @param image an Image instance, may not be NULL
         */
        ImageParticle(Map *map, Image *image);

        ~ImageParticle() override;

        /**
         * Draws the particle image
         */
        bool draw(Graphics *graphics, int offsetX, int offsetY) const override;

    protected:
        Image *mImage;   /**< The image used for this particle. */
};
