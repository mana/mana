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

#include "slider.h"

Slider::Slider(double scaleEnd):
    gcn::Slider(scaleEnd)
{
    setBorderSize(0);
    mMarkerWidth = gui_skin.slider.hGrip->w;
}

Slider::Slider(double scaleStart, double scaleEnd):
    gcn::Slider(scaleStart, scaleEnd)
{
    setBorderSize(0);
    mMarkerWidth = gui_skin.slider.hGrip->w;
}

void Slider::draw(gcn::Graphics *graphics)
{
    int w = getWidth();
    int h = getHeight();
    int x, y;
    getAbsolutePosition(x, y);

    y += (h - gui_skin.slider.hSlider[0]->h) / 2;

    masked_blit(gui_skin.slider.hSlider[0], gui_bitmap, 0, 0, x, y,
            gui_skin.slider.hSlider[0]->w, gui_skin.slider.hSlider[0]->h);

    w -= gui_skin.slider.hSlider[0]->w + gui_skin.slider.hSlider[2]->w;
    x += gui_skin.slider.hSlider[0]->w;

    masked_stretch_blit(gui_skin.slider.hSlider[1], gui_bitmap, 0, 0,
            gui_skin.slider.hSlider[1]->w, gui_skin.slider.hSlider[1]->h,
            x, y, w, gui_skin.slider.hSlider[1]->h);

    x += w;
    masked_blit(gui_skin.slider.hSlider[2], gui_bitmap, 0, 0, x, y,
            gui_skin.slider.hSlider[2]->w, gui_skin.slider.hSlider[2]->h);

    drawMarker(graphics);
}

void Slider::drawMarker(gcn::Graphics *graphics)
{
    int w = getWidth();
    int h = getHeight();
    int x, y;
    getAbsolutePosition(x, y);

    x += mMarkerPosition;
    y += (h - gui_skin.slider.hGrip->h) / 2;

    masked_blit(gui_skin.slider.hGrip, gui_bitmap, 0, 0, x, y,
            gui_skin.slider.hGrip->w, gui_skin.slider.hGrip->h);
}
