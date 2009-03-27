/*
 *  The Mana World
 *  Copyright (C) 2006  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef IMAGEPARTICLE_H
#define IMAGEPARTICLE_H

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

        /**
         * Destructor.
         */
        ~ImageParticle();

        /**
         * Draws the particle image
         */
        virtual void draw(Graphics *graphics, int offsetX, int offsetY) const;

    protected:
        Image *mImage;   /**< The image used for this particle. */
};

#endif
