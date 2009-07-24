/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the Low GNU General Public License as published by
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
 * 
 *  Source code taken from:
 *
 * SDL_rotozoom - rotozoomer
 * 
 * LGPL (c) A. Schiffler
 *
 */ 

#ifndef SDLRESCALEFACILITY_H
#define SDLRESCALEFACILITY_H

#include <SDL.h>
#include "image.h"

/**
 * _SDLzoomSurface is internally used by Image::getScaledImage() to provide
 * a rescaled copy of its internal mImage member.
 *
 * @see Image::getScaledImage() for more details
 *
 * @param src the original surface to rescale
 * @param zoomx the zoom factor used to rescale the surface horizontally. 1.0 doesn't rescale.
 * A value lesser than 1.O shrink the image.
 * @param zoomy the zoom factor used to rescale the surface vertically.
 * @param smooth transform the scaled surface into a 32bit aliased image to smooth the rescaling.
 *
 * @return SDL_Surface The rescaled surface.
 */

SDL_Surface *_SDLzoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth);

#endif