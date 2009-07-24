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

#include "sdlrescalefacility.h"

#define VALUE_LIMIT 0.001

typedef struct tColorRGBA {
Uint8 r;
Uint8 g;
Uint8 b;
Uint8 a;
} tColorRGBA;

void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight)
{
    /*
     * Sanity check zoom factors 
     */
    if (zoomx < VALUE_LIMIT) {
    zoomx = VALUE_LIMIT;
    }
    if (zoomy < VALUE_LIMIT) {
    zoomy = VALUE_LIMIT;
    }

    /*
     * Calculate target size 
     */
    *dstwidth = (int) ((double) width * zoomx);
    *dstheight = (int) ((double) height * zoomy);
    if (*dstwidth < 1) {
    *dstwidth = 1;
    }
    if (*dstheight < 1) {
    *dstheight = 1;
    }
}

int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy, int smooth)
{
    int x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, ex, ey, t1, t2, sstep, lx, ly;
    tColorRGBA *c00, *c01, *c10, *c11, *cswap;
    tColorRGBA *sp, *csp, *dp;
    int dgap;

    /*
     * Variable setup 
     */
    if (smooth) {
    /*
     * For interpolation: assume source dimension is one pixel 
     */
    /*
     * smaller to avoid overflow on right and bottom edge.     
     */
    sx = (int) (65536.0 * (float) (src->w - 1) / (float) dst->w);
    sy = (int) (65536.0 * (float) (src->h - 1) / (float) dst->h);
    } else {
    sx = (int) (65536.0 * (float) src->w / (float) dst->w);
    sy = (int) (65536.0 * (float) src->h / (float) dst->h);
    }

    /*
     * Allocate memory for row increments 
     */
    if ((sax = (int *) malloc((dst->w + 1) * sizeof(Uint32))) == NULL) {
    return (-1);
    }
    if ((say = (int *) malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
    free(sax);
    return (-1);
    }

    /*
     * Precalculate row increments 
     */
    sp = csp = (tColorRGBA *) src->pixels;
    dp = (tColorRGBA *) dst->pixels;

    if (flipx) csp += (src->w-1);
    if (flipy) csp += (src->pitch*(src->h-1));

    csx = 0;
    csax = sax;
    for (x = 0; x <= dst->w; x++) {
    *csax = csx;
    csax++;
    csx &= 0xffff;
    csx += sx;
    }
    csy = 0;
    csay = say;
    for (y = 0; y <= dst->h; y++) {
    *csay = csy;
    csay++;
    csy &= 0xffff;
    csy += sy;
    }

    dgap = dst->pitch - dst->w * 4;

    /*
     * Switch between interpolating and non-interpolating code 
     */
    if (smooth) {

    /*
     * Interpolating Zoom 
     */

    /*
     * Scan destination 
     */
    ly = 0;
    csay = say;
    for (y = 0; y < dst->h; y++) {
            /*
             * Setup color source pointers 
             */
            c00 = csp;      
            c01 = csp;
            c01++;      
            c10 = (tColorRGBA *) ((Uint8 *) csp + src->pitch);
            c11 = c10;
            c11++;
            csax = sax;
            if (flipx) {
         cswap = c00; c00=c01; c01=cswap;
         cswap = c10; c10=c11; c11=cswap;
            }
            if (flipy) {
         cswap = c00; c00=c10; c10=cswap;
         cswap = c01; c01=c11; c11=cswap;
            }
            lx = 0;
        for (x = 0; x < dst->w; x++) {
        /*
         * Interpolate colors 
         */
        ex = (*csax & 0xffff);
        ey = (*csay & 0xffff);
        t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
        t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
        dp->r = (((t2 - t1) * ey) >> 16) + t1;
        t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
        t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
        dp->g = (((t2 - t1) * ey) >> 16) + t1;
        t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
        t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
        dp->b = (((t2 - t1) * ey) >> 16) + t1;
        t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
        t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
        dp->a = (((t2 - t1) * ey) >> 16) + t1;

        /*
         * Advance source pointers 
         */
        csax++;
        sstep = (*csax >> 16);
        lx += sstep;
        if (lx >= src->w) sstep = 0;
        if (flipx) sstep = -sstep;
        c00 += sstep;
        c01 += sstep;
        c10 += sstep;
        c11 += sstep;
        /*
         * Advance destination pointer 
         */
        dp++;
        }
        /*
         * Advance source pointer 
         */
        csay++;
        sstep = (*csay >> 16);
            ly += sstep;
            if (ly >= src->h) sstep = 0;
            sstep *= src->pitch;
        if (flipy) sstep = -sstep;
        csp = (tColorRGBA *) ((Uint8 *) csp + sstep);

        /*
         * Advance destination pointers 
         */
        dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
    }
    } else {

    /*
     * Non-Interpolating Zoom 
     */

    csay = say;
    for (y = 0; y < dst->h; y++) {
        sp = csp;
        csax = sax;
        for (x = 0; x < dst->w; x++) {
        /*
         * Draw 
         */
        *dp = *sp;
        /*
         * Advance source pointers 
         */
        csax++;
        sstep = (*csax >> 16);
        if (flipx) sstep = -sstep;
        sp += sstep;
        /*
         * Advance destination pointer 
         */
        dp++;
        }
        /*
         * Advance source pointer 
         */
        csay++;
        sstep = (*csay >> 16) * src->pitch;
        if (flipy) sstep = -sstep;
        csp = (tColorRGBA *) ((Uint8 *) csp + sstep);

        /*
         * Advance destination pointers 
         */
        dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
    }
    }

    /*
     * Remove temp arrays 
     */
    free(sax);
    free(say);

    return (0);
}



int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy)
{
    Uint32 x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, sstep;
    Uint8 *sp, *dp, *csp;
    int dgap;

    /*
     * Variable setup 
     */
    sx = (Uint32) (65536.0 * (float) src->w / (float) dst->w);
    sy = (Uint32) (65536.0 * (float) src->h / (float) dst->h);

 
     /*
     * Allocate memory for row increments 
     */
    if ((sax = (Uint32 *) malloc((dst->w + 1) * sizeof(Uint32))) == NULL) {
    return (-1);
    }
    if ((say = (Uint32 *) malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
    free(sax);
    return (-1);
    }

    /*
     * Pointer setup 
     */
    sp = csp = (Uint8 *) src->pixels;
    dp = (Uint8 *) dst->pixels;
    dgap = dst->pitch - dst->w;

    if (flipx) csp += (src->w-1);
    if (flipy) csp  = ( (Uint8*)csp + src->pitch*(src->h-1) );

    /*
     * Precalculate row increments 
     */
    csx = 0;
    csax = sax;
    for (x = 0; x <= dst->w; x++) {
    *csax = csx;
    csax++;
    csx &= 0xffff;
    csx += sx;
    }
    csy = 0;
    csay = say;
    for (y = 0; y <= dst->h; y++) {
    *csay = csy;
    csay++;
    csy &= 0xffff;
    csy += sy;
    }


    /*
     * Draw 
     */
    csay = say;
    for (y = 0; y < dst->h; y++) {
    csax = sax;
    sp = csp;
    for (x = 0; x < dst->w; x++) {
        /*
         * Draw 
         */
        *dp = *sp;
        /*
         * Advance source pointers 
         */
        csax++;
            sstep = (*csax >> 16);
            if (flipx) sstep = -sstep;
            sp += sstep;
        /*
         * Advance destination pointer 
         */
        dp++;
    }
    /*
     * Advance source pointer (for row) 
     */
    csay++;
        sstep = (*csay >> 16) * src->pitch;
        if (flipy) sstep = -sstep;
        csp = ((Uint8 *) csp + sstep);

    /*
     * Advance destination pointers 
     */
    dp += dgap;
    }

    /*
     * Remove temp arrays 
     */
    free(sax);
    free(say);

    return (0);
}



SDL_Surface *_SDLzoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth)
{
    SDL_Surface *rz_src;
    SDL_Surface *rz_dst;
    int dstwidth, dstheight;
    int is32bit;
    int i, src_converted;
    int flipx, flipy;

    /*
     * Sanity check 
     */
    if (src == NULL)
    return (NULL);

    /*
     * Determine if source surface is 32bit or 8bit 
     */
    is32bit = (src->format->BitsPerPixel == 32);
    if ((is32bit) || (src->format->BitsPerPixel == 8)) {
    /*
     * Use source surface 'as is' 
     */
    rz_src = src;
    src_converted = 0;
    } else {
    /*
     * New source surface is 32bit with a defined RGBA ordering 
     */
    rz_src =
        SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, 
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                                0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#else
                                0xff000000,  0x00ff0000, 0x0000ff00, 0x000000ff
#endif
        );
    SDL_BlitSurface(src, NULL, rz_src, NULL);
    src_converted = 1;
    is32bit = 1;
    }

    flipx = (zoomx<0.0);
    if (flipx) zoomx = -zoomx;
    flipy = (zoomy<0.0);
    if (flipy) zoomy = -zoomy;

    /* Get size if target */
    zoomSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth, &dstheight);

    /*
     * Alloc space to completely contain the zoomed surface 
     */
    rz_dst = NULL;
    if (is32bit) {
    /*
     * Target surface is 32bit with source RGBA/ABGR ordering 
     */
    rz_dst =
        SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
                 rz_src->format->Rmask, rz_src->format->Gmask,
                 rz_src->format->Bmask, rz_src->format->Amask);
    } else {
    /*
     * Target surface is 8bit 
     */
    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0);
    }

    /*
     * Lock source surface 
     */
    SDL_LockSurface(rz_src);
    /*
     * Check which kind of surface we have 
     */
    if (is32bit) {
    /*
     * Call the 32bit transformation routine to do the zooming (using alpha) 
     */
    zoomSurfaceRGBA(rz_src, rz_dst, flipx, flipy, smooth);
    /*
     * Turn on source-alpha support 
     */
    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
    } else {
    /*
     * Copy palette and colorkey info 
     */
    for (i = 0; i < rz_src->format->palette->ncolors; i++) {
        rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
    }
    rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
    /*
     * Call the 8bit transformation routine to do the zooming 
     */
    zoomSurfaceY(rz_src, rz_dst, flipx, flipy);
    SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, rz_src->format->colorkey);
    }
    /*
     * Unlock source surface 
     */
    SDL_UnlockSurface(rz_src);

    /*
     * Cleanup temp surface 
     */
    if (src_converted) {
    SDL_FreeSurface(rz_src);
    }

    /*
     * Return destination surface 
     */
    return (rz_dst);
} 
